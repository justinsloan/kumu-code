#include "interpreter.hpp"
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <filesystem>
#include <cctype>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>

namespace kumu {

namespace {

// Thrown by ReturnStatement::execute and caught by CallExpression::evaluate
// to unwind out of a function body (including through nested IF/WHILE/FOR
// blocks) and carry the returned value back to the caller.
struct ReturnSignal {
    Value value;
};

// BREAK and CONTINUE unwind to the innermost enclosing loop the same way
// ReturnSignal unwinds to the enclosing call. Like ReturnSignal, these are
// deliberately NOT derived from std::runtime_error, so TryStatement -- which
// catches runtime_error -- lets them pass straight through: a BREAK inside a
// TRY block breaks the loop instead of being swallowed as an error.
struct BreakSignal {};
struct ContinueSignal {};

// Kumu function calls ride on the C++ call stack, so runaway recursion used
// to exhaust it and kill the process outright (SIGSEGV, no message, no line).
// This guard trips first, turning it into a normal Kumu error a program can
// even catch with TRY/CATCH. 1000 is far below the real stack limit but far
// above any plausible teaching example.
constexpr int kMaxCallDepth = 1000;

struct CallDepthGuard {
    Context& ctx;
    CallDepthGuard(Context& c, const std::string& functionName) : ctx(c) {
        if (ctx.callDepth >= kMaxCallDepth) {
            throw std::runtime_error("Maximum recursion depth (" + std::to_string(kMaxCallDepth) +
                                      ") exceeded -- is this function missing a base case?");
        }
        ctx.callDepth++;
        ctx.callStack.push_back({functionName, ctx.currentLine});
    }
    ~CallDepthGuard() {
        ctx.callDepth--;
        ctx.callStack.pop_back();
    }
};

// Renders the calls in progress, innermost first, as the trail that led to an
// error. Empty at the top level, where there is no trail to show.
std::string formatCallStack(const Context& ctx) {
    // A runaway recursion has a thousand frames, and printing them all buries
    // the message. The ends are what carry information: the innermost frames
    // show where it broke, the outermost show how it got started.
    constexpr size_t kHead = 5, kTail = 3;
    const size_t n = ctx.callStack.size();
    std::string out;
    for (size_t i = 0; i < n; ++i) {
        if (n > kHead + kTail + 1 && i == kHead) {
            out += "\n  ... " + std::to_string(n - kHead - kTail) + " more calls ...";
            i = n - kTail - 1;
            continue;
        }
        const auto& frame = ctx.callStack[n - 1 - i]; // innermost first
        out += "\n  in " + frame.function + "() called at line " + std::to_string(frame.line);
    }
    return out;
}

bool isTruthy(const Value& v) {
    if (is_number(v)) return as_number(v) != 0.0;
    if (is_string(v)) return !as_string(v).empty();
    if (is_matrix(v)) return !as_matrix(v).data.empty();
    if (is_list(v)) return !as_list(v).items.empty();
    return false;
}

// Used in error messages so "Cannot index a List" reads correctly instead of
// falling back on a two-way Number/String guess.
std::string valueKind(const Value& v) {
    if (is_number(v)) return "a Number";
    if (is_string(v)) return "a String";
    if (is_matrix(v)) return "a Matrix";
    return "a List";
}

std::string numberToString(double d) {
    // Print whole numbers without a trailing ".0" (e.g. "10" not "10.000000"),
    // matching how Kumu programmers expect PRINT to look.
    if (std::isfinite(d) && d == static_cast<long long>(d) &&
        std::abs(d) < 1e15) {
        return std::to_string(static_cast<long long>(d));
    }
    std::ostringstream oss;
    oss << d;
    return oss.str();
}

// Levenshtein distance, used only to suggest what a misspelled name might
// have been. Bounded work: the names involved are short.
size_t editDistance(const std::string& a, const std::string& b) {
    std::vector<size_t> prev(b.size() + 1), cur(b.size() + 1);
    for (size_t j = 0; j <= b.size(); ++j) prev[j] = j;
    for (size_t i = 1; i <= a.size(); ++i) {
        cur[0] = i;
        for (size_t j = 1; j <= b.size(); ++j) {
            size_t sub = prev[j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1);
            cur[j] = std::min({cur[j - 1] + 1, prev[j] + 1, sub});
        }
        prev = cur;
    }
    return prev[b.size()];
}

// " -- did you mean 'x'?" for the nearest candidate, or "" when nothing is
// close enough to be worth guessing at. A name that differs only in case gets
// said so explicitly, because that is a different mistake with a different fix.
std::string suggestName(const std::string& target, const std::vector<std::string>& candidates) {
    auto fold = [](std::string v) {
        std::transform(v.begin(), v.end(), v.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return v;
    };
    const std::string foldedTarget = fold(target);
    for (const std::string& c : candidates) {
        if (c != target && fold(c) == foldedTarget) {
            return " -- did you mean '" + c + "'? (names are case-sensitive)";
        }
    }
    // A name that is a prefix of a candidate, or vice versa, is almost always
    // the right guess however many edits apart they are -- LENGTH for LEN,
    // APPEND for APPEND_ROW. Three shared characters is enough to be sure.
    for (const std::string& c : candidates) {
        const std::string& shorter = c.size() < target.size() ? c : target;
        const std::string& longer = c.size() < target.size() ? target : c;
        if (shorter.size() >= 3 && shorter != longer &&
            longer.compare(0, shorter.size(), shorter) == 0) {
            return " -- did you mean '" + c + "'?";
        }
    }

    // Otherwise allow one edit for short names and two for longer ones; beyond
    // that a "suggestion" is more likely to mislead than help.
    const size_t limit = target.size() <= 4 ? 1 : 2;
    const std::string* best = nullptr;
    size_t bestDistance = limit + 1;
    for (const std::string& c : candidates) {
        size_t d = editDistance(target, c);
        if (d < bestDistance) {
            bestDistance = d;
            best = &c;
        }
    }
    return best ? " -- did you mean '" + *best + "'?" : "";
}

std::string toDisplayString(const Value& v); // defined below; listToString needs it

// Renders a List as [1, "two", [3]]. Strings are shown quoted -- inside a
// list that is the only thing distinguishing the number 1 from the text "1",
// and seeing that difference is most of the point of having both types.
std::string listToString(const List& l) {
    std::string out = "[";
    for (size_t i = 0; i < l.items.size(); ++i) {
        if (i > 0) out += ", ";
        const Value& item = l.items[i];
        out += is_string(item) ? ("\"" + as_string(item) + "\"") : toDisplayString(item);
    }
    return out + "]";
}

// Renders a Matrix as a Markdown-style table: row 1 (the header) then a
// '-'-filled rule, then each data row, columns aligned and separated by
// " | ". Column widths come from the widest cell (header included) in
// that column.
std::string matrixToString(const Matrix& m) {
    if (m.data.empty() || m.data[0].empty()) return "";
    size_t cols = m.data[0].size();

    std::vector<size_t> widths(cols, 0);
    for (const auto& row : m.data) {
        for (size_t c = 0; c < cols; ++c) {
            std::string cell = (c < row.size()) ? row[c] : std::string();
            widths[c] = std::max(widths[c], cell.size());
        }
    }

    auto renderRow = [&](const std::vector<std::string>& row) {
        std::string line;
        for (size_t c = 0; c < cols; ++c) {
            if (c > 0) line += " | ";
            std::string cell = (c < row.size()) ? row[c] : std::string();
            line += cell;
            line += std::string(widths[c] - cell.size(), ' ');
        }
        return line;
    };

    std::string result = renderRow(m.data[0]);
    result += "\n";
    for (size_t c = 0; c < cols; ++c) {
        if (c > 0) result += " | ";
        result += std::string(widths[c], '-');
    }
    for (size_t r = 1; r < m.data.size(); ++r) {
        result += "\n" + renderRow(m.data[r]);
    }
    return result;
}

std::string toDisplayString(const Value& v) {
    if (is_number(v)) return numberToString(as_number(v));
    if (is_string(v)) return as_string(v);
    if (is_list(v)) return listToString(as_list(v));
    return matrixToString(as_matrix(v));
}

// Structural equality. Recurses so nested lists compare properly; values of
// different types are simply unequal rather than an error, matching how
// Number-vs-String comparison already behaves.
bool valuesEqual(const Value& a, const Value& b) {
    if (is_number(a) && is_number(b)) return as_number(a) == as_number(b);
    if (is_string(a) && is_string(b)) return as_string(a) == as_string(b);
    if (is_list(a) && is_list(b)) {
        const List& la = as_list(a);
        const List& lb = as_list(b);
        if (la.items.size() != lb.items.size()) return false;
        for (size_t i = 0; i < la.items.size(); ++i) {
            if (!valuesEqual(la.items[i], lb.items[i])) return false;
        }
        return true;
    }
    return false;
}

double requireNumber(const Value& v, const std::string& context) {
    if (!is_number(v)) {
        throw std::runtime_error(context + " requires a number");
    }
    return as_number(v);
}

// Ordering used by both SORT implementations. std::sort requires a strict
// weak ordering, and plain `a < b` is not one when NaN is involved: NaN
// compares false against everything, including itself, which breaks the
// transitivity std::sort relies on and makes the sort undefined behaviour.
// Sorting NaN to the end keeps the ordering total and well defined.
bool numberLess(double a, double b) {
    if (std::isnan(a)) return false; // NaN is never less than anything
    if (std::isnan(b)) return true;  // ...and everything else precedes it
    return a < b;
}

// Renders one cell for CSV output. A cell holding a comma, a quote or a line
// break has to be wrapped in quotes, or reading the file back splits it in the
// wrong places -- which is exactly what used to happen: a contact noted as
// "Smith, Jr." came back as "Smith" with the rest silently dropped. Inside a
// quoted cell a literal quote is written doubled, per RFC 4180.
std::string csvEscapeCell(const std::string& cell) {
    if (cell.find_first_of(",\"\r\n") == std::string::npos) {
        return cell; // nothing that needs protecting
    }
    std::string out = "\"";
    for (char c : cell) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    out += "\"";
    return out;
}

// The matching reader. Parses the whole file at once rather than line by line,
// because a quoted cell is allowed to contain line breaks -- so "one row" is
// not the same thing as "one line".
std::vector<std::vector<std::string>> parseCsv(const std::string& text) {
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> row;
    std::string cell;
    bool inQuotes = false;
    bool rowHasContent = false; // distinguishes a blank line from an empty cell

    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];

        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') {
                    cell += '"'; // a doubled quote is one literal quote
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
                // A line break *inside* a quoted cell arrives as CRLF from a
                // file written on Windows. Drop the carriage return, exactly
                // as the between-rows case below does, so the same data reads
                // identically whichever platform wrote the file -- otherwise a
                // stray '\r' rides along inside the value.
                continue;
            } else {
                cell += c;
            }
            continue;
        }

