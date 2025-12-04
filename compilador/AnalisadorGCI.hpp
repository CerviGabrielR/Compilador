#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "AnalisadorLexico.hpp"

// Gerador de Código Intermediário simples (3 endereços) para ConvCC-2025-2.
// Percorre os tokens (já aprovados pelo léxico/sintático/semântico) e gera
// instruções de três endereços para atribuições, expressões, if/else, for,
// print/read e chamadas de função.
class AnalisadorGCI {
public:
    explicit AnalisadorGCI(const std::vector<AnalisadorLexico::TokenInfo>& tokens);

    bool gerar();
    const std::vector<std::string>& getCodigo() const { return codigo; }

private:
    struct ExprResult {
        std::string place;
        std::string type;
    };

    const std::vector<AnalisadorLexico::TokenInfo>& tokens;
    std::size_t pos = 0;
    bool errorFound = false;
    int tempCounter = 0;
    int labelCounter = 0;

    std::vector<std::unordered_map<std::string, std::string>> scopes;
    std::vector<std::string> loopEndLabels;
    std::vector<std::string> codigo;

    // utilidades
    const AnalisadorLexico::TokenInfo& peek() const;
    const AnalisadorLexico::TokenInfo& previous() const;
    bool isAtEnd() const;
    bool check(const std::string& type) const;
    bool match(const std::string& type);
    bool expect(const std::string& type, const std::string& message);
    void advance();
    std::string newTemp();
    std::string newLabel();
    void emit(const std::string& instr);
    void pushScope();
    void popScope();
    void declare(const std::string& name, const std::string& type);
    std::string resolve(const std::string& name) const;
    void reportError(const AnalisadorLexico::TokenInfo& tk, const std::string& msg);

    // gramática
    bool program();
    bool functionDef();
    bool paramList();
    bool statement();
    bool varDecl();
    bool atribStat(bool expectSemicolon = true);
    bool printStat();
    bool readStat();
    bool returnStat();
    bool ifStat();
    bool forStat();
    bool block();
    bool breakStat();

    // expressões
    ExprResult expression();
    ExprResult numExpression();
    ExprResult term();
    ExprResult unary();
    ExprResult factor();
    ExprResult lvalue();
    ExprResult functionCall(const AnalisadorLexico::TokenInfo& idToken);
    std::string combineType(const std::string& a, const std::string& b);
};
