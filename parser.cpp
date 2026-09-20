#include "parser.hpp"
#include <stdexcept>
#include <memory>
#include <iostream>

namespace kumu {

namespace {
// Stamps an expression with the position of the token that best identifies it
// -- an operator for a binary expression, a name for a reference. That token
// is what a caret should point at when the expression fails.
template <typename T>
std::unique_ptr<T> at(std::unique_ptr<T> node, const Token& t) {
    node->line = t.line;
    node->column = t.column;
    return node;
}

// Describes a token the way a reader would say it, so a message can name the
// thing that is actually in the way.
std::string describeToken(const Token& t) {
    switch (t.type) {
        case TokenType::EOF_:         return "end of file";
        case TokenType::String:       return "the text \"" + t.lexeme + "\"";
        case TokenType::Number:       return "the number " + t.lexeme;
        case TokenType::RightBrace:   return "'}' (there is no open block for it to close)";
        case TokenType::RightParen:   return "')'";
        case TokenType::RightBracket: return "']'";
        case TokenType::Else:         return "'ELSE' (it must follow an IF block)";
        case TokenType::ElseIf:       return "'ELSEIF' (it must follow an IF block)";
        case TokenType::Catch:        return "'CATCH' (it must follow a TRY block)";
        case TokenType::Identifier:   return "the name '" + t.lexeme + "'";
        default:                      return "'" + t.lexeme + "'";
    }
}
} // namespace

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseOr();
}

