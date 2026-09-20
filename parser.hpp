#pragma once
#include "lexer.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>
#include <string>

namespace kumu {

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Statement>> parse();

private:
    std::vector<Token> tokens;
    size_t current;

    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseOr();
    std::unique_ptr<Expression> parseAnd();
    std::unique_ptr<Expression> parseTerm();
    std::unique_ptr<Expression> parseFactor();
    std::unique_ptr<Expression> parseUnary();
    std::unique_ptr<Expression> parseComparison();

    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseStatementBody();
    std::unique_ptr<Statement> parseAssignment();
    std::unique_ptr<Statement> parseIf();
    std::unique_ptr<Statement> parseWhile();
    std::unique_ptr<Statement> parseFor();
    std::unique_ptr<Statement> parseFunction();

    std::unique_ptr<Expression> parsePrimary();
    std::unique_ptr<Expression> parsePostfix();

    Token advance();
    Token peek();
    bool match(TokenType type);
    void consume(TokenType type, const std::string& message);
    std::vector<std::unique_ptr<Statement>> parseBlock();
};

}
