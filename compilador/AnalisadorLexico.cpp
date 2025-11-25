#include "AnalisadorLexico.hpp"

#include <cctype>
#include <unordered_set>
#include <iostream>

namespace {

    bool isIdentifierStart(char c) {
        unsigned char uc = static_cast<unsigned char>(c);
        return std::isalpha(uc) || c == '_';
    }

    bool isIdentifierChar(char c) {
        unsigned char uc = static_cast<unsigned char>(c);
        return std::isalnum(uc) || c == '_';
    }

    const std::unordered_set<std::string> KEYWORDS = {
            "def", "int", "float", "string",
            "print", "read", "return",
            "if", "else", "for",
            "new", "null", "break"
    };

}

AnalisadorLexico::AnalisadorLexico()
        : tokens(),
          symbolTable(),
          source(),
          pos(0),
          currentLine(1),
          currentColumn(0) {}

AnalisadorLexico::~AnalisadorLexico() = default;


char AnalisadorLexico::peek() const {
    if (isAtEnd()) {
        return '\0';
    }
    return source[pos];
}

char AnalisadorLexico::advance() {
    char c = source[pos++];
    if (c == '\n') {
        ++currentLine;
        currentColumn = 0;
    } else {
        ++currentColumn;
    }
    return c;
}

bool AnalisadorLexico::isAtEnd() const {
    return pos >= source.size();
}

void AnalisadorLexico::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

void AnalisadorLexico::addIdentifierOccurrence(const std::string& name,
                                               std::size_t line,
                                               std::size_t column) {
    auto it = symbolTable.find(name);
    if (it == symbolTable.end()) {
        Simbolo s;
        s.type = "ident";
        s.address = symbolTable.size();
        s.occurrences.push_back({line, column});
        symbolTable.emplace(name, std::move(s));
    } else {
        it->second.occurrences.push_back({line, column});
    }
}


bool AnalisadorLexico::scanIdentifierOrKeyword(char firstChar) {
    std::string lexeme;
    lexeme.push_back(firstChar);

    std::size_t startLine = currentLine;
    std::size_t startColumn = currentColumn;

    while (!isAtEnd()) {
        char c = peek();
        if (isIdentifierChar(c)) {
            advance();
            lexeme.push_back(c);
        } else {
            break;
        }
    }

    if (KEYWORDS.find(lexeme) != KEYWORDS.end()) {
        tokens.push_back(lexeme);  // keyword mantém o lexema
    } else {
        tokens.push_back("id");    // <-- ALTERADO
        addIdentifierOccurrence(lexeme, startLine, startColumn);
    }

    return true;
}

bool AnalisadorLexico::scanNumber(char firstChar) {
    std::string lexeme;
    lexeme.push_back(firstChar);

    std::size_t startLine = currentLine;
    std::size_t startColumn = currentColumn;

    while (!isAtEnd()) {
        char c = peek();
        if (std::isdigit(static_cast<unsigned char>(c))) {
            advance();
            lexeme.push_back(c);
        } else {
            break;
        }
    }

    bool isFloat = false;
    if (!isAtEnd() && peek() == '.') {
        std::size_t savedPos = pos;
        std::size_t savedLine = currentLine;
        std::size_t savedColumn = currentColumn;

        advance();
        if (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            isFloat = true;
            lexeme.push_back('.');
            while (!isAtEnd()) {
                char c = peek();
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    advance();
                    lexeme.push_back(c);
                } else {
                    break;
                }
            }
        } else {
            pos = savedPos;
            currentLine = savedLine;
            currentColumn = savedColumn;
        }
    }

    tokens.push_back("num");  // <-- ALTERADO

    return true;
}

bool AnalisadorLexico::scanString() {
    std::string lexeme;

    while (!isAtEnd()) {
        char c = advance();
        if (c == '"') {
            tokens.push_back("string"); // <-- ALTERADO
            return true;
        }
        if (c == '\n') {
            std::cerr << "Erro léxico: string não fechada na linha "
                      << currentLine << "\n";
            return false;
        }
        lexeme.push_back(c);
    }

    std::cerr << "Erro léxico: fim de arquivo dentro de string\n";
    return false;
}

bool AnalisadorLexico::scanOperatorOrDelimiter(char c) {
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            tokens.push_back(std::string(1, c));
            return true;

        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ';':
        case ',':
            tokens.push_back(std::string(1, c));
            return true;

        case '<':
            if (!isAtEnd() && peek() == '=') {
                advance();
                tokens.push_back("<=");
            } else {
                tokens.push_back("<");
            }
            return true;

        case '>':
            if (!isAtEnd() && peek() == '=') {
                advance();
                tokens.push_back(">=");
            } else {
                tokens.push_back(">");
            }
            return true;

        case '=':
            if (!isAtEnd() && peek() == '=') {
                advance();
                tokens.push_back("==");
            } else {
                tokens.push_back("=");
            }
            return true;

        case '!':
            if (!isAtEnd() && peek() == '=') {
                advance();
                tokens.push_back("!=");
                return true;
            }
            std::cerr << "Erro léxico: '!' isolado na linha "
                      << currentLine << ", coluna " << currentColumn << "\n";
            return false;

        default:
            std::cerr << "Erro léxico: caractere inválido '"
                      << c << "' na linha " << currentLine
                      << ", coluna " << currentColumn << "\n";
            return false;
    }
}

bool AnalisadorLexico::scanToken() {
    char c = advance();

    if (isIdentifierStart(c)) {
        return scanIdentifierOrKeyword(c);
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        return scanNumber(c);
    }

    if (c == '"') {
        return scanString();
    }

    return scanOperatorOrDelimiter(c);
}


bool AnalisadorLexico::analisar(std::string_view input) {
    tokens.clear();
    symbolTable.clear();

    source = input;
    pos = 0;
    currentLine = 1;
    currentColumn = 0;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) {
            break;
        }

        if (!scanToken()) {
            return false;
        }
    }

    return true;
}
