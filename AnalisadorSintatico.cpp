// Integrantes: Antônio Torres; Gustavo Bodi; Lucas Brand; Ewaldo Uhlmann; Gabriel Cervi
#include "AnalisadorSintatico.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

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

                    // Se X é terminal, insere X diretamente em FOLLOW[B]
                    if (ehTerminal(X)) {
                        if (!FOLLOW[B].count(X)) {
                            FOLLOW[B].insert(X);
                            mudou = true;
                        }
                        tudoNullable = false; // terminal não é nullable
                        break;
                    }

                    // X é não-terminal: acrescenta FIRST(X) \ {ε} em FOLLOW(B)
                    for (auto& f : FIRST[X]) {
                        if (f == "ε") continue;
                        if (!FOLLOW[B].count(f)) {
                            FOLLOW[B].insert(f);
                            mudou = true;
                        }
                    }

                    // se X não gera ε, então paramos
                    if (!FIRST[X].count("ε")) {
                        tudoNullable = false;
                        break;
                    }

                    // caso X gere ε, continuamos para o próximo X
                }

                // se tudo depois de B pode gerar ε, então FOLLOW(A) ⊆ FOLLOW(B)
                if (tudoNullable) {
                    for (auto& f : FOLLOW[A]) {
                        if (!FOLLOW[B].count(f)) {
                            FOLLOW[B].insert(f);
                            mudou = true;
                        }
                    }
                }
            }
        }
    }
}


void AnalisadorSintatico::montarTabela() {
    for (auto& p : gramatica) {
        auto& A = p.lhs;
        auto& rhs = p.rhs;

        std::set<std::string> first_alpha;
        bool rhs_eh_nulavel = true; // Assumimos que a regra é nula até provar o contrário

        for (auto& X : rhs) {
            // Se encontrou epsilon explícito na regra (ex: A -> ε), continua sendo nulável
            if (X == "ε") continue; 

            if (ehTerminal(X)) {
                // Se encontrou um terminal (ex: '+', 'id'), a regra não é nula
                first_alpha.insert(X);
                rhs_eh_nulavel = false;
                break;
            } else {
                // É um Não-Terminal (ex: TERM_TAIL)
                // Adiciona tudo do FIRST(X) em FIRST(A), exceto o epsilon
                for (auto& f : FIRST[X]) {
                    if (f != "ε") first_alpha.insert(f);
                }

                // A regra só continua sendo nulável se X puder produzir epsilon
                if (!FIRST[X].count("ε")) {
                    rhs_eh_nulavel = false;
                    break;
                }
                // Se X produz epsilon, o loop continua para analisar o próximo símbolo
            }
        }

        // 1. Preenche a tabela com os terminais encontrados no FIRST
        for (auto& t : first_alpha) {
            tabela[A][t] = rhs;
        }

        // 2. Se a regra inteira pode sumir (ser nula), adiciona o FOLLOW(A)
        if (rhs_eh_nulavel) {
            for (auto& b : FOLLOW[A]) {
                // Apenas insere se não houver conflito ou sobrescreve (LL1 padrão)
                tabela[A][b] = rhs; 
            }
        }
    }
}


AnalisadorSintatico::AnalisadorSintatico(
    const std::string& caminhoGramatica,
    const std::vector<std::string>& tokensEntrada
) :
    tokens(tokensEntrada)
{
    pilha.push_back("$");
    pilha.push_back("PROGRAM");

    // Preenche tokensInfo com placeholders para permitir mensagens com posição.
    tokensInfo.reserve(tokensEntrada.size());
    for (std::size_t i = 0; i < tokensEntrada.size(); ++i) {
        tokensInfo.push_back({tokensEntrada[i], "", 0, i + 1});
    }

    carregarGramatica(caminhoGramatica);
    calcularFirst();
    calcularFollow();
    montarTabela();
}

AnalisadorSintatico::AnalisadorSintatico(
    const std::string& caminhoGramatica,
    const std::vector<AnalisadorLexico::TokenInfo>& tokensInfoEntrada
) {
    pilha.push_back("$");
    pilha.push_back("PROGRAM");
    tokensInfo = tokensInfoEntrada;
    tokens.reserve(tokensInfoEntrada.size());
    for (const auto& tk : tokensInfoEntrada) {
        tokens.push_back(tk.type);
    }
    carregarGramatica(caminhoGramatica);
    calcularFirst();
    calcularFollow();
    montarTabela();
}

std::string AnalisadorSintatico::tokenAtual() const {
    if (pos < tokens.size()) return tokens[pos];
    return "$";
}

const AnalisadorLexico::TokenInfo& AnalisadorSintatico::tokenInfoAtual() const {
    if (!tokensInfo.empty() && pos < tokensInfo.size()) return tokensInfo[pos];
    static AnalisadorLexico::TokenInfo dummy{"$", "", 0, 0};
    if (!tokensInfo.empty()) return tokensInfo.back();
    return dummy;
}

void AnalisadorSintatico::consumir() {
    if (pos < tokens.size()) pos++;
}

bool AnalisadorSintatico::reportMismatch(const std::string& esperado, const std::string& encontrado) {
    if (errorReported) return false;
    errorReported = true;
    auto tk = tokenInfoAtual();
    std::cerr << "[Erro sintático] Esperado '" << esperado << "' mas encontrou '" << encontrado
              << "' na linha " << tk.line << ", coluna " << tk.column << "\n";
    return false;
}

bool AnalisadorSintatico::reportTabelaVazia(const std::string& topo, const std::string& lookahead) {
    if (errorReported) return false;
    errorReported = true;
    auto tk = tokenInfoAtual();
    std::cerr << "[Erro sintático] Nenhuma produção para (" << topo << ", " << lookahead
              << ") na linha " << tk.line << ", coluna " << tk.column << "\n";
    return false;
}


bool AnalisadorSintatico::analisar() {
    while (!pilha.empty()) {
        std::string topo = pilha.back();
        std::string tk = tokenAtual();

        // Verifica se é Não-Terminal
        bool naoTerminal = !ehTerminal(topo);

        // CASO 1: O topo é um Terminal (ou $)
        if (!naoTerminal) { 
            if (topo == tk) {
                pilha.pop_back();
                consumir();
            } else {
                return reportMismatch(topo, tk);
            }
        } 
        // CASO 2: O topo é um Não-Terminal (Ex: PROGRAM, E, T)
        else {
            if (!tabela[topo].count(tk)) {
                return reportTabelaVazia(topo, tk);
            }

            auto rhs = tabela[topo][tk];

            pilha.pop_back();

            for (auto it = rhs.rbegin(); it != rhs.rend(); ++it)
                if (*it != "ε") // Não empilha epsilon
                    pilha.push_back(*it);
        }
    }

    // Validação final
    if (tokenAtual() == "$") {
        return true;
    } else {
        return false;
    }
}