        if (c == '"' && cell.empty()) {
            inQuotes = true;          // quotes only open a cell at its start
            rowHasContent = true;
        } else if (c == ',') {
            row.push_back(cell);
            cell.clear();
            rowHasContent = true;
        } else if (c == '\n') {
            if (rowHasContent) {
                row.push_back(cell);
                rows.push_back(row);
            } else {
                rows.push_back({}); // a blank line is a blank row
            }
            cell.clear();
            row.clear();
            rowHasContent = false;
        } else if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            continue;                 // CRLF: let the '\n' end the row
        } else {
            cell += c;
            rowHasContent = true;
        }
    }

    if (inQuotes) {
        // The file ended in the middle of a quoted cell, so its remaining
        // commas and line breaks were all swallowed as cell text. Say so
        // rather than handing back a quietly mangled table.
        throw std::runtime_error("unterminated quoted field -- a '\"' is missing its closing pair");
    }
    if (rowHasContent) { // a final row with no newline after it
        row.push_back(cell);
        rows.push_back(row);
    }
    return rows;
}

// Range-checks a 1-based position and returns it as an index.
//
// The check has to happen in double space, BEFORE the cast. Writing it the
// other way round -- cast first, then compare -- is unsound: a double too
// large for size_t converts to an unspecified value (0 on this platform), so
// a position of 1e30 passes both `pos < 1` and `index > limit` and then
// indexes wildly out of bounds. NaN fails every comparison here and so is
// rejected too, which is what we want.
bool positionInRange(double pos, size_t limit) {
    return pos >= 1.0 && pos <= static_cast<double>(limit);
}

// Clamps a Number used as a length to at most `available`, in double space
// and for the same reason. A negative or NaN length clamps to 0.
size_t clampLength(double len, size_t available) {
    if (!(len > 0.0)) return 0; // also catches NaN
    if (len >= static_cast<double>(available)) return available;
    return static_cast<size_t>(len);
}

