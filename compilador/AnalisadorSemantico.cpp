// Integrantes: Antônio Torres; Gustavo Bodi; Lucas Brand; Ewaldo Uhlmann; Gabriel Cervi
#include "AnalisadorSemantico.hpp"
#include <iostream>

AnalisadorSemantico::AnalisadorSemantico(const std::vector<AnalisadorLexico::TokenInfo>& tokens)
    : tokens(tokens) {
    pushScope(); // escopo global
}

const AnalisadorLexico::TokenInfo& AnalisadorSemantico::peek() const {
    static AnalisadorLexico::TokenInfo dummy{"$", "", 0, 0};
    if (tokens.empty()) return dummy;
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos];
}

const AnalisadorLexico::TokenInfo& AnalisadorSemantico::previous() const {
    static AnalisadorLexico::TokenInfo dummy{"$", "", 0, 0};
    if (tokens.empty()) return dummy;
    if (pos == 0) return tokens.front();
    return tokens[pos - 1];
}

bool AnalisadorSemantico::isAtEnd() const {
    return pos >= tokens.size();
}

bool AnalisadorSemantico::check(const std::string& type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool AnalisadorSemantico::match(const std::string& type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool AnalisadorSemantico::expect(const std::string& type, const std::string& message) {
    if (match(type)) return true;
    reportError(peek(), message);
    return false;
}

void AnalisadorSemantico::advance() {
    if (!isAtEnd()) pos++;
}

void AnalisadorSemantico::pushScope() {
    scopes.emplace_back();
}

void AnalisadorSemantico::popScope() {
    if (!scopes.empty()) scopes.pop_back();
}

bool AnalisadorSemantico::declareSymbol(const std::string& name,
                                        const Symbol& sym,
                                        const AnalisadorLexico::TokenInfo& where) {
    if (scopes.empty()) pushScope();
    auto& current = scopes.back();
    if (current.count(name)) {
        reportError(where, "Identificador '" + name + "' já declarado neste escopo");
        return false;
    }
    current[name] = sym;
    return true;
}

const AnalisadorSemantico::Symbol* AnalisadorSemantico::resolveSymbol(const std::string& name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

void AnalisadorSemantico::reportError(const AnalisadorLexico::TokenInfo& tk, const std::string& msg) {
    if (errorFound) return;
    errorFound = true;
    std::cerr << "[Erro semântico] Linha " << tk.line << ", coluna " << tk.column << ": " << msg << "\n";
}

std::shared_ptr<AnalisadorSemantico::ExprNode> AnalisadorSemantico::makeNode(
        const std::string& label,
        std::shared_ptr<ExprNode> left,
        std::shared_ptr<ExprNode> right) {
    auto n = std::make_shared<ExprNode>();
    n->label = label;
    n->left = std::move(left);
    n->right = std::move(right);
    return n;
}

void AnalisadorSemantico::recordExpression(const ExpressionResult& res, bool capture) {
    if (capture && res.node) {
        expressionTrees.push_back(res.node);
    }
}

bool AnalisadorSemantico::analisar() {
    expressionTrees.clear();
    bool ok = program();
    return ok && !errorFound;
}

std::unordered_map<std::string, std::string> AnalisadorSemantico::getCollectedSymbols() const {
    std::unordered_map<std::string, std::string> out;
    for (const auto& scope : scopes) {
        for (const auto& [name, sym] : scope) {
            out[name] = sym.type;
        }
    }
    return out;
}

std::vector<AnalisadorSemantico::ScopeSymbols> AnalisadorSemantico::getScopesSymbols() const {
    std::vector<ScopeSymbols> res;
    res.reserve(scopes.size());
    for (const auto& scope : scopes) {
        ScopeSymbols m;
        for (const auto& [name, sym] : scope) {
            m[name] = sym.type;
        }
        res.push_back(std::move(m));
    }
    return res;
}

bool AnalisadorSemantico::program() {
    while (!isAtEnd() && !errorFound) {
        if (check("def")) {
            if (!functionDef()) return false;
        } else {
            if (!statement()) return false;
        }
    }
    return !errorFound;
}

bool AnalisadorSemantico::functionDef() {
    auto start = peek();
    advance(); // def

    if (!expect("id", "Esperado nome de função após 'def'")) return false;
    auto nameTk = previous();

    Symbol fSym;
    fSym.type = "function";
    fSym.isFunction = true;
    // por enquanto não inferimos tipo de retorno

    if (!expect("(", "Esperado '(' após nome da função")) return false;

    if (!check(")")) {
        if (!paramList(fSym.params)) return false;
    }
    if (!expect(")", "Esperado ')' após parâmetros")) return false;

    // registra função no escopo global
    if (!declareSymbol(nameTk.lexeme, fSym, nameTk)) return false;

    // novo escopo para corpo da função
    pushScope();
    // declara parâmetros no escopo da função
    for (std::size_t i = 0; i + 1 < fSym.params.size(); i += 2) {
        Symbol param;
        param.type = fSym.params[i];
        declareSymbol(fSym.params[i + 1], param, nameTk);
    }

    if (!expect("{", "Esperado '{' após cabeçalho da função")) return false;
    while (!check("}") && !isAtEnd()) {
        if (!statement()) break;
    }
    if (!expect("}", "Esperado '}' para fechar função")) return false;

    popScope();
    return !errorFound;
}

bool AnalisadorSemantico::paramList(std::vector<std::string>& params) {
    while (true) {
        if (!(check("int") || check("float") || check("string"))) {
            reportError(peek(), "Esperado tipo de parâmetro (int|float|string)");
            return false;
        }
        auto typeTk = peek();
        advance();
        if (!expect("id", "Esperado identificador de parâmetro")) return false;
        auto nameTk = previous();
        params.push_back(typeTk.type);
        params.push_back(nameTk.lexeme);

        if (match(",")) continue;
        break;
    }
    return true;
}

bool AnalisadorSemantico::statement() {
    if (check("int") || check("float") || check("string")) return varDecl();
    if (check("id")) {
        // lookahead para função ou atribuição
        auto idTk = peek();
        if (pos + 1 < tokens.size() && tokens[pos + 1].type == "(") {
            // chamada de função como statement
            advance(); // id
            auto res = functionCall(idTk);
            recordExpression(res, true);
            expect(";", "Esperado ';' após chamada de função");
            return !errorFound;
        }
        return atribStat();
    }
    if (check("print")) return printStat();
    if (check("read")) return readStat();
    if (check("return")) return returnStat();
    if (check("if")) return ifStat();
    if (check("for")) return forStat();
    if (check("{")) return block();
    if (check("break")) return breakStat();
    if (check(";")) { advance(); return true; }

    reportError(peek(), "Comando inesperado");
    return false;
}

bool AnalisadorSemantico::varDecl() {
    auto typeTk = peek();
    advance(); // tipo
    if (!expect("id", "Esperado identificador na declaração")) return false;
    auto nameTk = previous();

    Symbol sym;
    sym.type = typeTk.type;
    if (!declareSymbol(nameTk.lexeme, sym, nameTk)) return false;

    // ignora dimensões de array apenas consumindo tokens
    while (match("[")) {
        numExpression();
        expect("]", "Esperado ']' após dimensão de array");
    }

    if (!expect(";", "Esperado ';' após declaração")) return false;
    return !errorFound;
}

bool AnalisadorSemantico::atribStat(bool expectSemicolon) {
    auto lhs = lvalue();
    if (lhs.type.empty()) lhs.type = "unknown";
    if (!expect("=", "Esperado '=' em atribuição")) return false;
    auto expr = expression(true);

    if (lhs.type != "unknown" && expr.type != "unknown" && expr.type != "null") {
        if (lhs.type == "float" && expr.type == "int") {
            // ok: promoção
        } else if (lhs.type != expr.type) {
            reportError(previous(), "Tipos incompatíveis na atribuição: " + lhs.type + " = " + expr.type);
            return false;
        }
    }

    if (expectSemicolon) {
        if (!expect(";", "Esperado ';' ao final da atribuição")) return false;
    }
    return !errorFound;
}

bool AnalisadorSemantico::printStat() {
    advance(); // print
    expression(true);
    expect(";", "Esperado ';' após print");
    return !errorFound;
}

bool AnalisadorSemantico::readStat() {
    advance(); // read
    lvalue();
    expect(";", "Esperado ';' após read");
    return !errorFound;
}

bool AnalisadorSemantico::returnStat() {
    advance(); // return
    expression(true);
    expect(";", "Esperado ';' após return");
    return !errorFound;
}

bool AnalisadorSemantico::ifStat() {
    advance(); // if
    if (!expect("(", "Esperado '(' após if")) return false;
    auto cond = expression(true);
    if (cond.type == "string") {
        reportError(previous(), "Condição de if não pode ser string");
    }
    if (!expect(")", "Esperado ')' após condição")) return false;
    statement();
    if (match("else")) {
        statement();
    }
    return !errorFound;
}

bool AnalisadorSemantico::forStat() {
    advance(); // for
    if (!expect("(", "Esperado '(' após for")) return false;
    atribStat(false);
    expect(";", "Esperado ';' após inicialização do for");
    auto cond = expression(true);
    if (cond.type == "string") {
        reportError(previous(), "Condição de for não pode ser string");
    }
    expect(";", "Esperado ';' após expressão de condição do for");
    atribStat(false);
    if (!expect(")", "Esperado ')' após cabeçalho do for")) return false;

    loopDepth++;
    statement();
    loopDepth--;
    return !errorFound;
}

bool AnalisadorSemantico::block() {
    expect("{", "Esperado '{'");
    pushScope();
    while (!check("}") && !isAtEnd()) {
        if (!statement()) break;
    }
    expect("}", "Esperado '}' ao fim do bloco");
    popScope();
    return !errorFound;
}

bool AnalisadorSemantico::breakStat() {
    auto tk = peek();
    advance(); // break
    if (loopDepth == 0) {
        reportError(tk, "break fora de laço");
    }
    expect(";", "Esperado ';' após break");
    return !errorFound;
}

AnalisadorSemantico::ExpressionResult AnalisadorSemantico::expression(bool capture) {
    auto left = numExpression();
    if (check("<") || check(">") || check("<=") || check(">=") || check("==") || check("!=")) {
        auto opTk = peek();
        advance();
        auto right = numExpression();
        if (left.type != "unknown" && right.type != "unknown" && left.type != right.type) {
            if (!(left.type == "float" && right.type == "int") && !(left.type == "int" && right.type == "float")) {
                reportError(opTk, "Comparação entre tipos incompatíveis: " + left.type + " e " + right.type);
            }
        }
        ExpressionResult res{"bool", makeNode(opTk.type, left.node, right.node)};
        recordExpression(res, capture);
        return res;
    }
    recordExpression(left, capture);
    return left;
}

AnalisadorSemantico::ExpressionResult AnalisadorSemantico::numExpression() {
    auto left = term();
    while (check("+") || check("-")) {
        auto opTk = peek();
        advance();
        auto right = term();
        left = combineArithmetic(left, right, opTk.type, opTk);
    }
    return left;
}

AnalisadorSemantico::ExpressionResult AnalisadorSemantico::term() {
    auto left = unary();
    while (check("*") || check("/") || check("%")) {
        auto opTk = peek();
        advance();
        auto right = unary();
        left = combineArithmetic(left, right, opTk.type, opTk);
    }
    return left;
}

AnalisadorSemantico::ExpressionResult AnalisadorSemantico::unary() {
    if (match("+") || match("-")) {
        auto opTk = previous();
        auto t = unary();
        if (t.type != "int" && t.type != "float" && t.type != "unknown") {
            reportError(opTk, "Operador unário só é permitido para int/float");
        }
        return {t.type, makeNode(opTk.type, t.node)};
    }
    return factor();
}

AnalisadorSemantico::ExpressionResult AnalisadorSemantico::factor() {
    if (match("int_constant")) return { "int", makeNode(previous().lexeme) };
    if (match("float_constant")) return { "float", makeNode(previous().lexeme) };
    if (match("string_constant")) return { "string", makeNode(previous().lexeme) };
    if (match("null")) return { "null", makeNode("null") };

    if (check("id")) {
        auto idTk = peek();
        // lookahead para função
        if (pos + 1 < tokens.size() && tokens[pos + 1].type == "(") {
            advance();
            return functionCall(idTk);
        }
        return lvalue();
    }

    if (match("(")) {
        auto t = numExpression();
        expect(")", "Esperado ')' após expressão");
        return t;
    }

    reportError(peek(), "Fator inválido");
    advance();
    return {"unknown", nullptr};
}

AnalisadorSemantico::ExpressionResult AnalisadorSemantico::lvalue() {
    if (!expect("id", "Esperado identificador")) return {"unknown", nullptr};
    auto idTk = previous();
    auto sym = resolveSymbol(idTk.lexeme);
    if (!sym) {
        reportError(idTk, "Identificador '" + idTk.lexeme + "' não declarado");
        // ainda consome possíveis [ ]
    }
    std::shared_ptr<ExprNode> node = makeNode(idTk.lexeme);
    while (match("[")) {
        auto idx = numExpression();
        expect("]", "Esperado ']' em acesso de array");
        node = makeNode("[]", node, idx.node);
    }
    return {sym ? sym->type : "unknown", node};
}

AnalisadorSemantico::ExpressionResult AnalisadorSemantico::functionCall(const AnalisadorLexico::TokenInfo& idToken) {
    const Symbol* sym = resolveSymbol(idToken.lexeme);
    if (!sym || !sym->isFunction) {
        reportError(idToken, "Função '" + idToken.lexeme + "' não declarada");
    }

    expect("(", "Esperado '(' em chamada de função");
    std::vector<ExpressionResult> args;
    if (!check(")")) {
        do {
            args.push_back(expression());
        } while (match(","));
    }
    expect(")", "Esperado ')' ao fim dos argumentos");

    if (sym && sym->isFunction) {
        std::size_t expected = sym->params.size() / 2;
        if (args.size() != expected) {
            reportError(idToken, "Quantidade de argumentos incompatível em '" + idToken.lexeme + "'");
        } else {
            for (std::size_t i = 0; i < args.size(); ++i) {
                auto expectedType = sym->params[i * 2];
                if (expectedType != "unknown" && args[i].type != "unknown" && args[i].type != expectedType) {
                    if (!(expectedType == "float" && args[i].type == "int")) {
                        reportError(idToken, "Argumento " + std::to_string(i + 1) + " da função '" +
                                               idToken.lexeme + "' espera " + expectedType +
                                               " mas recebeu " + args[i].type);
                    }
                }
            }
        }
    }

    std::shared_ptr<ExprNode> argChain = nullptr;
    if (!args.empty()) {
        argChain = args.front().node;
        for (std::size_t i = 1; i < args.size(); ++i) {
            argChain = makeNode(",", argChain, args[i].node);
        }
    }
    auto callNode = makeNode("call " + idToken.lexeme, argChain, nullptr);
    return {sym && sym->isFunction ? sym->returnType : "unknown", callNode};
}

AnalisadorSemantico::ExpressionResult AnalisadorSemantico::combineArithmetic(const ExpressionResult& lhs,
                                                   const ExpressionResult& rhs,
                                                   const std::string& op,
                                                   const AnalisadorLexico::TokenInfo& tk) {
    auto node = makeNode(op, lhs.node, rhs.node);

    if (lhs.type == "unknown" || rhs.type == "unknown") return { "unknown", node };
    if (lhs.type == "null" || rhs.type == "null") {
        reportError(tk, "Operação aritmética com 'null' não é permitida");
        return {"unknown", node};
    }

    if (op == "%" && (lhs.type != "int" || rhs.type != "int")) {
        reportError(tk, "Operador % requer operandos int");
        return {"unknown", node};
    }

    if (lhs.type == "float" || rhs.type == "float") {
        if ((lhs.type == "int" || lhs.type == "float") && (rhs.type == "int" || rhs.type == "float")) {
            return {"float", node};
        }
    }

    if (lhs.type == rhs.type) {
        // string/string permitido somente em +
        if (lhs.type == "string" && op != "+") {
            reportError(tk, "Operação com strings suportada apenas com '+'");
            return {"unknown", node};
        }
        return {lhs.type, node};
    }

    reportError(tk, "Operação entre tipos incompatíveis: " + lhs.type + " " + op + " " + rhs.type);
    return {"unknown", node};
}
