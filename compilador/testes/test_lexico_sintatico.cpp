#include <iostream>
#include <string>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorSintatico.hpp"

int main() {
    std::string codigoValido = R"(
        def main(int a) {
            int x;
            x = a + 1;
        }
    )";

    std::string codigoInvalido = R"(
        def main(int a {
            int x;
            x = a + ;
        }
    )"; // falta ')' e expressão

    AnalisadorLexico lex;

    std::cout << "=== LEX+SINT - VALIDO ===\n";
    if (!lex.analisar(codigoValido)) {
        std::cout << "[FALHA] léxico\n";
        return 1;
    }
    AnalisadorSintatico sintVal("gramaticaLL1.txt", lex.getTokens());
    if (!sintVal.analisar()) {
        std::cout << "[FALHA] sintático no válido\n";
        return 1;
    }
    std::cout << "[OK] válido aceito\n\n";

    std::cout << "=== LEX+SINT - INVALIDO ===\n";
    if (!lex.analisar(codigoInvalido)) {
        std::cout << "[OK] léxico já rejeitou\n";
        return 0;
    }
    AnalisadorSintatico sintInv("gramaticaLL1.txt", lex.getTokens());
    bool okSint = sintInv.analisar();
    if (okSint) {
        std::cout << "[FALHA] sintático não detectou erro\n";
        return 1;
    } else {
        std::cout << "[OK] erro sintático detectado\n";
    }
    return 0;
}
