#pragma once
#include "core.hpp"
#include <string>
#include <vector>

namespace kumu {

enum class TokenType {
    Number, String, Identifier,
    Plus, Minus, Star, Slash,
    Equal,      // '='  -- assignment only
    EqualEqual, // '==' -- equality comparison
    NotEqual, Less, Greater, LessEqual, GreaterEqual,
    Exclamation, // Logical NOT
    And, Or, Not, Mod, // word-form logical operators and modulo
    LeftParen, RightParen,
    LeftBrace, RightBrace,     // blocks -- and nothing else
    LeftBracket, RightBracket, // list literals and indexing
    Comma, Dot,
    // Keywords
    Var, Print, Input, If, Then, ElseIf, Else, End,
    While, For, To, Step, Each, In,
    Break, Continue,
    Function, Return,
    ReadFile, WriteFile, ReadCsv,
    Trace, Try, Catch,
    EOF_
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column = 1; // 1-based, for pointing a caret at the exact spot
};

// True when `id` is one of Kumu's keywords written in the wrong case --
// "print", "Print", "iF". Fills `suggestion` with the correct spelling.
// Used by the parser to explain the mistake, since a miscased keyword is a
// perfectly valid identifier and so fails somewhere other than where it is.
bool miscasedKeyword(const std::string& id, std::string& suggestion);

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos;
    int line;
    int column;        // kept in step with pos by advance()
    int tokenLine;     // where the token currently being read started
    int tokenColumn;

    Token nextToken();
    void skipWhitespace();
    std::string readIdentifier();
    std::string readNumber();
    std::string readString();
    char peek();
    char advance();
};

}