// INPUT has no type keyword, so a line that looks entirely like a number
// (aside from surrounding whitespace) becomes a Number; anything else,
// including an empty line, is kept as a String.
bool tryParseNumber(const std::string& s, double& out) {
    if (s.empty()) return false;
    try {
        size_t idx = 0;
        double v = std::stod(s, &idx);
        while (idx < s.size() && std::isspace(static_cast<unsigned char>(s[idx]))) idx++;
        if (idx != s.size()) return false;
        // std::stod happily parses "nan", "inf" and "-infinity". Kumu Numbers
        // are ordinary finite numbers, and letting those through meant a user
        // typing "nan" at an INPUT prompt -- or a CSV cell containing it --
        // produced a value that poisoned every sum and made sorting
        // meaningless. Treat such text as text.
        if (!std::isfinite(v)) return false;
        out = v;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// Every place that runs a statement (function bodies, IF/WHILE/FOR blocks,
// the top-level program, TRY bodies) goes through this instead of calling
// stmt.execute() directly, so TRACE and line-tagged errors work everywhere
// uniformly rather than needing to be threaded through each construct.
// Points the caret at this expression. Called immediately before anything
// that can throw -- not on entry -- because a parent expression evaluates its
// children first, and whichever of them ran last would otherwise claim the
// position.
void markErrorAt(Context& ctx, const Expression& e) {
    if (e.line > 0) {
        ctx.errorLine = e.line;
        ctx.errorColumn = e.column;
    }
}

Value* lookupVariable(Context& context, const std::string& name); // defined below

void runStatement(Statement& stmt, Context& context) {
    context.currentLine = stmt.line; // so a call made here knows where it came from
    // Start each statement pointing at itself; an expression inside it will
    // narrow this to the exact operator or name if it fails.
    context.errorLine = stmt.line;
    context.errorColumn = stmt.column;

    // Indent by call depth, which turns a recursive trace from an
    // indistinguishable list of repeated lines into a readable shape.
    const std::string indent(2 * static_cast<size_t>(context.callDepth), ' ');

    // Statements that change a variable are traced AFTER they run, so the
    // trace can show the value that landed. Everything else is traced before,
    // so the order of execution reads correctly and a statement that fails is
    // still announced.
    const bool tracesValue = dynamic_cast<Assignment*>(&stmt) != nullptr ||
                              dynamic_cast<InputStatement*>(&stmt) != nullptr;

    if (context.traceEnabled && !tracesValue) {
        std::cout << indent << "[TRACE line " << stmt.line << "] " << stmt.describe() << std::endl;
    }

    try {
        stmt.execute(context);
    } catch (const TracedError&) {
        throw; // already tagged with the line where this actually happened
    } catch (const std::runtime_error& e) {
        throw TracedError(std::string(e.what()) + " (line " + std::to_string(stmt.line) + ")",
                          formatCallStack(context), context.errorLine, context.errorColumn);
    }

    if (context.traceEnabled && tracesValue) {
        std::cout << indent << "[TRACE line " << stmt.line << "] " << stmt.describe();
        const std::string* name = nullptr;
        if (auto* a = dynamic_cast<Assignment*>(&stmt)) name = &a->varName;
        else if (auto* in = dynamic_cast<InputStatement*>(&stmt)) name = &in->varName;
        if (name) {
            if (Value* v = lookupVariable(context, *name)) {
                std::cout << " = " << toDisplayString(*v);
            }
        }
        std::cout << std::endl;
    }
}

// Looks up `name`, checking the innermost active function-call scope (if
// any) before falling through to the global scope. Never consults any
// scope but the current one and global, so a callee can't see a caller's
// locals -- matching how a real call stack works.
Value* lookupVariable(Context& context, const std::string& name) {
    if (!context.locals.empty()) {
        auto& frame = context.locals.back();
        auto found = frame.find(name);
        if (found != frame.end()) return &found->second;
    }
    auto found = context.variables.find(name);
    if (found != context.variables.end()) return &found->second;
    return nullptr;
}

// Assigns `name` = value. A declaration (VAR, a FOR loop's counter, or a
// caught error's binding) always writes into the current scope -- the
// innermost active call frame, or global at the top level -- shadowing
// anything outside it for the rest of that call. A bare assignment instead
// updates whichever existing binding is visible (current frame, else
// global), or creates a new global if neither has it yet -- this is what
// lets a function deliberately reach out and mutate global state without
// declaring every such variable as a parameter.
void assignVariable(Context& context, const std::string& name, Value value, bool isDeclaration) {
    if (isDeclaration) {
        if (!context.locals.empty()) {
            context.locals.back()[name] = std::move(value);
        } else {
            context.variables[name] = std::move(value);
        }
        return;
    }
    if (!context.locals.empty()) {
        auto& frame = context.locals.back();
        auto found = frame.find(name);
        if (found != frame.end()) {
            found->second = std::move(value);
            return;
        }
    }
    context.variables[name] = std::move(value);
}

} // namespace

// ---- Expressions ----

Value VariableReference::evaluate(Context& context) {
    Value* v = lookupVariable(context, name);
    if (!v) {
        markErrorAt(context, *this);
        // Gather every name actually visible from here, so the suggestion can
        // only ever point at something that exists.
        std::vector<std::string> visible;
        if (!context.locals.empty()) {
            for (const auto& entry : context.locals.back()) visible.push_back(entry.first);
        }
        for (const auto& entry : context.variables) visible.push_back(entry.first);
        throw std::runtime_error("Undefined variable: " + name + suggestName(name, visible));
    }
    return *v;
}

Value BinaryExpression::evaluate(Context& context) {
    Value l = left->evaluate(context);
    Value r = right->evaluate(context);
    markErrorAt(context, *this); // operands are done; anything below is ours

    // '+' concatenates whenever either side is a string.
    if (op == "+" && (is_string(l) || is_string(r))) {
        return toDisplayString(l) + toDisplayString(r);
    }

    if (is_number(l) && is_number(r)) {
        double a = as_number(l);
        double b = as_number(r);
        // Overflowing to infinity used to pass silently, and inf minus inf is
        // NaN -- a value that poisons comparisons, sorting and every later
        // sum. POW already refuses to produce a non-finite result; these do
        // too, so that a Kumu Number is always an ordinary finite number.
        auto checked = [this](double result) {
            if (!std::isfinite(result)) {
                throw std::runtime_error("Number overflow in '" + op +
                                          "': the result is too large to represent");
            }
            return result;
        };
        if (op == "+") return checked(a + b);
        if (op == "-") return checked(a - b);
        if (op == "*") return checked(a * b);
        if (op == "/") {
            if (b == 0.0) throw std::runtime_error("Division by zero");
            return checked(a / b);
        }
        if (op == "MOD") {
            if (b == 0.0) throw std::runtime_error("Division by zero in MOD");
            return std::fmod(a, b);
        }
        if (op == "==") return (a == b) ? 1.0 : 0.0;
        if (op == "!=") return (a != b) ? 1.0 : 0.0;
        if (op == "<") return (a < b) ? 1.0 : 0.0;
        if (op == ">") return (a > b) ? 1.0 : 0.0;
        if (op == "<=") return (a <= b) ? 1.0 : 0.0;
        if (op == ">=") return (a >= b) ? 1.0 : 0.0;
    }

    if (is_string(l) && is_string(r)) {
        const std::string& a = as_string(l);
        const std::string& b = as_string(r);
        if (op == "==") return (a == b) ? 1.0 : 0.0;
        if (op == "!=") return (a != b) ? 1.0 : 0.0;
        if (op == "<") return (a < b) ? 1.0 : 0.0;
        if (op == ">") return (a > b) ? 1.0 : 0.0;
        if (op == "<=") return (a <= b) ? 1.0 : 0.0;
        if (op == ">=") return (a >= b) ? 1.0 : 0.0;
    }

    // Lists compare element-wise, and only for equality: there is no obvious
    // meaning for one list being "less than" another, so ordering stays an
    // error exactly as it is for Matrices.
    if (is_list(l) || is_list(r)) {
        if (op == "==") return valuesEqual(l, r) ? 1.0 : 0.0;
        if (op == "!=") return valuesEqual(l, r) ? 0.0 : 1.0;
    }

    // Mismatched scalar types (e.g. a Number compared to a String) are
    // simply unequal -- important for values read via INPUT, whose type
    // depends on what the user typed and so isn't known when the program
    // was written. Matrices are excluded: there's no defined equality for
    // them, so comparing one still falls through to the error below.
    if (!is_matrix(l) && !is_matrix(r)) {
        if (op == "==") return 0.0;
        if (op == "!=") return 1.0;
    }

    // Name what was actually there. "Invalid operand types" told the reader
    // only that something was wrong, never which side or what it held.
    std::string hint;
    if ((is_string(l) || is_string(r)) && (op == "-" || op == "*" || op == "/" || op == "MOD")) {
        hint = " -- only '+' joins text";
    } else if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        hint = " -- both sides must be the same type, and Matrices cannot be ordered";
    }
    throw std::runtime_error("Cannot use '" + op + "' on " + valueKind(l) +
                              " and " + valueKind(r) + hint);
}

Value ListLiteral::evaluate(Context& context) {
    markErrorAt(context, *this);
    std::vector<Value> values;
    values.reserve(elements.size());
    for (auto& e : elements) values.push_back(e->evaluate(context));
    return make_list(std::move(values));
}

Value IndexExpression::evaluate(Context& context) {
    Value receiverVal = receiver->evaluate(context);
    markErrorAt(context, *this);
    if (!is_list(receiverVal)) {
        throw std::runtime_error("Cannot use [position] on " + valueKind(receiverVal) +
                                  " -- only a List can be indexed");
    }
    const List& l = as_list(receiverVal);
    double idx = requireNumber(index->evaluate(context), "A list position");
    markErrorAt(context, *this); // the index expression may have moved it
    // 1-based, like SUBSTRING, INDEX_OF and Matrix row numbers. Consistency
    // inside the language matters more here than matching any other one.
    if (!positionInRange(idx, l.items.size())) {
        throw std::runtime_error("Position " + numberToString(idx) + " is out of range (the list has " +
                                  std::to_string(l.items.size()) + " item(s))");
    }
    return l.items[static_cast<size_t>(idx) - 1];
}

Value LogicalExpression::evaluate(Context& context) {
    bool leftTruthy = isTruthy(left->evaluate(context));
    // Short-circuit: the right operand is only evaluated when it can still
    // change the answer. That is what makes a guard like
    // `IF n != 0 AND total / n > 5` safe rather than a division by zero.
    if (op == "AND") {
        if (!leftTruthy) return 0.0;
        return isTruthy(right->evaluate(context)) ? 1.0 : 0.0;
    }
    if (op == "OR") {
        if (leftTruthy) return 1.0;
        return isTruthy(right->evaluate(context)) ? 1.0 : 0.0;
    }
    throw std::runtime_error("Unknown logical operator: " + op);
}

Value UnaryExpression::evaluate(Context& context) {
    Value v = expr->evaluate(context);
    markErrorAt(context, *this);
    if (op == "-") {
        return -requireNumber(v, "Unary '-'");
    }
    if (op == "!") {
        return isTruthy(v) ? 0.0 : 1.0;
    }
    throw std::runtime_error("Unknown unary operator: " + op);
}

