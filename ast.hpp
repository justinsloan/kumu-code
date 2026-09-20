#pragma once
#include "core.hpp"
#include <string>
#include <vector>
#include <memory>

namespace kumu {

struct Expression {
    // Where this expression sits in the source, so a runtime error can point a
    // caret at the operator or name that actually failed rather than at the
    // start of the statement containing it.
    int line = 0;
    int column = 0;
    virtual ~Expression() = default;
    virtual Value evaluate(Context& context) = 0;
};

struct Statement {
    int line = 0;   // source line of this statement's first token, set by the parser
    int column = 0; // and its column
    virtual ~Statement() = default;
    virtual void execute(Context& context) = 0;
    // A short label for TRACE output, e.g. "PRINT" or "ASSIGN x". Purely
    // cosmetic -- never affects behavior.
    virtual std::string describe() const = 0;
};

// Expressions
struct Literal : public Expression {
    Value value;
    Literal(Value v) : value(v) {}
    Value evaluate(Context&) override { return value; }
};

struct VariableReference : public Expression {
    std::string name;
    VariableReference(std::string n) : name(n) {}
    Value evaluate(Context& context) override;
};

struct BinaryExpression : public Expression {
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
    BinaryExpression(std::unique_ptr<Expression> l, std::string o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    Value evaluate(Context& context) override;
};

// AND / OR. Deliberately NOT a BinaryExpression: that node evaluates both
// operands before it looks at the operator, which would make these eager.
// Short-circuiting is why `IF count > 0 AND total / count > 5` is safe -- the
// division never runs when count is 0.
struct LogicalExpression : public Expression {
    std::unique_ptr<Expression> left;
    std::string op; // "AND" or "OR"
    std::unique_ptr<Expression> right;
    LogicalExpression(std::unique_ptr<Expression> l, std::string o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
    Value evaluate(Context& context) override;
};

struct UnaryExpression : public Expression {
    std::string op;
    std::unique_ptr<Expression> expr;
    UnaryExpression(std::string o, std::unique_ptr<Expression> e) : op(o), expr(std::move(e)) {}
    Value evaluate(Context& context) override;
};

struct CallExpression : public Expression {
    std::string name;
    std::vector<std::unique_ptr<Expression>> args;
    CallExpression(std::string n, std::vector<std::unique_ptr<Expression>> a)
        : name(n), args(std::move(a)) {}
    Value evaluate(Context& context) override;
};

// [a, b, c] -- unlike a Matrix literal, whose cells must be literal strings
// or numbers baked in at parse time, list elements are arbitrary expressions
// evaluated fresh each time the literal is reached.
struct ListLiteral : public Expression {
    std::vector<std::unique_ptr<Expression>> elements;
    explicit ListLiteral(std::vector<std::unique_ptr<Expression>> e) : elements(std::move(e)) {}
    Value evaluate(Context& context) override;
};

// receiver[index] -- 1-based, matching row numbers, SUBSTRING and INDEX_OF.
struct IndexExpression : public Expression {
    std::unique_ptr<Expression> receiver;
    std::unique_ptr<Expression> index;
    IndexExpression(std::unique_ptr<Expression> r, std::unique_ptr<Expression> i)
        : receiver(std::move(r)), index(std::move(i)) {}
    Value evaluate(Context& context) override;
};

// receiver.METHOD(args, ...) -- currently only Matrix values have methods;
// evaluating this on anything else is a runtime error. Chains naturally
// (book.SET(...).SUM(...)) since the parser wraps each `.` in one of these
// around whatever expression came before it.
struct MethodCallExpression : public Expression {
    std::unique_ptr<Expression> receiver;
    std::string method;
    std::vector<std::unique_ptr<Expression>> args;
    MethodCallExpression(std::unique_ptr<Expression> r, std::string m, std::vector<std::unique_ptr<Expression>> a)
        : receiver(std::move(r)), method(std::move(m)), args(std::move(a)) {}
    Value evaluate(Context& context) override;
};

// Statements
struct Assignment : public Statement {
    std::string varName;
    std::unique_ptr<Expression> expr;
    // true for `VAR x = ...` (declares/shadows a binding in the current
    // scope), false for bare `x = ...` (updates an existing binding,
    // reaching out to an enclosing scope, or creates a new global).
    bool isDeclaration;
    Assignment(std::string name, std::unique_ptr<Expression> e, bool declaration)
        : varName(name), expr(std::move(e)), isDeclaration(declaration) {}
    void execute(Context& context) override;
    std::string describe() const override { return (isDeclaration ? "VAR " : "ASSIGN ") + varName; }
};

struct PrintStatement : public Statement {
    std::unique_ptr<Expression> expr;
    PrintStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "PRINT"; }
};

struct InputStatement : public Statement {
    std::string varName;
    InputStatement(std::string name) : varName(std::move(name)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "INPUT " + varName; }
};

struct IfStatement : public Statement {
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBranch;
    std::vector<std::unique_ptr<Statement>> elseBranch;
    IfStatement(std::unique_ptr<Expression> cond, std::vector<std::unique_ptr<Statement>> then, std::vector<std::unique_ptr<Statement>> else_branch)
        : condition(std::move(cond)), thenBranch(std::move(then)), elseBranch(std::move(else_branch)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "IF"; }
};

struct WhileStatement : public Statement {
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;
    WhileStatement(std::unique_ptr<Expression> cond, std::vector<std::unique_ptr<Statement>> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "WHILE"; }
};

struct ForStatement : public Statement {
    std::string iterVar;
    std::unique_ptr<Expression> start;
    std::unique_ptr<Expression> end;
    // Optional STEP amount; null means 1. A negative step counts down, which
    // is the only way to count down -- `FOR i = 10 TO 1` still runs zero
    // times, exactly as it always has.
    std::unique_ptr<Expression> step;
    std::vector<std::unique_ptr<Statement>> body;
    ForStatement(std::string var, std::unique_ptr<Expression> s, std::unique_ptr<Expression> e,
                 std::unique_ptr<Expression> st, std::vector<std::unique_ptr<Statement>> b)
        : iterVar(std::move(var)), start(std::move(s)), end(std::move(e)),
          step(std::move(st)), body(std::move(b)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "FOR " + iterVar; }
};

// FOR EACH item IN collection. Works over a List, and over a Matrix (yielding
// each data row as a List), so the two collection types connect rather than
// sitting in separate worlds.
struct ForEachStatement : public Statement {
    std::string iterVar;
    std::unique_ptr<Expression> collection;
    std::vector<std::unique_ptr<Statement>> body;
    ForEachStatement(std::string var, std::unique_ptr<Expression> c, std::vector<std::unique_ptr<Statement>> b)
        : iterVar(std::move(var)), collection(std::move(c)), body(std::move(b)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "FOR EACH " + iterVar; }
};

// BREAK leaves the innermost loop; CONTINUE skips to its next iteration.
// Both are implemented by throwing a signal caught by the loop, the same
// mechanism RETURN already uses to unwind out of a function body.
struct BreakStatement : public Statement {
    void execute(Context& context) override;
    std::string describe() const override { return "BREAK"; }
};

struct ContinueStatement : public Statement {
    void execute(Context& context) override;
    std::string describe() const override { return "CONTINUE"; }
};

struct FunctionDefinition : public Statement {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<Statement>> body;
    // Registering moves `body` out of this node, so it must happen exactly
    // once. Top-level definitions are registered by a hoisting pass before
    // the program runs; this flag is what lets the statement then be a no-op
    // when control flow reaches it, while a genuine duplicate name -- a
    // different node, still unregistered -- is still reported as an error.
    bool registered = false;
    FunctionDefinition(std::string n, std::vector<std::string> p, std::vector<std::unique_ptr<Statement>> b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "FUNCTION " + name; }
};

struct ReturnStatement : public Statement {
    std::unique_ptr<Expression> expr;
    ReturnStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "RETURN"; }
};

// A bare expression used as a statement, e.g. a call made only for its
// side effect: WRITE_FILE("out.txt", "hi") on its own line.
struct ExpressionStatement : public Statement {
    std::unique_ptr<Expression> expr;
    ExpressionStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "EXPRESSION"; }
};

// Toggles TRACE mode: while on, each statement prints itself (with its
// source line) just before it runs. TRACE 1 turns it on, TRACE 0 off --
// any expression works, evaluated for truthiness like an IF condition.
struct TraceStatement : public Statement {
    std::unique_ptr<Expression> expr;
    TraceStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "TRACE"; }
};

// TRY ( ... ) CATCH errVar ( ... ) END: runs the try body, and if any
// statement in it throws a runtime error, binds the error message (as a
// String) to errVar and runs the catch body instead. A RETURN inside the
// try body still exits the enclosing function normally, unaffected by
// the catch.
struct TryStatement : public Statement {
    std::vector<std::unique_ptr<Statement>> tryBody;
    std::string errVarName;
    std::vector<std::unique_ptr<Statement>> catchBody;
    TryStatement(std::vector<std::unique_ptr<Statement>> t, std::string e, std::vector<std::unique_ptr<Statement>> c)
        : tryBody(std::move(t)), errVarName(std::move(e)), catchBody(std::move(c)) {}
    void execute(Context& context) override;
    std::string describe() const override { return "TRY"; }
};

}
