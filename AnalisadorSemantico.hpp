#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "AnalisadorLexico.hpp"

// Analisador semântico simples para ConvCC-2025-2.
// Faz checagens básicas de escopo, tipos em expressões aritméticas,
// verificação de identificadores e uso de break em laços.
class AnalisadorSemantico {
public:
    explicit AnalisadorSemantico(const std::vector<AnalisadorLexico::TokenInfo>& tokens);

    bool analisar();
    std::unordered_map<std::string, std::string> getCollectedSymbols() const;
    using ScopeSymbols = std::unordered_map<std::string, std::string>;
    std::vector<ScopeSymbols> getScopesSymbols() const;
    struct ExprNode {
        std::string label;
        std::shared_ptr<ExprNode> left;
        std::shared_ptr<ExprNode> right;
    };

    const std::vector<std::shared_ptr<ExprNode>>& getExpressionTrees() const {
        return expressionTrees;
    }

private:
    struct Symbol {
        std::string type;                  // int, float, string, function
        bool isFunction = false;
        std::vector<std::string> params;   // tipos dos parâmetros, se função
        std::string returnType = "unknown";
    };

    struct ExpressionResult {
        std::string type;
        std::shared_ptr<ExprNode> node;
    };

    const std::vector<AnalisadorLexico::TokenInfo>& tokens;
    std::size_t pos = 0;
    int loopDepth = 0;
    bool errorFound = false;

    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    std::vector<std::shared_ptr<ExprNode>> expressionTrees;

    // parsing helpers
    const AnalisadorLexico::TokenInfo& peek() const;
    const AnalisadorLexico::TokenInfo& previous() const;
    bool isAtEnd() const;
    bool check(const std::string& type) const;
    bool match(const std::string& type);
    bool expect(const std::string& type, const std::string& message);
    void advance();

    // scope helpers
    void pushScope();
    void popScope();
    bool declareSymbol(const std::string& name, const Symbol& sym, const AnalisadorLexico::TokenInfo& where);
    const Symbol* resolveSymbol(const std::string& name) const;

    // error
    void reportError(const AnalisadorLexico::TokenInfo& tk, const std::string& msg);

    // grammar
    bool program();
    bool functionDef();
    bool paramList(std::vector<std::string>& params);
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

    // expressions
    ExpressionResult expression(bool capture = false);
    ExpressionResult numExpression();
    ExpressionResult term();
    ExpressionResult unary();
    ExpressionResult factor();
    ExpressionResult lvalue();
    ExpressionResult functionCall(const AnalisadorLexico::TokenInfo& idToken);

    // helpers for type checking
    ExpressionResult combineArithmetic(const ExpressionResult& lhs,
                                       const ExpressionResult& rhs,
                                       const std::string& op,
                                       const AnalisadorLexico::TokenInfo& tk);

    std::shared_ptr<ExprNode> makeNode(const std::string& label,
                                       std::shared_ptr<ExprNode> left = nullptr,
                                       std::shared_ptr<ExprNode> right = nullptr);
    void recordExpression(const ExpressionResult& res, bool capture);
};
