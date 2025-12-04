#include <iostream>
#include <string>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorSemantico.hpp"

int main() {
    std::string valido = R"(
        def main(int a) {
            int x;
            float y;
            x = a + 1;
            y = x + 2.0;
            return y;
        }
    )";

    std::string invalido = R"(
        def main() {
            int x;
            x = "texto";  // erro de tipo
            break;        // fora de laco
        }
    )";

    AnalisadorLexico lex;

    std::cout << "=== SEMANTICO - VALIDO ===\n";
    if (!lex.analisar(valido)) {
        std::cout << "[FALHA] lexico\n";
        return 1;
    }
    AnalisadorSemantico semOk(lex.getTokensInfo());
    if (!semOk.analisar()) {
        std::cout << "[FALHA] semantico no valido\n";
        return 1;
    }
    std::cout << "[OK] valido aceito\n\n";

    std::cout << "=== SEMANTICO - INVALIDO ===\n";
    if (!lex.analisar(invalido)) {
        std::cout << "[FALHA] lexico no invalido (deveria passar lexico)\n";
        return 1;
    }
    AnalisadorSemantico semBad(lex.getTokensInfo());
    if (semBad.analisar()) {
        std::cout << "[FALHA] semantico nao detectou erro\n";
        return 1;
    } else {
        std::cout << "[OK] erro semantico detectado\n";
    }
    return 0;
}
