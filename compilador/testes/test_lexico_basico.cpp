#include <iostream>
#include <string>
#include "../AnalisadorLexico.hpp"

int main() {
    AnalisadorLexico lex;

    std::string codigoValido = R"(
        int x;
        x = 10 + 2;
    )";

    std::cout << "=== LEXICO - CASO VALIDO ===\n";
    if (!lex.analisar(codigoValido)) {
        std::cout << "[FALHA] Não deveria falhar no válido.\n";
        return 1;
    }
    for (auto& t : lex.getTokens()) std::cout << t << " ";
    std::cout << "\n\n";

    std::string codigoInvalido = R"(
        int x;
        x = 10 $ 2;
    )";

    std::cout << "=== LEXICO - CASO INVALIDO ===\n";
    bool ok = lex.analisar(codigoInvalido);
    if (ok) {
        std::cout << "[FALHA] Token inválido não detectado.\n";
        return 1;
    } else {
        std::cout << "[OK] Erro léxico detectado.\n";
    }
    return 0;
}
