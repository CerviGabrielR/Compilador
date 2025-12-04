#include "AnalisadorGCI.hpp"
#include <iostream>

AnalisadorGCI::AnalisadorGCI(const std::vector<AnalisadorLexico::TokenInfo>& tokens)
    : tokens(tokens) {
    pushScope();
}

const AnalisadorLexico::TokenInfo& AnalisadorGCI::peek() const {
    static AnalisadorLexico::TokenInfo dummy{"$", "", 0, 0};
    if (tokens.empty()) return dummy;
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos];
}

const AnalisadorLexico::TokenInfo& AnalisadorGCI::previous() const {
    static AnalisadorLexico::TokenInfo dummy{"$", "", 0, 0};
    if (tokens.empty()) return dummy;
    if (pos == 0) return tokens.front();
    return tokens[pos - 1];
}

bool AnalisadorGCI::isAtEnd() const {
    return pos >= tokens.size();
}

bool AnalisadorGCI::check(const std::string& type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool AnalisadorGCI::match(const std::string& type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool AnalisadorGCI::expect(const std::string& type, const std::string& message) {
    if (match(type)) return true;
    reportError(peek(), message);
    return false;
}

void AnalisadorGCI::advance() {
    if (!isAtEnd()) pos++;
}

std::string AnalisadorGCI::newTemp() {
    return "t" + std::to_string(++tempCounter);
}

std::string AnalisadorGCI::newLabel() {
    return "L" + std::to_string(++labelCounter);
}

void AnalisadorGCI::emit(const std::string& instr) {
    codigo.push_back(instr);
}

void AnalisadorGCI::pushScope() {
    scopes.emplace_back();
}

void AnalisadorGCI::popScope() {
    if (!scopes.empty()) scopes.pop_back();
}

void AnalisadorGCI::declare(const std::string& name, const std::string& type) {
    if (scopes.empty()) pushScope();
    scopes.back()[name] = type;
}

std::string AnalisadorGCI::resolve(const std::string& name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto f = it->find(name);
        if (f != it->end()) return f->second;
    }
    return "unknown";
}

void AnalisadorGCI::reportError(const AnalisadorLexico::TokenInfo& tk, const std::string& msg) {
    if (errorFound) return;
    errorFound = true;
    std::cerr << "[Erro GCI] Linha " << tk.line << ", coluna " << tk.column << ": " << msg << "\n";
}

bool AnalisadorGCI::gerar() {
    bool ok = program();
    return ok && !errorFound;
}

bool AnalisadorGCI::program() {
    while (!isAtEnd() && !errorFound) {
        if (check("def")) {
            if (!functionDef()) return false;
        } else {
            if (!statement()) return false;
        }
    }
    return !errorFound;
}

bool AnalisadorGCI::functionDef() {
    expect("def", "Esperado 'def'");
    expect("id", "Esperado nome de função");
    auto nameTk = previous();
    emit("func " + nameTk.lexeme + ":");

    expect("(", "Esperado '('");
    pushScope();
    if (!check(")")) paramList();
    expect(")", "Esperado ')'");

    // corpo
    expect("{", "Esperado '{'");
    while (!check("}") && !isAtEnd()) {
        if (!statement()) break;
    }
    expect("}", "Esperado '}'");

    emit("endfunc " + nameTk.lexeme);
    popScope();
    return !errorFound;
}

bool AnalisadorGCI::paramList() {
    while (true) {
        if (!(check("int") || check("float") || check("string"))) {
            reportError(peek(), "Esperado tipo em parâmetro");
            return false;
        }
        auto type = peek().type;
        advance();
        expect("id", "Esperado nome do parâmetro");
        auto name = previous().lexeme;
        declare(name, type);
        if (match(",")) continue;
        break;
    }
    return true;
}

bool AnalisadorGCI::statement() {
    if (check("int") || check("float") || check("string")) return varDecl();
    if (check("id")) {
        auto idTk = peek();
        if (pos + 1 < tokens.size() && tokens[pos + 1].type == "(") {
            advance(); // id
            functionCall(idTk);
            expect(";", "Esperado ';' após chamada");
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

bool AnalisadorGCI::varDecl() {
    auto type = peek().type;
    advance();
    expect("id", "Esperado identificador");
    auto name = previous().lexeme;
    declare(name, type);

    // ignora array, apenas consome
    while (match("[")) {
        expression();
        expect("]", "Esperado ']'");
    }
    expect(";", "Esperado ';' após declaração");
    return !errorFound;
}

bool AnalisadorGCI::atribStat(bool expectSemicolon) {
    auto lhs = lvalue();
    expect("=", "Esperado '='");
    auto rhs = expression();
    emit(lhs.place + " = " + rhs.place);
    if (expectSemicolon) expect(";", "Esperado ';'");
    return !errorFound;
}

bool AnalisadorGCI::printStat() {
    advance(); // print
    auto e = expression();
    emit("print " + e.place);
    expect(";", "Esperado ';'");
    return !errorFound;
}

bool AnalisadorGCI::readStat() {
    advance(); // read
    auto lv = lvalue();
    emit("read " + lv.place);
    expect(";", "Esperado ';'");
    return !errorFound;
}

bool AnalisadorGCI::returnStat() {
    advance(); // return
    auto e = expression();
    emit("return " + e.place);
    expect(";", "Esperado ';'");
    return !errorFound;
}

bool AnalisadorGCI::ifStat() {
    advance(); // if
    expect("(", "Esperado '('");
    auto cond = expression();
    expect(")", "Esperado ')'");
    auto elseLabel = newLabel();
    auto endLabel = newLabel();
    emit("ifFalse " + cond.place + " goto " + elseLabel);
    statement();
    emit("goto " + endLabel);
    emit(elseLabel + ":");
    if (match("else")) {
        statement();
    }
    emit(endLabel + ":");
    return !errorFound;
}

bool AnalisadorGCI::forStat() {
    advance(); // for
    expect("(", "Esperado '('");
    atribStat(false);
    expect(";", "Esperado ';'");
    auto testLabel = newLabel();
    auto endLabel = newLabel();
    emit(testLabel + ":");
    auto cond = expression();
    emit("ifFalse " + cond.place + " goto " + endLabel);
    expect(";", "Esperado ';'");
    auto updateLabel = newLabel();
    auto bodyLabel = newLabel();

    // captura instruções da atualização para emitir depois do corpo
    std::size_t updateStart = codigo.size();
    atribStat(false);
    std::vector<std::string> updateInstr(codigo.begin() + updateStart, codigo.end());
    codigo.erase(codigo.begin() + updateStart, codigo.end());

    expect(")", "Esperado ')'");

    emit("goto " + bodyLabel);
    emit(updateLabel + ":");
    for (auto& instr : updateInstr) emit(instr);
    emit("goto " + testLabel);
    emit(bodyLabel + ":");
    loopEndLabels.push_back(endLabel);
    statement();
    loopEndLabels.pop_back();
    emit("goto " + updateLabel);
    emit(endLabel + ":");
    return !errorFound;
}

bool AnalisadorGCI::block() {
    expect("{", "Esperado '{'");
    pushScope();
    while (!check("}") && !isAtEnd()) {
        if (!statement()) break;
    }
    expect("}", "Esperado '}'");
    popScope();
    return !errorFound;
}

bool AnalisadorGCI::breakStat() {
    auto tk = peek();
    advance();
    if (loopEndLabels.empty()) {
        reportError(tk, "break fora de laço");
    } else {
        emit("goto " + loopEndLabels.back());
    }
    expect(";", "Esperado ';'");
    return !errorFound;
}

AnalisadorGCI::ExprResult AnalisadorGCI::expression() {
    auto left = numExpression();
    if (check("<") || check(">") || check("<=") || check(">=") || check("==") || check("!=")) {
        auto op = peek().type;
        advance();
        auto right = numExpression();
        auto t = newTemp();
        emit(t + " = " + left.place + " " + op + " " + right.place);
        return {t, "bool"};
    }
    return left;
}

AnalisadorGCI::ExprResult AnalisadorGCI::numExpression() {
    auto left = term();
    while (check("+") || check("-")) {
        auto op = peek().type;
        advance();
        auto right = term();
        auto t = newTemp();
        emit(t + " = " + left.place + " " + op + " " + right.place);
        left = {t, combineType(left.type, right.type)};
    }
    return left;
}

AnalisadorGCI::ExprResult AnalisadorGCI::term() {
    auto left = unary();
    while (check("*") || check("/") || check("%")) {
        auto op = peek().type;
        advance();
        auto right = unary();
        auto t = newTemp();
        emit(t + " = " + left.place + " " + op + " " + right.place);
        left = {t, combineType(left.type, right.type)};
    }
    return left;
}

AnalisadorGCI::ExprResult AnalisadorGCI::unary() {
    if (match("+") || match("-")) {
        auto op = previous().type;
        auto r = unary();
        auto t = newTemp();
        emit(t + " = " + op + r.place);
        return {t, r.type};
    }
    return factor();
}

AnalisadorGCI::ExprResult AnalisadorGCI::factor() {
    if (match("int_constant")) return {previous().lexeme, "int"};
    if (match("float_constant")) return {previous().lexeme, "float"};
    if (match("string_constant")) return {"\"" + previous().lexeme + "\"", "string"};
    if (match("null")) return {"null", "null"};

    if (check("id")) {
        auto idTk = peek();
        if (pos + 1 < tokens.size() && tokens[pos + 1].type == "(") {
            advance();
            return functionCall(idTk);
        }
        return lvalue();
    }

    if (match("(")) {
        auto r = numExpression();
        expect(")", "Esperado ')'");
        return r;
    }

    reportError(peek(), "Fator inválido");
    advance();
    return {"?", "unknown"};
}

AnalisadorGCI::ExprResult AnalisadorGCI::lvalue() {
    expect("id", "Esperado identificador");
    auto name = previous().lexeme;
    auto type = resolve(name);
    std::string place = name;
    while (match("[")) {
        auto idx = expression();
        expect("]", "Esperado ']'");
        auto t = newTemp();
        emit(t + " = " + place + "[" + idx.place + "]");
        place = t;
        type = "unknown";
    }
    return {place, type};
}

AnalisadorGCI::ExprResult AnalisadorGCI::functionCall(const AnalisadorLexico::TokenInfo& idToken) {
    expect("(", "Esperado '('");
    std::vector<ExprResult> args;
    if (!check(")")) {
        do {
            args.push_back(expression());
        } while (match(","));
    }
    expect(")", "Esperado ')'");
    for (auto& a : args) emit("param " + a.place);
    auto t = newTemp();
    emit(t + " = call " + idToken.lexeme + ", " + std::to_string(args.size()));
    return {t, "unknown"};
}

std::string AnalisadorGCI::combineType(const std::string& a, const std::string& b) {
    if (a == "float" || b == "float") return "float";
    if (a == "int" || b == "int") return "int";
    if (a == "string" && b == "string") return "string";
    return "unknown";
}
