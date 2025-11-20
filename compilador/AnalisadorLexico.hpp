#pragma once
#include <vector>
#include <string_view>
#include <unordered_map>
#include "Simbolo.hpp"

class AnalisadorLexico {
public:
    AnalisadorLexico();

    bool analisar(std::string_view input);

    const std::vector<std::string>& getTokens() const {
        return tokens;
    }

    const std::unordered_map<std::string, Simbolo>& getSymbolTable() const {
        return symbolTable;
    }

    ~AnalisadorLexico();

private:
    std::vector<std::string> tokens {};
    std::unordered_map<std::string, Simbolo> symbolTable;

    std::string_view source {};
    std::size_t pos {0};
    std::size_t currentLine {1};
    std::size_t currentColumn {0};

    char peek() const;
    char advance();
    bool isAtEnd() const;

    // lógica de análise
    void skipWhitespace();
    bool scanToken();
    bool scanIdentifierOrKeyword(char firstChar);
    bool scanNumber(char firstChar);
    bool scanString();
    bool scanOperatorOrDelimiter(char firstChar);

    void addIdentifierOccurrence(const std::string& name,
                                 std::size_t line,
                                 std::size_t column);
};