namespace {

Value evaluateBuiltin(const std::string& name,
                       const std::vector<std::unique_ptr<Expression>>& args,
                       Context& context) {
    if (name == "READ_FILE") {
        if (args.size() != 1) throw std::runtime_error("READ_FILE expects 1 argument (path)");
        Value pathVal = args[0]->evaluate(context);
        if (!is_string(pathVal)) throw std::runtime_error("READ_FILE expects a string path");
        std::ifstream file(as_string(pathVal));
        if (!file.is_open()) throw std::runtime_error("READ_FILE: could not open file '" + as_string(pathVal) + "'");
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    if (name == "WRITE_FILE") {
        if (args.size() != 2) throw std::runtime_error("WRITE_FILE expects 2 arguments (path, content)");
        Value pathVal = args[0]->evaluate(context);
        Value contentVal = args[1]->evaluate(context);
        if (!is_string(pathVal)) throw std::runtime_error("WRITE_FILE expects a string path");
        std::ofstream file(as_string(pathVal));
        if (!file.is_open()) throw std::runtime_error("WRITE_FILE: could not open file '" + as_string(pathVal) + "'");
        file << toDisplayString(contentVal);
        return 1.0;
    }

    if (name == "READ_CSV") {
        if (args.size() != 1) throw std::runtime_error("READ_CSV expects 1 argument (path)");
        Value pathVal = args[0]->evaluate(context);
        if (!is_string(pathVal)) throw std::runtime_error("READ_CSV expects a string path");
        std::ifstream file(as_string(pathVal));
        if (!file.is_open()) throw std::runtime_error("READ_CSV: could not open file '" + as_string(pathVal) + "'");
        std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        Matrix m;
        try {
            m.data = parseCsv(text);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("READ_CSV: " + as_string(pathVal) + ": " + e.what());
        }
        return m;
    }

    if (name == "FILE_EXISTS") {
        if (args.size() != 1) throw std::runtime_error("FILE_EXISTS expects 1 argument (path)");
        Value pathVal = args[0]->evaluate(context);
        if (!is_string(pathVal)) throw std::runtime_error("FILE_EXISTS expects a string path");
        return std::filesystem::exists(as_string(pathVal)) ? 1.0 : 0.0;
    }

    if (name == "LEN") {
        if (args.size() != 1) throw std::runtime_error("LEN expects 1 argument");
        Value v = args[0]->evaluate(context);
        // "How many things are in this?" -- one question, one word, whatever
        // kind of collection it is. Matrices report data rows, never counting
        // the header, matching .ROWS().
        if (is_string(v)) return static_cast<double>(as_string(v).size());
        if (is_list(v)) return static_cast<double>(as_list(v).items.size());
        if (is_matrix(v)) {
            const Matrix m = as_matrix(v);
            return static_cast<double>(m.data.empty() ? 0 : m.data.size() - 1);
        }
        throw std::runtime_error("LEN expects a String, a List or a Matrix");
    }

    if (name == "SUBSTRING") {
        if (args.size() != 3) throw std::runtime_error("SUBSTRING expects 3 arguments (string, start, length)");
        Value sVal = args[0]->evaluate(context);
        Value startVal = args[1]->evaluate(context);
        Value lenVal = args[2]->evaluate(context);
        if (!is_string(sVal)) throw std::runtime_error("SUBSTRING expects a string");
        double startD = requireNumber(startVal, "SUBSTRING start");
        double lenD = requireNumber(lenVal, "SUBSTRING length");
        const std::string& s = as_string(sVal);
        // 1-indexed, like everywhere else; start == length()+1 yields "".
        if (startD < 1 || startD > static_cast<double>(s.size()) + 1) {
            throw std::runtime_error("SUBSTRING: start " + numberToString(startD) + " is out of range");
        }
        if (lenD < 0) throw std::runtime_error("SUBSTRING: length cannot be negative");
        size_t start = static_cast<size_t>(startD) - 1; // startD already range-checked above
        size_t available = s.size() - start;
        return s.substr(start, clampLength(lenD, available));
    }

    if (name == "INDEX_OF") {
        if (args.size() != 2) throw std::runtime_error("INDEX_OF expects 2 arguments (string, search)");
        Value sVal = args[0]->evaluate(context);
        Value searchVal = args[1]->evaluate(context);
        if (!is_string(sVal) || !is_string(searchVal)) throw std::runtime_error("INDEX_OF expects two strings");
        size_t pos = as_string(sVal).find(as_string(searchVal));
        // 0 means "not found", matching the same convention used throughout
        // this dialect's own address-book-style lookup functions.
        return pos == std::string::npos ? 0.0 : static_cast<double>(pos + 1);
    }

    if (name == "UPPER" || name == "LOWER") {
        if (args.size() != 1) throw std::runtime_error(name + " expects 1 argument (string)");
        Value sVal = args[0]->evaluate(context);
        if (!is_string(sVal)) throw std::runtime_error(name + " expects a string");
        std::string s = as_string(sVal);
        std::transform(s.begin(), s.end(), s.begin(), [&](unsigned char c) {
            return name == "UPPER" ? std::toupper(c) : std::tolower(c);
        });
        return s;
    }

    if (name == "RANDOM") {
        if (args.size() != 2) throw std::runtime_error("RANDOM expects 2 arguments (min, max)");
        Value minVal = args[0]->evaluate(context);
        Value maxVal = args[1]->evaluate(context);
        double minD = requireNumber(minVal, "RANDOM min");
        double maxD = requireNumber(maxVal, "RANDOM max");
        // Checked before the cast, for the same reason positions are: a bound
        // outside long long's range converts to an unspecified value, and
        // RANDOM(1e30, 1e30) would then quietly return a huge negative number.
        // 1e15 is the same threshold numberToString uses for whole numbers.
        if (!std::isfinite(minD) || !std::isfinite(maxD) ||
            std::fabs(minD) > 1e15 || std::fabs(maxD) > 1e15) {
            throw std::runtime_error("RANDOM: min and max must be whole numbers no larger than 1e15");
        }
        long long lo = static_cast<long long>(minD);
        long long hi = static_cast<long long>(maxD);
        if (lo > hi) throw std::runtime_error("RANDOM: min must be <= max");
        // Seeded once (function-local static) rather than every call, so
        // consecutive calls don't risk repeating on a fast clock.
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<long long> dist(lo, hi);
        return static_cast<double>(dist(rng));
    }

    if (name == "CLEAR_SCREEN") {
        if (!args.empty()) throw std::runtime_error("CLEAR_SCREEN expects no arguments");
        // Standard ANSI sequence: clear the whole screen, then home the
        // cursor to row 1, column 1. Works in any terminal that understands
        // ANSI escapes (essentially all of them on Linux and macOS, and
        // modern Windows terminals).
        std::cout << "\033[2J\033[H" << std::flush;
        return 1.0;
    }

    if (name == "SLEEP") {
        if (args.size() != 1) throw std::runtime_error("SLEEP expects 1 argument (seconds)");
        Value secondsVal = args[0]->evaluate(context);
        double seconds = requireNumber(secondsVal, "SLEEP");
        if (seconds < 0) throw std::runtime_error("SLEEP: duration cannot be negative");
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
        return 1.0;
    }

    if (name == "ABS" || name == "FLOOR" || name == "ROUND" || name == "SQRT") {
        if (args.size() != 1) throw std::runtime_error(name + " expects 1 argument (number)");
        double x = requireNumber(args[0]->evaluate(context), name);
        if (name == "ABS") return std::fabs(x);
        if (name == "FLOOR") return std::floor(x);
        if (name == "ROUND") return std::round(x);
        if (x < 0) throw std::runtime_error("SQRT: cannot take the square root of a negative number");
        return std::sqrt(x);
    }

    if (name == "POW") {
        if (args.size() != 2) throw std::runtime_error("POW expects 2 arguments (base, exponent)");
        double base = requireNumber(args[0]->evaluate(context), "POW base");
        double exp = requireNumber(args[1]->evaluate(context), "POW exponent");
        double result = std::pow(base, exp);
        if (!std::isfinite(result)) {
            throw std::runtime_error("POW: the result is not a valid number");
        }
        return result;
    }

    if (name == "IS_NUMBER") {
        if (args.size() != 1) throw std::runtime_error("IS_NUMBER expects 1 argument");
        Value v = args[0]->evaluate(context);
        // Answers "could this be used as a number?", so it says yes both to a
        // Number and to text that reads as one. That is the question worth
        // asking after INPUT, whose result is a Number only when the user
        // happened to type digits, and after reading a CSV, where every cell
        // arrives as text.
        if (is_number(v)) return 1.0;
        double parsed;
        return (is_string(v) && tryParseNumber(as_string(v), parsed)) ? 1.0 : 0.0;
    }

    if (name == "NUMBER") {
        if (args.size() != 1) throw std::runtime_error("NUMBER expects 1 argument");
        Value v = args[0]->evaluate(context);
        if (is_number(v)) return v;
        double parsed;
        if (is_string(v) && tryParseNumber(as_string(v), parsed)) return parsed;
        throw std::runtime_error("NUMBER: '" + toDisplayString(v) + "' is not a number -- "
                                  "check it with IS_NUMBER first");
    }

    if (name == "STRING") {
        if (args.size() != 1) throw std::runtime_error("STRING expects 1 argument");
        return toDisplayString(args[0]->evaluate(context));
    }

    if (name == "TRIM") {
        if (args.size() != 1) throw std::runtime_error("TRIM expects 1 argument (string)");
        Value v = args[0]->evaluate(context);
        if (!is_string(v)) throw std::runtime_error("TRIM expects a string");
        const std::string& str = as_string(v);
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return std::string();
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }

    if (name == "CONTAINS" || name == "STARTS_WITH" || name == "ENDS_WITH") {
        if (args.size() != 2) throw std::runtime_error(name + " expects 2 arguments (string, text)");
        Value sVal = args[0]->evaluate(context);
        Value tVal = args[1]->evaluate(context);
        if (!is_string(sVal) || !is_string(tVal)) throw std::runtime_error(name + " expects two strings");
        const std::string& str = as_string(sVal);
        const std::string& sub = as_string(tVal);
        if (name == "CONTAINS") return str.find(sub) != std::string::npos ? 1.0 : 0.0;
        if (sub.size() > str.size()) return 0.0;
        if (name == "STARTS_WITH") return str.compare(0, sub.size(), sub) == 0 ? 1.0 : 0.0;
        return str.compare(str.size() - sub.size(), sub.size(), sub) == 0 ? 1.0 : 0.0;
    }

    if (name == "REPLACE") {
        if (args.size() != 3) throw std::runtime_error("REPLACE expects 3 arguments (string, find, replacement)");
        Value sVal = args[0]->evaluate(context);
        Value fVal = args[1]->evaluate(context);
        Value rVal = args[2]->evaluate(context);
        if (!is_string(sVal) || !is_string(fVal) || !is_string(rVal)) {
            throw std::runtime_error("REPLACE expects three strings");
        }
        const std::string& find = as_string(fVal);
        if (find.empty()) throw std::runtime_error("REPLACE: the text to find cannot be empty");
        const std::string& repl = as_string(rVal);
        std::string out = as_string(sVal);
        size_t at = 0;
        // Advancing past the replacement (rather than re-scanning from `at`)
        // is what stops REPLACE(s, "a", "aa") from looping forever.
        while ((at = out.find(find, at)) != std::string::npos) {
            out.replace(at, find.size(), repl);
            at += repl.size();
        }
        return out;
    }

    if (name == "SPLIT") {
        if (args.size() != 2) throw std::runtime_error("SPLIT expects 2 arguments (string, separator)");
        Value sVal = args[0]->evaluate(context);
        Value sepVal = args[1]->evaluate(context);
        if (!is_string(sVal) || !is_string(sepVal)) throw std::runtime_error("SPLIT expects two strings");
        const std::string& str = as_string(sVal);
        const std::string& sep = as_string(sepVal);
        std::vector<Value> parts;
        if (sep.empty()) {
            // An empty separator splits into single characters, which is the
            // obvious reading and is useful for letter-by-letter exercises.
            for (char c : str) parts.push_back(std::string(1, c));
            return make_list(std::move(parts));
        }
        size_t start = 0;
        while (true) {
            size_t at = str.find(sep, start);
            if (at == std::string::npos) {
                parts.push_back(str.substr(start));
                break;
            }
            parts.push_back(str.substr(start, at - start));
            start = at + sep.size();
        }
        return make_list(std::move(parts));
    }

    throw std::runtime_error("Undefined function: " + name); // sentinel: not a builtin
}

const char* const kBuiltinNames[] = {"READ_FILE", "WRITE_FILE", "READ_CSV",
                                      "FILE_EXISTS", "LEN", "SUBSTRING", "INDEX_OF",
                                      "UPPER", "LOWER", "RANDOM", "CLEAR_SCREEN", "SLEEP",
                                      "ABS", "FLOOR", "ROUND", "SQRT", "POW",
                                      "IS_NUMBER", "NUMBER", "STRING",
                                      "TRIM", "REPLACE", "CONTAINS", "STARTS_WITH", "ENDS_WITH",
                                      "SPLIT"};

bool isBuiltin(const std::string& name) {
    for (const char* b : kBuiltinNames) {
        if (name == b) return true;
    }
    return false;
}

} // namespace

Value CallExpression::evaluate(Context& context) {
    markErrorAt(context, *this);
    if (isBuiltin(name)) {
        return evaluateBuiltin(name, args, context);
    }

    auto it = context.functions.find(name);
    if (it == context.functions.end()) {
        std::vector<std::string> known;
        for (const auto& entry : context.functions) known.push_back(entry.first);
        for (const char* b : kBuiltinNames) known.push_back(b);
        throw std::runtime_error("Undefined function: " + name + suggestName(name, known));
    }
    const std::vector<std::string>& params = it->second.first;
    if (args.size() != params.size()) {
        throw std::runtime_error("Function '" + name + "' expects " +
                                  std::to_string(params.size()) +
                                  " argument(s), got " +
                                  std::to_string(args.size()));
    }

    // Counts this call before anything else happens, so the depth limit trips
    // on the way in rather than after another frame's worth of work. Unwinds
    // correctly on any exit path -- return, error, or ReturnSignal.
    CallDepthGuard depthGuard(context, name);

    // Evaluate arguments in the caller's scope before entering the callee's.
    std::vector<Value> argValues;
    argValues.reserve(args.size());
    for (auto& arg : args) {
        argValues.push_back(arg->evaluate(context));
    }

    // Push a fresh scope for this call and bind the parameters into it.
    // Pushing a frame (rather than saving and restoring each parameter by
    // name) is what makes recursion just work, and what keeps a function's
    // own VAR-declared locals from leaking into or colliding with whatever
    // called it: each active call gets its own frame, so a recursive call's
    // "n" -- or any function's own "row" or "total" -- never collides with
    // an unrelated variable of the same name anywhere else on the stack.
    context.locals.push_back({});
    for (size_t i = 0; i < params.size(); ++i) {
        context.locals.back()[params[i]] = std::move(argValues[i]);
    }

    Value result = 0.0;
    try {
        for (auto& stmt : it->second.second) {
            runStatement(*stmt, context);
        }
    } catch (ReturnSignal& ret) {
        result = std::move(ret.value);
    } catch (const BreakSignal&) {
        // Without this, a stray BREAK inside a function would sail out of the
        // call and break a loop in the *caller* -- action at a distance.
        context.locals.pop_back();
        throw std::runtime_error("BREAK used outside of a loop");
    } catch (const ContinueSignal&) {
        context.locals.pop_back();
        throw std::runtime_error("CONTINUE used outside of a loop");
    } catch (...) {
        context.locals.pop_back(); // still pop the frame on any other error
        throw;
    }

    context.locals.pop_back();
    return result;
}

namespace {

// Resolves a column selector -- a 1-based Number position, or a String
// matching a header cell exactly -- to a 1-based column position. Every
// Matrix method goes through this, which is what makes GET(1, "Score")
// and GET(1, 2) equivalent. Requires at least a header row.
size_t resolveColumn(const Matrix& m, const Value& selector) {
    if (m.data.empty()) {
        throw std::runtime_error("matrix has no header row");
    }
    if (is_number(selector)) {
        double c = as_number(selector);
        if (!positionInRange(c, m.data[0].size())) {
            throw std::runtime_error("column " + numberToString(c) + " is out of range (the matrix has " +
                                      std::to_string(m.data[0].size()) + " column(s))");
        }
        return static_cast<size_t>(c);
    }
    if (is_string(selector)) {
        const std::string& colName = as_string(selector);
        for (size_t i = 0; i < m.data[0].size(); ++i) {
            if (m.data[0][i] == colName) return i + 1;
        }
        std::vector<std::string> headers(m.data[0].begin(), m.data[0].end());
        std::string names;
        for (size_t i = 0; i < headers.size(); ++i) {
            if (i > 0) names += ", ";
            names += headers[i];
        }
        throw std::runtime_error("no column named '" + colName + "' (the columns are: " +
                                  names + ")" + suggestName(colName, headers));
    }
    throw std::runtime_error("a column must be a Number (position) or a String (header name)");
}

// Resolves a 1-based DATA row number (the header is never counted) to the
// physical index into m.data -- which, conveniently, is the same number:
// m.data[0] is the header, so m.data[1] is data row 1, m.data[N] is data
// row N. Requires at least a header row.
size_t resolveDataRowIndex(const Matrix& m, double rowNumber) {
    if (m.data.empty()) {
        throw std::runtime_error("matrix has no header row");
    }
    size_t dataRows = m.data.size() - 1;
    if (!positionInRange(rowNumber, dataRows)) {
        throw std::runtime_error("row " + numberToString(rowNumber) + " is out of range");
    }
    return static_cast<size_t>(rowNumber);
}

// Strict numeric column read, used by SUM/AVERAGE/MIN/MAX: every data-row
// cell in the column must parse as a number, or this throws naming the
// offending row and value.
std::vector<double> collectNumericColumn(const Matrix& m, size_t col, const std::string& methodName) {
    std::vector<double> values;
    for (size_t r = 1; r < m.data.size(); ++r) {
        if (col > m.data[r].size()) {
            throw std::runtime_error(methodName + ": row " + std::to_string(r) + " has no column " + std::to_string(col));
        }
        const std::string& cell = m.data[r][col - 1];
        double num;
        if (!tryParseNumber(cell, num)) {
            throw std::runtime_error(methodName + ": row " + std::to_string(r) + ", column " +
                                      std::to_string(col) + " ('" + cell + "') is not a number");
        }
        values.push_back(num);
    }
    return values;
}

// Reads a list as numbers for SUM/MIN/MAX/AVERAGE. Numeric strings are
// accepted as well as Numbers, so a list that came out of SPLIT or a CSV
// column adds up without the caller converting it element by element --
// the same leniency Matrix's numeric columns already have.
std::vector<double> listNumericValues(const List& l, const std::string& methodName) {
    std::vector<double> values;
    values.reserve(l.items.size());
    for (size_t i = 0; i < l.items.size(); ++i) {
        const Value& item = l.items[i];
        if (is_number(item)) {
            values.push_back(as_number(item));
            continue;
        }
        double parsed;
        if (is_string(item) && tryParseNumber(as_string(item), parsed)) {
            values.push_back(parsed);
            continue;
        }
        throw std::runtime_error(methodName + ": item " + std::to_string(i + 1) + " (" +
                                  toDisplayString(item) + ") is not a number");
    }
    return values;
}

// Resolves a 1-based position argument, allowing `size + 1` when `allowEnd`
// is set (INSERT needs to be able to append).
size_t resolveListPosition(const List& l, double pos, const std::string& methodName, bool allowEnd = false) {
    size_t limit = l.items.size() + (allowEnd ? 1 : 0);
    if (!positionInRange(pos, limit)) {
        throw std::runtime_error(methodName + ": position " + numberToString(pos) +
                                  " is out of range (the list has " + std::to_string(l.items.size()) +
                                  " item(s))");
    }
    return static_cast<size_t>(pos);
}

Value evaluateListMethod(const std::string& method,
                          const std::vector<std::unique_ptr<Expression>>& args,
                          const List& l, Context& context) {
    // Every mutator below builds a fresh vector and hands it to make_list;
    // none of them touch `l`. That is the whole immutability rule, and it is
    // enforced here by `l` being const rather than by convention.
    if (method == "LEN") {
        if (!args.empty()) throw std::runtime_error("LEN expects no arguments");
        return static_cast<double>(l.items.size());
    }

    if (method == "GET") {
        if (args.size() != 1) throw std::runtime_error("GET expects 1 argument (position)");
        double pos = requireNumber(args[0]->evaluate(context), "GET position");
        return l.items[resolveListPosition(l, pos, "GET") - 1];
    }

    if (method == "SET") {
        if (args.size() != 2) throw std::runtime_error("SET expects 2 arguments (position, value)");
        double pos = requireNumber(args[0]->evaluate(context), "SET position");
        Value newVal = args[1]->evaluate(context);
        size_t at = resolveListPosition(l, pos, "SET");
        std::vector<Value> items = l.items;
        items[at - 1] = std::move(newVal);
        return make_list(std::move(items));
    }

    if (method == "APPEND") {
        if (args.empty()) throw std::runtime_error("APPEND expects at least one value");
        std::vector<Value> items = l.items;
        for (auto& a : args) items.push_back(a->evaluate(context));
        return make_list(std::move(items));
    }

    if (method == "INSERT") {
        if (args.size() != 2) throw std::runtime_error("INSERT expects 2 arguments (position, value)");
        double pos = requireNumber(args[0]->evaluate(context), "INSERT position");
        Value newVal = args[1]->evaluate(context);
        size_t at = resolveListPosition(l, pos, "INSERT", /*allowEnd=*/true);
        std::vector<Value> items = l.items;
        items.insert(items.begin() + static_cast<std::ptrdiff_t>(at - 1), std::move(newVal));
        return make_list(std::move(items));
    }

    if (method == "REMOVE") {
        if (args.size() != 1) throw std::runtime_error("REMOVE expects 1 argument (position)");
        double pos = requireNumber(args[0]->evaluate(context), "REMOVE position");
        size_t at = resolveListPosition(l, pos, "REMOVE");
        std::vector<Value> items = l.items;
        items.erase(items.begin() + static_cast<std::ptrdiff_t>(at - 1));
        return make_list(std::move(items));
    }

    if (method == "CONTAINS" || method == "INDEX_OF") {
        if (args.size() != 1) throw std::runtime_error(method + " expects 1 argument (value)");
        Value target = args[0]->evaluate(context);
        for (size_t i = 0; i < l.items.size(); ++i) {
            if (valuesEqual(l.items[i], target)) {
                return method == "CONTAINS" ? 1.0 : static_cast<double>(i + 1);
            }
        }
        return 0.0; // INDEX_OF: 0 means "not found", as it does for strings
    }

    if (method == "REVERSE") {
        if (!args.empty()) throw std::runtime_error("REVERSE expects no arguments");
        std::vector<Value> items(l.items.rbegin(), l.items.rend());
        return make_list(std::move(items));
    }

    if (method == "SORT") {
        if (!args.empty()) throw std::runtime_error("SORT expects no arguments");
        // Same auto-detection Matrix.SORT uses: numeric when every item reads
        // as a number, otherwise ordinary text order.
        bool allNumeric = true;
        for (const Value& item : l.items) {
            double dummy;
            if (is_number(item)) continue;
            if (is_string(item) && tryParseNumber(as_string(item), dummy)) continue;
            allNumeric = false;
            break;
        }
        std::vector<Value> items = l.items;
        std::sort(items.begin(), items.end(), [allNumeric](const Value& a, const Value& b) {
            if (allNumeric) {
                double da = 0.0, db = 0.0;
                if (is_number(a)) da = as_number(a); else tryParseNumber(as_string(a), da);
                if (is_number(b)) db = as_number(b); else tryParseNumber(as_string(b), db);
                return numberLess(da, db);
            }
            return toDisplayString(a) < toDisplayString(b);
        });
        return make_list(std::move(items));
    }

    if (method == "SLICE") {
        if (args.size() != 2) throw std::runtime_error("SLICE expects 2 arguments (start, length)");
        double startD = requireNumber(args[0]->evaluate(context), "SLICE start");
        double lenD = requireNumber(args[1]->evaluate(context), "SLICE length");
        if (lenD < 0) throw std::runtime_error("SLICE: length cannot be negative");
        // start == size + 1 yields an empty list, mirroring SUBSTRING.
        size_t start = resolveListPosition(l, startD, "SLICE", /*allowEnd=*/true);
        size_t available = l.items.size() - (start - 1);
        size_t take = clampLength(lenD, available);
        std::vector<Value> items(l.items.begin() + static_cast<std::ptrdiff_t>(start - 1),
                                 l.items.begin() + static_cast<std::ptrdiff_t>(start - 1 + take));
        return make_list(std::move(items));
    }

    if (method == "JOIN") {
        if (args.size() != 1) throw std::runtime_error("JOIN expects 1 argument (separator)");
        Value sepVal = args[0]->evaluate(context);
        if (!is_string(sepVal)) throw std::runtime_error("JOIN expects a string separator");
        const std::string& sep = as_string(sepVal);
        std::string out;
        for (size_t i = 0; i < l.items.size(); ++i) {
            if (i > 0) out += sep;
            out += toDisplayString(l.items[i]); // unquoted, unlike list display
        }
        return out;
    }

    if (method == "SUM" || method == "AVERAGE" || method == "MIN" || method == "MAX") {
        if (!args.empty()) throw std::runtime_error(method + " expects no arguments");
        std::vector<double> values = listNumericValues(l, method);
        if (method == "SUM") {
            double total = 0.0;
            for (double v : values) total += v;
            return total;
        }
        if (values.empty()) throw std::runtime_error(method + ": the list is empty");
        if (method == "AVERAGE") {
            double total = 0.0;
            for (double v : values) total += v;
            return total / static_cast<double>(values.size());
        }
        if (method == "MIN") return *std::min_element(values.begin(), values.end());
        return *std::max_element(values.begin(), values.end()); // MAX
    }

    static const std::vector<std::string> kListMethods = {
        "LEN", "GET", "SET", "APPEND", "INSERT", "REMOVE", "CONTAINS", "INDEX_OF",
        "REVERSE", "SORT", "SLICE", "JOIN", "SUM", "AVERAGE", "MIN", "MAX"};
    throw std::runtime_error("List has no method '" + method + "'" +
                              suggestName(method, kListMethods));
}

} // namespace

Value MethodCallExpression::evaluate(Context& context) {
    Value receiverVal = receiver->evaluate(context);
    markErrorAt(context, *this);
    if (is_list(receiverVal)) {
        return evaluateListMethod(method, args, as_list(receiverVal), context);
    }
    if (!is_matrix(receiverVal)) {
        throw std::runtime_error("Cannot call method '" + method + "' on " + valueKind(receiverVal));
    }
    // Every method works on its own copy -- consistent with this language's
    // existing rule that Matrix values are never mutated in place; a
    // mutator (SET, APPEND_ROW, DELETE_ROW, SORT) returns this modified
    // copy, and the caller reassigns if they want the change to stick.
    Matrix m = as_matrix(receiverVal);

    if (method == "GET") {
        if (args.size() != 2) throw std::runtime_error("GET expects 2 arguments (row, col)");
        double rowNum = requireNumber(args[0]->evaluate(context), "GET row");
        Value colSel = args[1]->evaluate(context);
        size_t rowIdx = resolveDataRowIndex(m, rowNum);
        size_t col = resolveColumn(m, colSel);
        if (col > m.data[rowIdx].size()) {
            throw std::runtime_error("GET: row " + numberToString(rowNum) + " has no column " + std::to_string(col));
        }
        return m.data[rowIdx][col - 1];
    }

    if (method == "SET") {
        if (args.size() != 3) throw std::runtime_error("SET expects 3 arguments (row, col, value)");
        double rowNum = requireNumber(args[0]->evaluate(context), "SET row");
        Value colSel = args[1]->evaluate(context);
        Value newVal = args[2]->evaluate(context);
        size_t rowIdx = resolveDataRowIndex(m, rowNum);
        size_t col = resolveColumn(m, colSel);
        if (col > m.data[rowIdx].size()) {
            throw std::runtime_error("SET: row " + numberToString(rowNum) + " has no column " + std::to_string(col));
        }
        m.data[rowIdx][col - 1] = toDisplayString(newVal);
        return m;
    }

    if (method == "ROWS") {
        if (!args.empty()) throw std::runtime_error("ROWS expects no arguments");
        return static_cast<double>(m.data.empty() ? 0 : m.data.size() - 1);
    }

    if (method == "COLS") {
        if (!args.empty()) throw std::runtime_error("COLS expects no arguments");
        return static_cast<double>(m.data.empty() ? 0 : m.data[0].size());
    }

    if (method == "APPEND_ROW") {
        if (args.empty()) throw std::runtime_error("APPEND_ROW expects at least one cell value");
        std::vector<std::string> row;
        row.reserve(args.size());
        for (auto& a : args) row.push_back(toDisplayString(a->evaluate(context)));
        if (!m.data.empty() && row.size() != m.data[0].size()) {
            throw std::runtime_error("APPEND_ROW: expected " + std::to_string(m.data[0].size()) +
                                      " cell(s), got " + std::to_string(row.size()));
        }
        m.data.push_back(std::move(row));
        return m;
    }

    if (method == "DELETE_ROW") {
        if (args.size() != 1) throw std::runtime_error("DELETE_ROW expects 1 argument (row)");
        double rowNum = requireNumber(args[0]->evaluate(context), "DELETE_ROW row");
        size_t rowIdx = resolveDataRowIndex(m, rowNum);
        m.data.erase(m.data.begin() + static_cast<std::ptrdiff_t>(rowIdx));
        return m;
    }

    if (method == "SUM" || method == "AVERAGE" || method == "MIN" || method == "MAX") {
        if (args.size() != 1) throw std::runtime_error(method + " expects 1 argument (column)");
        Value colSel = args[0]->evaluate(context);
        size_t col = resolveColumn(m, colSel);
        std::vector<double> values = collectNumericColumn(m, col, method);
        if (method == "SUM") {
            double total = 0.0;
            for (double v : values) total += v;
            return total;
        }
        if (values.empty()) throw std::runtime_error(method + ": no data rows");
        if (method == "AVERAGE") {
            double total = 0.0;
            for (double v : values) total += v;
            return total / static_cast<double>(values.size());
        }
        if (method == "MIN") return *std::min_element(values.begin(), values.end());
        return *std::max_element(values.begin(), values.end()); // MAX
    }

    if (method == "COUNT") {
        // Deliberately lenient (unlike SUM/AVERAGE/MIN/MAX): counts cells
        // that parse as numbers, silently skipping blanks and non-numeric
        // text like "N/A" -- Excel's actual COUNT, not COUNTA.
        if (args.size() != 1) throw std::runtime_error("COUNT expects 1 argument (column)");
        Value colSel = args[0]->evaluate(context);
        size_t col = resolveColumn(m, colSel);
        size_t count = 0;
        for (size_t r = 1; r < m.data.size(); ++r) {
            double num;
            if (col <= m.data[r].size() && tryParseNumber(m.data[r][col - 1], num)) count++;
        }
        return static_cast<double>(count);
    }

    if (method == "FIND") {
        // The optional third argument makes the match case-insensitive.
        // address.kumu hand-rolls its own contact lookup purely because FIND
        // could only match exactly, so "bob smith" would not find "Bob Smith".
        if (args.size() != 2 && args.size() != 3) {
            throw std::runtime_error("FIND expects 2 or 3 arguments (column, value, ignoreCase)");
        }
        Value colSel = args[0]->evaluate(context);
        std::string target = toDisplayString(args[1]->evaluate(context));
        bool ignoreCase = args.size() == 3 && isTruthy(args[2]->evaluate(context));
        size_t col = resolveColumn(m, colSel);
        auto fold = [](std::string v) {
            std::transform(v.begin(), v.end(), v.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            return v;
        };
        if (ignoreCase) target = fold(target);
        for (size_t r = 1; r < m.data.size(); ++r) {
            if (col > m.data[r].size()) continue;
            const std::string& cell = m.data[r][col - 1];
            if ((ignoreCase ? fold(cell) : cell) == target) {
                return static_cast<double>(r); // already the 1-based data-row number
            }
        }
        return 0.0;
    }

    if (method == "SORT") {
        if (args.size() != 1) throw std::runtime_error("SORT expects 1 argument (column)");
        Value colSel = args[0]->evaluate(context);
        size_t col = resolveColumn(m, colSel);
        // Auto-detect: numeric sort if every data-row cell in this column
        // parses as a number, otherwise lexicographic (string) sort -- so
        // the caller never has to say which kind of comparison they want.
        bool allNumeric = true;
        for (size_t r = 1; r < m.data.size() && allNumeric; ++r) {
            double dummy;
            if (col > m.data[r].size() || !tryParseNumber(m.data[r][col - 1], dummy)) allNumeric = false;
        }
        std::vector<std::vector<std::string>> dataRows(m.data.begin() + 1, m.data.end());
        std::sort(dataRows.begin(), dataRows.end(),
                  [col, allNumeric](const std::vector<std::string>& a, const std::vector<std::string>& b) {
            std::string ca = (col - 1 < a.size()) ? a[col - 1] : std::string();
            std::string cb = (col - 1 < b.size()) ? b[col - 1] : std::string();
            if (allNumeric) {
                double da = 0.0, db = 0.0;
                tryParseNumber(ca, da);
                tryParseNumber(cb, db);
                return numberLess(da, db);
            }
            return ca < cb;
        });
        Matrix sorted;
        sorted.data.push_back(m.data[0]);
        for (auto& row : dataRows) sorted.data.push_back(std::move(row));
        return sorted;
    }

    if (method == "SAVE") {
        if (args.size() != 1) throw std::runtime_error("SAVE expects 1 argument (path)");
        Value pathVal = args[0]->evaluate(context);
        if (!is_string(pathVal)) throw std::runtime_error("SAVE expects a string path");
        std::ofstream file(as_string(pathVal));
        if (!file.is_open()) throw std::runtime_error("SAVE: could not open file '" + as_string(pathVal) + "'");
        for (const auto& row : m.data) {
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) file << ",";
                file << csvEscapeCell(row[i]);
            }
            file << "\n";
        }
        return 1.0;
    }

