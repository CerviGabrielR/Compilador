#include <iostream>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorSemantico.hpp"

int main() {
    std::string programaValido = R"(
        def main() {
            int x;
            float y;
            x = 10;
            y = x + 2.5;

            for (x = 0; x < 3; x = x + 1) {
                print y;
            }

            return y;
        }
    )";

    std::string programaInvalido = R"(
        def main() {
            int x;
            x = "texto";
            break;
        }
    )";

    {
        std::cout << "=== TESTE SEMANTICO (programa valido) ===\n";
        AnalisadorLexico lex;
        if (!lex.analisar(programaValido)) {
            std::cerr << "Falha na análise léxica do programa válido\n";
            return 1;
        }
        AnalisadorSemantico sem(lex.getTokensInfo());
        bool ok = sem.analisar();
        std::cout << (ok ? "Semantica OK\n" : "Semantica falhou\n") << "\n";
    }

    {
        std::cout << "=== TESTE SEMANTICO (programa invalido) ===\n";
        AnalisadorLexico lex;
        if (!lex.analisar(programaInvalido)) {
            std::cerr << "Falha na análise léxica do programa inválido\n";
            return 1;
        }
        AnalisadorSemantico sem(lex.getTokensInfo());
        bool ok = sem.analisar();
        std::cout << (ok ? "Semantica OK (inesperado)\n" : "Semantica detectou erro\n");
    }
}
