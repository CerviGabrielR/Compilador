#include <iostream>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorSintatico.hpp"
#include "../AnalisadorSemantico.hpp"

int main() {
    std::string codigo = R"(
        def main(int a) {
            int x;
            float y;
            x = 10;
            y = x + 2.5;

            if (y > 0) {
                print y;
            } else {
                print x;
            }

            return y + a;
        }
    )";

    std::cout << "=== ETAPA DE ANALISE (LEXICO + SINTATICO + SEMANTICO) ===\n\n";
    std::cout << codigo << "\n";

    AnalisadorLexico lex;
    if (!lex.analisar(codigo)) {
        std::cerr << "[FALHA] Erro na análise léxica.\n";
        return 1;
    }
    std::cout << "[OK] Léxico aceitou.\n";

    AnalisadorSintatico sint("../gramaticaLL1.txt", lex.getTokens());
    if (!sint.analisar()) {
        std::cerr << "[FALHA] Erro na análise sintática.\n";
        return 1;
    }
    std::cout << "[OK] Sintático aceitou.\n";

    AnalisadorSemantico sem(lex.getTokensInfo());
    if (!sem.analisar()) {
        std::cerr << "[FALHA] Erro na análise semântica.\n";
        return 1;
    }
    std::cout << "[OK] Semântico aceitou.\n";

    std::cout << "\n=== SUCESSO: Código aceito em todas as fases. ===\n";
    return 0;
}
