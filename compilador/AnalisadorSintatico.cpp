#include "AnalisadorSintatico.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

// ========================
// CARREGAR GRAMÁTICA
// ========================

void AnalisadorSintatico::carregarGramatica(const std::string& caminho) {
    std::ifstream arq(caminho);
    std::string linha;

    while (std::getline(arq, linha)) {
        if (linha.empty()) continue;

        std::stringstream ss(linha);
        std::string lhs, seta;
        ss >> lhs >> seta; // LHS ->

        std::vector<std::string> rhs;
        std::string s;
        while (ss >> s) {
            rhs.push_back(s);
        }

        gramatica.push_back({lhs, rhs});
    }
}

bool AnalisadorSintatico::ehTerminal(const std::string& s) const {
    // tudo que NÃO é LHS de produção é terminal
    for (auto& p : gramatica)
        if (p.lhs == s) return false;
    return true;
}

bool AnalisadorSintatico::nullable(const std::vector<std::string>& rhs) const {
    return rhs.size() == 1 && rhs[0] == "ε";
}

// ========================
// FIRST
// ========================

void AnalisadorSintatico::calcularFirst() {
    bool mudou = true;

    while (mudou) {
        mudou = false;

        for (auto& p : gramatica) {
            auto& A = p.lhs;
            auto& rhs = p.rhs;

            // ε
            if (nullable(rhs)) {
                if (!FIRST[A].count("ε")) {
                    FIRST[A].insert("ε");
                    mudou = true;
                }
                continue;
            }

            // caso geral
            bool tudoNullable = true;

            for (auto& X : rhs) {
                if (ehTerminal(X)) {
                    if (!FIRST[A].count(X)) {
                        FIRST[A].insert(X);
                        mudou = true;
                    }
                    tudoNullable = false;
                    break;
                } else {
                    for (auto& f : FIRST[X])
                        if (f != "ε")
                            if (!FIRST[A].count(f)) {
                                FIRST[A].insert(f);
                                mudou = true;
                            }

                    if (!FIRST[X].count("ε")) {
                        tudoNullable = false;
                        break;
                    }
                }
            }

            if (tudoNullable) {
                if (!FIRST[A].count("ε")) {
                    FIRST[A].insert("ε");
                    mudou = true;
                }
            }
        }
    }
}

// ========================
// FOLLOW
// ========================

void AnalisadorSintatico::calcularFollow() {
    FOLLOW["PROGRAM"].insert("$");

    bool mudou = true;

    while (mudou) {
        mudou = false;

        for (auto& p : gramatica) {
            auto& A = p.lhs;
            auto& rhs = p.rhs;

            for (int i = 0; i < (int)rhs.size(); i++) {
                auto& B = rhs[i];

                if (ehTerminal(B)) continue;

                bool tudoNullable = true;

                for (int j = i + 1; j < (int)rhs.size(); j++) {
                    auto& X = rhs[j];

                    for (auto& f : FIRST[X])
                        if (f != "ε")
                            if (!FOLLOW[B].count(f)) {
                                FOLLOW[B].insert(f);
                                mudou = true;
                            }

                    if (!FIRST[X].count("ε")) {
                        tudoNullable = false;
                        break;
                    }
                }

                if (tudoNullable) {
                    for (auto& f : FOLLOW[A])
                        if (!FOLLOW[B].count(f)) {
                            FOLLOW[B].insert(f);
                            mudou = true;
                        }
                }
            }
        }
    }
}

// ========================
// TABELA LL1
// ========================

void AnalisadorSintatico::montarTabela() {
    for (auto& p : gramatica) {
        auto& A = p.lhs;
        auto& rhs = p.rhs;

        std::set<std::string> first_alpha;

        if (nullable(rhs)) {
            first_alpha.insert("ε");
        } else {
            for (auto& X : rhs) {
                if (ehTerminal(X)) {
                    first_alpha.insert(X);
                    break;
                } else {
                    for (auto& f : FIRST[X])
                        if (f != "ε") first_alpha.insert(f);

                    if (!FIRST[X].count("ε")) break;
                }
            }
        }

        for (auto& t : first_alpha) {
            if (t != "ε")
                tabela[A][t] = rhs;
        }

        if (first_alpha.count("ε")) {
            for (auto& b : FOLLOW[A]) {
                tabela[A][b] = rhs;
            }
        }
    }
}

// ========================
// PARSER
// ========================

AnalisadorSintatico::AnalisadorSintatico(
    const std::string& caminhoGramatica,
    const std::vector<std::string>& tokensEntrada
) :
    tokens(tokensEntrada)
{
    pilha.push_back("$");
    pilha.push_back("PROGRAM");

    carregarGramatica(caminhoGramatica);
    calcularFirst();
    calcularFollow();
    montarTabela();
}

std::string AnalisadorSintatico::tokenAtual() const {
    if (pos < tokens.size()) return tokens[pos];
    return "$";
}

void AnalisadorSintatico::consumir() {
    if (pos < tokens.size()) pos++;
}

bool AnalisadorSintatico::analisar() {
    while (!pilha.empty()) {
        std::string topo = pilha.back();
        std::string tk = tokenAtual();

        bool naoTerminal = tabela.count(topo);

        if (!naoTerminal) { 
            if (topo == tk) {
                pilha.pop_back();
                consumir();
            } else {
                std::cerr << "Erro: esperado '" << topo
                          << "', encontrou '" << tk << "'\n";
                return false;
            }
        } else {
            if (!tabela[topo].count(tk)) {
                std::cerr << "Erro sintático: token '" << tk
                          << "' não pode iniciar produções de <"
                          << topo << ">\n";
                return false;
            }

            auto rhs = tabela[topo][tk];
            pilha.pop_back();

            for (auto it = rhs.rbegin(); it != rhs.rend(); ++it)
                if (*it != "ε")
                    pilha.push_back(*it);
        }
    }

    return tokenAtual() == "$";
}
