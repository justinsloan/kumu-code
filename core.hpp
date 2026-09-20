#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <string>

namespace kumu {

// Error kinds. All three derive from std::runtime_error so the ~60 existing
// throw sites keep working unchanged, and so TryStatement -- which catches
// std::runtime_error -- keeps catching runtime failures. The distinction
// exists so main() can say "Syntax Error" for problems found before the
// program ever ran, instead of blaming everything on the runtime.
// A problem in the text of the program, carrying where it is so the reporter
// can show the offending line with a caret under it. The message itself no
// longer spells out "at line N" -- the position is data, not prose, so it can
// be rendered once and consistently.
struct SourceError : std::runtime_error {
    int line = 0;    // 0 means "position unknown"
    int column = 0;
    SourceError(const std::string& message, int l = 0, int c = 0)
        : std::runtime_error(message), line(l), column(c) {}
};
struct LexError : SourceError {
    using SourceError::SourceError;
};
struct ParseError : SourceError {
    using SourceError::SourceError;
};

// A runtime error that has already been tagged with the line it happened on,
// and which carries the chain of calls that led there. The call chain is kept
// separate from the message on purpose: a TRY/CATCH binds only the message, so
// a program handling an error sees clean text, while an error that escapes to
// the top prints the whole trail for whoever has to debug it.
struct TracedError : std::runtime_error {
    std::string callStack;
    int line = 0;
    int column = 0;
    TracedError(const std::string& message, std::string stack = "", int l = 0, int c = 0)
        : std::runtime_error(message), callStack(std::move(stack)), line(l), column(c) {}
};

// Forward declaration of Statement
class Statement;

// Value can be a Number (double), a String, a Matrix (a 2D table of strings
// with a header row), or a List (an ordered sequence of any Values).
struct Matrix {
    std::vector<std::vector<std::string>> data;
};

// A List holds Values, so it can nest and so numbers stay numbers rather than
// being flattened to text the way Matrix cells are. That makes Value
// recursive, which a variant cannot express directly -- hence the indirection
// through shared_ptr, which is allowed to name an incomplete type.
//
// The sharing is never observable: every List method returns a NEW list
// rather than mutating in place (the same rule Matrix already follows), so
// two Values can point at the same List only for as long as neither changes.
// The payoff is that copying a Value copies a pointer instead of the whole
// sequence -- strictly cheaper than Matrix, which deep-copies on every
// method call.
struct List;

using Value = std::variant<double, std::string, Matrix, std::shared_ptr<List>>;

struct List {
    std::vector<Value> items;
    // How deeply lists are nested at this node: 1 for a flat list, 2 for a
    // list of flat lists, and so on. Cached rather than recomputed because
    // it has to be checked on every construction (see make_list).
    int depth = 1;
};

// Context holds the state of the execution.
struct Context {
    std::map<std::string, Value> variables; // the global scope
    // One frame per function call currently in progress, holding that call's
    // parameters and VAR-declared locals. Only the top (the innermost,
    // currently-executing call) is ever consulted -- a callee never sees a
    // caller's frame, matching how a real call stack works. Empty at the
    // top level, where VAR and bare assignment both just touch `variables`.
    std::vector<std::map<std::string, Value>> locals;
    std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::unique_ptr<Statement>>>> functions;
    bool traceEnabled = false; // toggled by TRACE; shared, like everything else in Context
    // Depth of function calls currently in progress. Kumu runs on the C++
    // stack, so unbounded recursion would exhaust it and take the whole
    // process down with a segfault -- no message, no line number, nothing a
    // learner could act on. This counter turns that into an ordinary,
    // catchable Kumu error before the real stack runs out.
    int callDepth = 0;
    // One entry per call currently in progress: the function's name and the
    // line its call appeared on. Enough to reconstruct how execution got to
    // wherever it is now.
    struct CallSite {
        std::string function;
        int line;
    };
    std::vector<CallSite> callStack;
    // The line of the statement currently executing, so a call can record
    // where it was made from.
    int currentLine = 0;
    // Where to point if something throws. Reset to the statement's own
    // position as each statement begins, then narrowed by whichever
    // expression is about to fail -- so the caret lands on the operator or
    // name at fault, not merely on the line.
    int errorLine = 0;
    int errorColumn = 0;
};

// Helper functions to access the variant
inline bool is_number(const Value& v) { return std::holds_alternative<double>(v); }
inline bool is_string(const Value& v) { return std::holds_alternative<std::string>(v); }
inline bool is_matrix(const Value& v) { return std::holds_alternative<Matrix>(v); }
inline bool is_list(const Value& v) { return std::holds_alternative<std::shared_ptr<List>>(v); }

inline double as_number(const Value& v) { return std::get<double>(v); }
inline std::string as_string(const Value& v) { return std::get<std::string>(v); }
inline Matrix as_matrix(const Value& v) { return std::get<Matrix>(v); }
// Returned by reference: unlike as_matrix, reading a List never copies it.
inline const List& as_list(const Value& v) { return *std::get<std::shared_ptr<List>>(v); }

// Nesting limit. Lists are linked through shared_ptr, so anything that walks
// a nested list walks it recursively -- printing it, comparing it, and,
// unavoidably, destroying it. A list nested hundreds of thousands deep
// (`x = [x]` in a loop) therefore overflowed the C++ stack and killed the
// process, and did so even in a program that never printed the thing, because
// the destructor chain runs at scope exit regardless.
//
// Bounding the depth once, at construction, makes all three safe at a stroke;
// checking only at the point of printing would leave the destructor crash.
// 100 is far past any legitimate use and far short of any stack limit.
constexpr int kMaxListDepth = 100;

// The only way a List is built. Every list-producing operation goes through
// here, which is what keeps "methods never mutate" true by construction --
// and is what gives the depth check a single place to live.
inline Value make_list(std::vector<Value> items) {
    int depth = 1;
    for (const Value& item : items) {
        if (is_list(item)) {
            depth = std::max(depth, as_list(item).depth + 1);
        }
    }
    if (depth > kMaxListDepth) {
        throw std::runtime_error("Lists cannot be nested more than " +
                                  std::to_string(kMaxListDepth) + " deep");
    }
    auto l = std::make_shared<List>();
    l->items = std::move(items);
    l->depth = depth;
    return l;
}

} // kumu