    static const std::vector<std::string> kMatrixMethods = {
        "GET", "SET", "ROWS", "COLS", "APPEND_ROW", "DELETE_ROW", "SUM", "AVERAGE",
        "MIN", "MAX", "COUNT", "FIND", "SORT", "SAVE"};
    throw std::runtime_error("Matrix has no method '" + method + "'" +
                              suggestName(method, kMatrixMethods));
}

// ---- Statements ----

void Assignment::execute(Context& context) {
    assignVariable(context, varName, expr->evaluate(context), isDeclaration);
}

void PrintStatement::execute(Context& context) {
    std::cout << toDisplayString(expr->evaluate(context)) << std::endl;
}

void InputStatement::execute(Context& context) {
    std::string line;
    if (!std::getline(std::cin, line)) {
        // True end-of-input (stdin closed/exhausted): stop cleanly with a
        // clear error rather than silently feeding an interactive loop an
        // endless stream of empty strings, which would just spin forever.
        throw std::runtime_error("INPUT: no more input available (end of input)");
    }
    double num;
    // INPUT has no VAR keyword, so it's treated like a bare assignment: it
    // updates an existing local/global binding of this name if one is
    // visible, otherwise it creates a new global.
    if (tryParseNumber(line, num)) {
        assignVariable(context, varName, num, false);
    } else {
        assignVariable(context, varName, line, false);
    }
}

