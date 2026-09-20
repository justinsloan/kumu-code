#include "lexer.hpp"
#include <cctype>
#include <algorithm>
#include <unordered_map>

namespace kumu {

namespace {
// The one list of keywords. Anything not in here is an ordinary identifier.
const std::unordered_map<std::string, TokenType>& keywordTable() {
    static const std::unordered_map<std::string, TokenType> table = {
            {"VAR", TokenType::Var},           {"PRINT", TokenType::Print},
            {"INPUT", TokenType::Input},       {"IF", TokenType::If},
            {"THEN", TokenType::Then},         {"ELSEIF", TokenType::ElseIf},  // THEN: retired, see parser
            {"ELSE", TokenType::Else},         {"END", TokenType::End},
            {"WHILE", TokenType::While},       {"FOR", TokenType::For},
            {"TO", TokenType::To},             {"STEP", TokenType::Step},
            {"EACH", TokenType::Each},         {"IN", TokenType::In},
            {"BREAK", TokenType::Break},       {"CONTINUE", TokenType::Continue},
            {"AND", TokenType::And},           {"OR", TokenType::Or},
            {"NOT", TokenType::Not},           {"MOD", TokenType::Mod},
            {"FUNCTION", TokenType::Function}, {"RETURN", TokenType::Return},
            {"READ_FILE", TokenType::ReadFile},{"WRITE_FILE", TokenType::WriteFile},
            {"READ_CSV", TokenType::ReadCsv},  {"TRACE", TokenType::Trace},
            {"TRY", TokenType::Try},           {"CATCH", TokenType::Catch},
        };
    return table;
}
} // namespace

bool miscasedKeyword(const std::string& id, std::string& suggestion) {
    std::string upper = id;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    if (upper == id) return false; // already correct, or has no letters to fix
    if (keywordTable().find(upper) == keywordTable().end()) return false;
    suggestion = upper;
    return true;
}

Lexer::Lexer(const std::string& source)
    : source(source), pos(0), line(1), column(1), tokenLine(1), tokenColumn(1) {}

void Lexer::skipWhitespace() {
    while (pos < source.length()) {
        if (std::isspace(source[pos])) {
            advance();
        } else if (source[pos] == '#') {
            // Line comment: skip through end of line (but not the '\n'
            // itself, so the next pass through this loop counts it).
            while (pos < source.length() && source[pos] != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

char Lexer::peek() {
    return (pos < source.length()) ? source[pos] : '\0';
}

char Lexer::advance() {
    char c = source[pos++];
    // The one place position is tracked, so every reader stays in step.
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

std::string Lexer::readIdentifier() {
    std::string result;
    while (pos < source.length() && (std::isalnum(source[pos]) || source[pos] == '_')) {
        result += advance();
    }
    return result;
}

std::string Lexer::readNumber() {
    std::string result;
    bool seenDot = false;
    while (pos < source.length() && (std::isdigit(source[pos]) || source[pos] == '.')) {
        if (source[pos] == '.') {
            // Without this check "1.2.3" lexes as one token and std::stod
            // silently stops at the second dot, yielding 1.2 -- a wrong
            // answer with no warning, which is the worst thing a teaching
            // language can do.
            if (seenDot) {
                    throw LexError("Malformed number '" + result +
                               ".' -- a number can have only one decimal point", line, column);
            }
            // A '.' with no digit after it is not part of the number -- it is
            // a method call like 5.LEN(). Leaving it for the parser produces
            // "Cannot call method 'LEN' on a Number", which says what is
            // actually wrong.
            if (pos + 1 >= source.length() || !std::isdigit(static_cast<unsigned char>(source[pos + 1]))) {
                break;
            }
            seenDot = true;
        }
        result += advance();
    }
    return result;
}

std::string Lexer::readString() {
    int startLine = line;
    int startColumn = column;
    advance(); // Skip opening "
    std::string result;
    while (pos < source.length() && source[pos] != '"') {
        result += advance();
    }
    // Running off the end used to be silent: the loop just stopped and the
    // stray advance() below walked past the end of the buffer.
    if (pos >= source.length()) {
        throw LexError("Unterminated string -- it is missing a closing '\"'",
                       startLine, startColumn);
    }
    advance(); // Skip closing "
    return result;
}

Token Lexer::nextToken() {
    skipWhitespace();
    tokenLine = line;
    tokenColumn = column;
    if (pos >= source.length()) return {TokenType::EOF_, "", tokenLine, tokenColumn};

    char c = peek();
    if (std::isdigit(c)) {
        std::string num = readNumber();
        return {TokenType::Number, num, tokenLine, tokenColumn};
    }

    if (c == '"') {
        std::string str = readString();
        return {TokenType::String, str, tokenLine, tokenColumn};
    }

    if (std::isalpha(c) || c == '_') {
        std::string id = readIdentifier();
        auto found = keywordTable().find(id);
        TokenType type = (found != keywordTable().end()) ? found->second : TokenType::Identifier;
        return {type, id, tokenLine, tokenColumn};
    }

    advance();
    switch (c) {
        case '+': return {TokenType::Plus, "+", tokenLine, tokenColumn};
        case '-': return {TokenType::Minus, "-", tokenLine, tokenColumn};
        case '*': return {TokenType::Star, "*", tokenLine, tokenColumn};
        case '/': return {TokenType::Slash, "/", tokenLine, tokenColumn};
        case '=':
            if (peek() == '=') {
                advance();
                return {TokenType::EqualEqual, "==", tokenLine, tokenColumn};
            }
            return {TokenType::Equal, "=", tokenLine, tokenColumn};
        case '!':
            if (peek() == '=') {
                advance();
                return {TokenType::NotEqual, "!=", tokenLine, tokenColumn};
            }
            return {TokenType::Exclamation, "!", tokenLine, tokenColumn};
        case '<':
            if (peek() == '=') {
                advance();
                return {TokenType::LessEqual, "<=", tokenLine, tokenColumn};
            }
            return {TokenType::Less, "<", tokenLine, tokenColumn};
        case '>':
            if (peek() == '=') {
                advance();
                return {TokenType::GreaterEqual, ">=", tokenLine, tokenColumn};
            }
            return {TokenType::Greater, ">", tokenLine, tokenColumn};
        case '(': return {TokenType::LeftParen, "(", tokenLine, tokenColumn};
        case '{': return {TokenType::LeftBrace, "{", tokenLine, tokenColumn};
        case '}': return {TokenType::RightBrace, "}", line};
        case '[': return {TokenType::LeftBracket, "[", tokenLine, tokenColumn};
        case ']': return {TokenType::RightBracket, "]", tokenLine, tokenColumn};
        case ')': return {TokenType::RightParen, ")", tokenLine, tokenColumn};
        case ',': return {TokenType::Comma, ",", line};
        case '.': return {TokenType::Dot, ".", tokenLine, tokenColumn};
    }

    // Anything else is a character Kumu has no meaning for. This used to
    // return an EOF token, which made tokenize() stop and silently discard
    // the entire rest of the file -- the program just ended early, with no
    // error and a zero exit status.
    throw LexError(std::string("Unexpected character '") + c + "'", tokenLine, tokenColumn);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = nextToken();
        tokens.push_back(t);
        if (t.type == TokenType::EOF_) break;
    }
    return tokens;
}

}