// OR binds loosest, then AND, then the comparisons -- so
// `a == 1 AND b == 2 OR c == 3` groups as `((a==1 AND b==2) OR c==3)`,
// which is what every other language does and what a reader expects.
std::unique_ptr<Expression> Parser::parseOr() {
    auto expr = parseAnd();
    while (peek().type == TokenType::Or) {
        Token op = peek();
        advance();
        auto right = parseAnd();
        expr = at(std::make_unique<LogicalExpression>(std::move(expr), "OR", std::move(right)), op);
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseAnd() {
    auto expr = parseComparison();
    while (peek().type == TokenType::And) {
        Token op = peek();
        advance();
        auto right = parseComparison();
        expr = at(std::make_unique<LogicalExpression>(std::move(expr), "AND", std::move(right)), op);
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseComparison() {
    auto expr = parseTerm();
    while (peek().type == TokenType::Less || peek().type == TokenType::Greater ||
           peek().type == TokenType::LessEqual || peek().type == TokenType::GreaterEqual ||
           peek().type == TokenType::EqualEqual || peek().type == TokenType::Equal ||
           peek().type == TokenType::NotEqual) {

        std::string op;
        if (peek().type == TokenType::Less) op = "<";
        else if (peek().type == TokenType::Greater) op = ">";
        else if (peek().type == TokenType::LessEqual) op = "<=";
        else if (peek().type == TokenType::GreaterEqual) op = ">=";
        else if (peek().type == TokenType::NotEqual) op = "!=";
        else {
            // Both '==' and a lone '=' land here. A lone '=' in a comparison
            // is almost always the classic typo, but programs written before
            // '==' was required depend on it working, so it stays valid and
            // gets a warning rather than an error.
            op = "==";
            if (peek().type == TokenType::Equal) {
                std::cerr << "Warning (line " << peek().line
                          << "): '=' used as a comparison -- did you mean '=='?" << std::endl;
            }
        }

        Token opToken = peek();
        advance();
        auto right = parseTerm();
        expr = at(std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right)), opToken);
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseTerm() {
    auto expr = parseFactor();
    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        std::string op;
        if (peek().type == TokenType::Plus) op = "+";
        else if (peek().type == TokenType::Minus) op = "-";
        Token opToken = peek();
        advance();
        auto right = parseFactor();
        expr = at(std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right)), opToken);
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseFactor() {
    auto expr = parseUnary();
    while (peek().type == TokenType::Star || peek().type == TokenType::Slash ||
           peek().type == TokenType::Mod) {
        std::string op;
        if (peek().type == TokenType::Star) op = "*";
        else if (peek().type == TokenType::Slash) op = "/";
        else if (peek().type == TokenType::Mod) op = "MOD";
        Token opToken = peek();
        advance();
        auto right = parseUnary();
        expr = at(std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right)), opToken);
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseUnary() {
    if (peek().type == TokenType::Minus) {
        Token op = peek();
        advance();
        auto operand = parseUnary();
        return at(std::make_unique<UnaryExpression>("-", std::move(operand)), op);
    }
    // NOT is a spelled-out synonym for '!'; both build the same node, so
    // existing programs using '!' are untouched.
    if (peek().type == TokenType::Exclamation || peek().type == TokenType::Not) {
        Token op = peek();
        advance();
        auto operand = parseUnary();
        return at(std::make_unique<UnaryExpression>("!", std::move(operand)), op);
    }
    return parsePostfix();
}

std::unique_ptr<Expression> Parser::parsePostfix() {
    auto expr = parsePrimary();
    // '.' and '[' chain freely in either order: names.SORT()[1] is fine.
    while (peek().type == TokenType::Dot || peek().type == TokenType::LeftBracket) {
        if (peek().type == TokenType::LeftBracket) {
            Token bracket = peek();
            advance(); // consume '['
            auto index = parseExpression();
            consume(TokenType::RightBracket, "Expected ']' after index");
            expr = at(std::make_unique<IndexExpression>(std::move(expr), std::move(index)), bracket);
            continue;
        }
        advance(); // consume '.'
        Token methodToken = peek();
        std::string methodName = peek().lexeme;
        advance(); // consume the method name
        consume(TokenType::LeftParen, "Expected '(' after method name");
        std::vector<std::unique_ptr<Expression>> args;
        while (peek().type != TokenType::RightParen) {
            args.push_back(parseExpression());
            if (peek().type == TokenType::Comma) advance();
        }
        consume(TokenType::RightParen, "Expected ')' after method arguments");
        expr = at(std::make_unique<MethodCallExpression>(std::move(expr), methodName, std::move(args)),
                  methodToken);
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    Token t = peek();

    if (t.type == TokenType::Number) {
        advance();
        return at(std::make_unique<Literal>(Value(std::stod(t.lexeme))), t);
    }

    if (t.type == TokenType::String) {
        advance();
        return at(std::make_unique<Literal>(Value(t.lexeme)), t);
    }

    if (t.type == TokenType::LeftBracket) {
        advance(); // consume '['
        std::vector<std::unique_ptr<Expression>> elements;
        while (peek().type != TokenType::RightBracket) {
            elements.push_back(parseExpression());
            if (peek().type == TokenType::Comma) {
                advance();
            } else if (peek().type != TokenType::RightBracket) {
                throw ParseError("Expected ',' or ']' in this list", peek().line, peek().column);
            }
        }
        consume(TokenType::RightBracket, "Expected ']' to close list literal");
        return at(std::make_unique<ListLiteral>(std::move(elements)), t);
    }

    if (t.type == TokenType::LeftParen) {
        advance(); // consume '('

        // A '(' immediately followed by another '(' is a Matrix literal:
        // ( (cell, cell, ...), (cell, cell, ...), ... )
        if (peek().type == TokenType::LeftParen) {
            Matrix m;
            while (peek().type == TokenType::LeftParen) {
                advance(); // consume the row's '('
                std::vector<std::string> row;
                while (peek().type != TokenType::RightParen) {
                    Token cell = peek();
                    if (cell.type != TokenType::String && cell.type != TokenType::Number) {
                        throw ParseError("A matrix row may only hold text or numbers, but this is '" +
                                         cell.lexeme + "'", cell.line, cell.column);
                    }
                    row.push_back(cell.lexeme);
                    advance();
                    if (peek().type == TokenType::Comma) advance();
                }
                consume(TokenType::RightParen, "Expected ')' to close matrix row");
                m.data.push_back(std::move(row));
                if (peek().type == TokenType::Comma) advance();
            }
            consume(TokenType::RightParen, "Expected ')' to close matrix literal");
            return at(std::make_unique<Literal>(Value(m)), t);
        }

        // Otherwise it's just a parenthesized sub-expression.
        auto expr = parseExpression();
        consume(TokenType::RightParen, "Expected ')' after expression");
        return expr;
    }

    // Plain identifiers and the file-I/O keywords all share call syntax:
    // NAME(arg, arg, ...). The file-I/O keywords always require it.
    if (t.type == TokenType::Identifier || t.type == TokenType::ReadFile ||
        t.type == TokenType::WriteFile || t.type == TokenType::ReadCsv) {
        advance();

        if (peek().type == TokenType::LeftParen) {
            // Unambiguous now that '(' never opens a block: an identifier
            // followed by '(' is a call, full stop. This used to be a
            // speculative parse that rewound when it "didn't look like" a
            // call -- guesswork that silently misread `WHILE ready ( x )`.
            advance(); // consume '('
            std::vector<std::unique_ptr<Expression>> args;
            while (peek().type != TokenType::RightParen) {
                args.push_back(parseExpression());
                if (peek().type == TokenType::Comma) {
                    advance();
                } else if (peek().type != TokenType::RightParen) {
                    throw ParseError("Expected ',' or ')' in this argument list", peek().line, peek().column);
                }
            }
            consume(TokenType::RightParen, "Expected ')' after arguments");
            return at(std::make_unique<CallExpression>(t.lexeme, std::move(args)), t);
        }

        if (t.type != TokenType::Identifier) {
            throw ParseError(t.lexeme + " must be called, e.g. " + t.lexeme + "(...)", t.line, t.column);
        }
        return at(std::make_unique<VariableReference>(t.lexeme), t);
    }

    throw ParseError(t.type == TokenType::EOF_
                         ? std::string("Unexpected end of file -- a value was expected here")
                         : "Unexpected " + describeToken(t) + " -- a value was expected here",
                     t.line, t.column);
}

std::unique_ptr<Statement> Parser::parseAssignment() {
    std::string name = peek().lexeme;
    advance();
    consume(TokenType::Equal, "Expected '=' after variable name");
    auto expr = parseExpression();
    return std::make_unique<Assignment>(name, std::move(expr), false);
}

std::unique_ptr<Statement> Parser::parseStatement() {
    int startLine = peek().line;
    int startColumn = peek().column;
    std::unique_ptr<Statement> result = parseStatementBody();
    result->line = startLine;
    result->column = startColumn;
    return result;
}

std::unique_ptr<Statement> Parser::parseStatementBody() {
    if (peek().type == TokenType::Var) {
        advance();
        std::string name = peek().lexeme;
        advance();
        consume(TokenType::Equal, "Expected '=' after variable name");
        auto expr = parseExpression();
        return std::make_unique<Assignment>(name, std::move(expr), true);
    } else if (peek().type == TokenType::Print) {
        advance();
        auto expr = parseExpression();
        return std::make_unique<PrintStatement>(std::move(expr));
    } else if (peek().type == TokenType::Input) {
        advance();
        std::string name = peek().lexeme;
        advance();
        return std::make_unique<InputStatement>(name);
    } else if (peek().type == TokenType::If) {
        // IF cond THEN block (ELSEIF cond THEN block)* (ELSE block)? END
        //
        // ELSEIF is pure syntax sugar: it's desugared here into the same
        // nested "ELSE ( IF ... )" structure this parser already builds by
        // hand, so IfStatement and the interpreter never need to know
        // ELSEIF exists -- only the parser does, and only in this one spot.
        struct Branch {
            int line;
            std::unique_ptr<Expression> cond;
            std::vector<std::unique_ptr<Statement>> block;
        };
        std::vector<Branch> branches;

        int firstLine = peek().line;
        advance();
        auto cond = parseExpression();
        auto block = parseBlock();
        branches.push_back({firstLine, std::move(cond), std::move(block)});

        while (peek().type == TokenType::ElseIf) {
            int branchLine = peek().line;
            advance();
            auto elseifCond = parseExpression();
            auto elseifBlock = parseBlock();
            branches.push_back({branchLine, std::move(elseifCond), std::move(elseifBlock)});
        }

        std::vector<std::unique_ptr<Statement>> elseChain;
        if (peek().type == TokenType::Else) {
            advance();
            elseChain = parseBlock();
        }

        // Fold branches into nested IfStatements from the last one back to
        // the first, so branches[0] ends up as the outermost statement.
        for (auto it = branches.rbegin(); it != branches.rend(); ++it) {
            auto ifStmt = std::make_unique<IfStatement>(std::move(it->cond), std::move(it->block), std::move(elseChain));
            ifStmt->line = it->line;
            elseChain.clear();
            elseChain.push_back(std::move(ifStmt));
        }
        return std::move(elseChain[0]);
    } else if (peek().type == TokenType::While) {
        advance();
        auto cond = parseExpression();
        auto body = parseBlock();
        return std::make_unique<WhileStatement>(std::move(cond), std::move(body));
    } else if (peek().type == TokenType::For) {
        advance();

        // FOR EACH item IN collection
        if (peek().type == TokenType::Each) {
            advance();
            std::string eachVar = peek().lexeme;
            advance();
            consume(TokenType::In, "Expected 'IN' after the FOR EACH variable");
            auto collection = parseExpression();
            auto eachBody = parseBlock();
            return std::make_unique<ForEachStatement>(eachVar, std::move(collection), std::move(eachBody));
        }

        // FOR i = start TO end [STEP amount]
        std::string varName = peek().lexeme;
        advance();
        consume(TokenType::Equal, "Expected '=' in FOR loop");
        auto start = parseExpression();
        consume(TokenType::To, "Expected 'TO' in FOR loop");
        auto end = parseExpression();
        std::unique_ptr<Expression> step; // null means 1
        if (peek().type == TokenType::Step) {
            advance();
            step = parseExpression();
        }
        auto body = parseBlock();
        return std::make_unique<ForStatement>(varName, std::move(start), std::move(end),
                                              std::move(step), std::move(body));
    } else if (peek().type == TokenType::Break) {
        advance();
        return std::make_unique<BreakStatement>();
    } else if (peek().type == TokenType::Continue) {
        advance();
        return std::make_unique<ContinueStatement>();
    } else if (peek().type == TokenType::Function) {
        advance();
        std::string name = peek().lexeme;
        advance();
        // Required, not optional -- `FUNCTION greet` used to be legal, which
        // made `FUNCTION greet (` ambiguous between a parameter list and a
        // body and silently turned the body into a list of parameters.
        consume(TokenType::LeftParen, "Expected '(' to start the parameter list -- "
                                       "a function with no parameters is written `FUNCTION name()`");
        std::vector<std::string> params;
        while (peek().type != TokenType::RightParen) {
            params.push_back(peek().lexeme);
            advance();
            if (peek().type == TokenType::Comma) advance();
        }
        consume(TokenType::RightParen, "Expected ')' to close the parameter list");
        auto body = parseBlock();
        return std::make_unique<FunctionDefinition>(name, params, std::move(body));
    } else if (peek().type == TokenType::Return) {
        advance();
        // A RETURN with nothing after it returns 0. Detected by looking for
        // whatever could close the enclosing block; ReturnStatement::execute
        // already handled a null expression, it just had no way to get one.
        std::unique_ptr<Expression> expr;
        TokenType next = peek().type;
        if (next != TokenType::RightBrace && next != TokenType::EOF_) {
            expr = parseExpression();
        }
        return std::make_unique<ReturnStatement>(std::move(expr));
    } else if (peek().type == TokenType::Trace) {
        advance();
        auto expr = parseExpression();
        return std::make_unique<TraceStatement>(std::move(expr));
    } else if (peek().type == TokenType::Try) {
        advance();
        auto tryBody = parseBlock();
        consume(TokenType::Catch, "Expected 'CATCH' after TRY block");
        std::string errVarName = peek().lexeme;
        advance();
        auto catchBody = parseBlock();
        return std::make_unique<TryStatement>(std::move(tryBody), errVarName, std::move(catchBody));
    } else if (peek().type == TokenType::Identifier) {
        // A keyword written in the wrong case is a perfectly good identifier,
        // so it parses happily and the complaint lands on whatever came after
        // it: `print "hi"` used to report an unexpected `hi`, naming the one
        // part of the line that was fine. Catch it here, where the name is
        // still in hand -- but only when it isn't being used as an ordinary
        // name, so `VAR print = 5` and `print = 5` keep working.
        {
            TokenType after = (current + 1 < tokens.size()) ? tokens[current + 1].type
                                                            : TokenType::EOF_;
            bool usedAsName = after == TokenType::Equal || after == TokenType::LeftParen ||
                              after == TokenType::LeftBracket || after == TokenType::Dot;
            std::string suggestion;
            if (!usedAsName && miscasedKeyword(peek().lexeme, suggestion)) {
                if (suggestion == "THEN" || suggestion == "END") {
                    throw ParseError("'" + peek().lexeme + "' is not a keyword -- and '" + suggestion +
                                     "' is no longer part of Kumu either; blocks are written `{ ... }`",
                                     peek().line, peek().column);
                }
                throw ParseError("'" + peek().lexeme +
                                 "' is not a keyword -- Kumu keywords are written in capitals. "
                                 "Did you mean '" + suggestion + "'?",
                                 peek().line, peek().column);
            }
        }

        // "name = expr" is an assignment; anything else starting with an
        // identifier (e.g. a bare function call) is an expression statement.
        if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::Equal) {
            return parseAssignment();
        }
        // `name[...] = value` has to be caught before parsing, not after:
        // parseComparison accepts a lone '=' as equality, so the line would
        // otherwise turn into a comparison whose result is silently thrown
        // away -- the assignment looking like it worked while doing nothing.
        if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::LeftBracket) {
            // Skip past every chained index group, so g[1][2] = x is caught
            // just as g[1] = x is.
            size_t scan = current + 1;
            while (scan < tokens.size() && tokens[scan].type == TokenType::LeftBracket) {
                int depth = 0;
                while (scan < tokens.size() && tokens[scan].type != TokenType::EOF_) {
                    if (tokens[scan].type == TokenType::LeftBracket) {
                        depth++;
                    } else if (tokens[scan].type == TokenType::RightBracket && --depth == 0) {
                        scan++;
                        break;
                    }
                    scan++;
                }
            }
            if (scan < tokens.size() && tokens[scan].type == TokenType::Equal) {
                throw ParseError("Cannot assign to a position -- lists are immutable; use `" +
                                 peek().lexeme + " = " + peek().lexeme + ".SET(position, value)`",
                                 tokens[scan].line, tokens[scan].column);
            }
        }
        auto expr = parseExpression();
        return std::make_unique<ExpressionStatement>(std::move(expr));
    } else if (peek().type == TokenType::ReadFile || peek().type == TokenType::WriteFile ||
               peek().type == TokenType::ReadCsv) {
        // These are always calls, used here only for their side effect,
        // e.g. WRITE_FILE("out.txt", "hi") as its own statement.
        auto expr = parseExpression();
        return std::make_unique<ExpressionStatement>(std::move(expr));
    }

    if (peek().type == TokenType::End || peek().type == TokenType::Then) {
        // Both were block syntax in an earlier version of Kumu. They stay
        // reserved purely so an old program gets told what to do instead of
        // a confusing complaint about an unexpected name.
        throw ParseError("'" + peek().lexeme + "' is no longer part of Kumu -- "
                         "blocks are written `{ ... }`", peek().line, peek().column);
    }
    throw ParseError(peek().type == TokenType::EOF_
                         ? std::string("Unexpected end of file -- a statement is unfinished")
                         : "Unexpected " + describeToken(peek()) + " -- a statement cannot start here",
                     peek().line, peek().column);
}

std::vector<std::unique_ptr<Statement>> Parser::parseBlock() {
    // Every block in the language is `{ ... }`. There is no second form and
    // no optional part, which is the whole point: a learner never has to ask
    // which style a construct wants, and '{' cannot be confused with the '('
    // that groups an expression or opens an argument list.
    if (peek().type == TokenType::End || peek().type == TokenType::Then) {
        throw ParseError("'" + peek().lexeme + "' is no longer part of Kumu -- "
                         "blocks are written `{ ... }`", peek().line, peek().column);
    }
    consume(TokenType::LeftBrace, "Expected '{' to open a block");

    std::vector<std::unique_ptr<Statement>> block;
    while (peek().type != TokenType::RightBrace && peek().type != TokenType::EOF_) {
        block.push_back(parseStatement());
    }
    consume(TokenType::RightBrace, "Expected '}' to close a block");
    return block;
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::advance() {
    return tokens[current++];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (peek().type == type) {
        advance();
    } else {
        throw ParseError(message, peek().line, peek().column);
    }
}

std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> program;
    while (peek().type != TokenType::EOF_) {
        program.push_back(parseStatement());
    }
    return program;
}

}