void ExpressionStatement::execute(Context& context) {
    expr->evaluate(context); // evaluated only for its side effect
}

void IfStatement::execute(Context& context) {
    if (isTruthy(condition->evaluate(context))) {
        for (auto& stmt : thenBranch) runStatement(*stmt, context);
    } else {
        for (auto& stmt : elseBranch) runStatement(*stmt, context);
    }
}

void WhileStatement::execute(Context& context) {
    while (isTruthy(condition->evaluate(context))) {
        try {
            for (auto& stmt : body) runStatement(*stmt, context);
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            continue; // re-tests the condition, as it would normally
        }
    }
}

void ForStatement::execute(Context& context) {
    double startVal = requireNumber(start->evaluate(context), "FOR start value");
    double endVal = requireNumber(end->evaluate(context), "FOR end value");
    double stepVal = 1.0;
    if (step) {
        stepVal = requireNumber(step->evaluate(context), "FOR STEP value");
        if (stepVal == 0.0) {
            throw std::runtime_error("FOR STEP cannot be 0 -- the loop would never end");
        }
    }
    // A negative step counts down and so tests the bound the other way round.
    // Without STEP the behavior is byte-for-byte what it always was, including
    // `FOR i = 10 TO 1` running zero times.
    for (double i = startVal; stepVal > 0.0 ? i <= endVal : i >= endVal; i += stepVal) {
        // Treated like a declaration: local to the current call if inside
        // one, so a FOR loop's counter can't leak into or clobber a caller's
        // variable of the same name.
        assignVariable(context, iterVar, i, true);
        try {
            for (auto& stmt : body) runStatement(*stmt, context);
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            continue; // still advances by stepVal, as a skipped iteration should
        }
    }
}

void ForEachStatement::execute(Context& context) {
    Value collectionVal = collection->evaluate(context);

    // The items are snapshotted before the loop starts, so rebinding the
    // collection inside the body (`names = names.APPEND(...)`) cannot make
    // the loop it is running in grow or shrink underneath itself.
    std::vector<Value> items;
    if (is_list(collectionVal)) {
        items = as_list(collectionVal).items;
    } else if (is_matrix(collectionVal)) {
        // Each data row becomes a List of its cells; the header is skipped,
        // consistent with every other Matrix operation.
        Matrix m = as_matrix(collectionVal);
        for (size_t r = 1; r < m.data.size(); ++r) {
            std::vector<Value> row;
            row.reserve(m.data[r].size());
            for (const std::string& cell : m.data[r]) row.push_back(cell);
            items.push_back(make_list(std::move(row)));
        }
    } else {
        throw std::runtime_error("FOR EACH needs a List or a Matrix, got " + valueKind(collectionVal));
    }

    for (Value& item : items) {
        // Declared, like a FOR counter, so it stays local to the current call.
        assignVariable(context, iterVar, item, true);
        try {
            for (auto& stmt : body) runStatement(*stmt, context);
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            continue;
        }
    }
}

void TraceStatement::execute(Context& context) {
    context.traceEnabled = isTruthy(expr->evaluate(context));
}

void TryStatement::execute(Context& context) {
    try {
        for (auto& stmt : tryBody) runStatement(*stmt, context);
    } catch (const std::runtime_error& e) {
        // Bound like a VAR declaration: local to the current call if inside
        // one (so it doesn't leak into or collide with anything outside this
        // function), global at the top level.
        assignVariable(context, errVarName, std::string(e.what()), true);
        for (auto& stmt : catchBody) runStatement(*stmt, context);
    }
}

void FunctionDefinition::execute(Context& context) {
    // Registering moves `body` out of this node, so re-executing the same
    // definition (inside a loop, say, or after the hoisting pass already
    // handled it) must do nothing rather than move from an emptied body.
    if (registered) return;
    if (context.functions.find(name) != context.functions.end()) {
        // Previously the second definition was silently ignored, so a
        // program with a duplicated name quietly ran the wrong code.
        throw std::runtime_error("Function '" + name + "' is already defined");
    }
    context.functions[name] = std::make_pair(params, std::move(body));
    registered = true;
}

void BreakStatement::execute(Context&) {
    throw BreakSignal{};
}

void ContinueStatement::execute(Context&) {
    throw ContinueSignal{};
}

void ReturnStatement::execute(Context& context) {
    Value v = expr ? expr->evaluate(context) : Value(0.0);
    throw ReturnSignal{std::move(v)};
}

// ---- Interpreter ----

void Interpreter::execute(std::vector<std::unique_ptr<Statement>>& program, Context& ctx) {
    // Hoisting pass: register every top-level FUNCTION before running any of
    // the program, so a call no longer has to appear below the definition it
    // refers to. Nested definitions (inside an IF or a loop) are untouched --
    // they still register when control flow reaches them.
    for (auto& stmt : program) {
        if (auto* fn = dynamic_cast<FunctionDefinition*>(stmt.get())) {
            fn->execute(ctx);
        }
    }

    try {
        for (auto& stmt : program) {
            runStatement(*stmt, ctx);
        }
    } catch (ReturnSignal&) {
        throw std::runtime_error("RETURN used outside of a function");
    } catch (const BreakSignal&) {
        throw std::runtime_error("BREAK used outside of a loop");
    } catch (const ContinueSignal&) {
        throw std::runtime_error("CONTINUE used outside of a loop");
    }
}

} // namespace kumu
