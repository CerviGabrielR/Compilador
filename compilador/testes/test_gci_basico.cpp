#include <iostream>
#include <string>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorGCI.hpp"

int main() {
    std::string codigo = R"(
        def main(int a) {
            int x;
            x = a + 1;
            print x;
            return x;
        }
    )";

    AnalisadorLexico lex;
    if (!lex.analisar(codigo)) {
        std::cerr << "[FALHA] léxico\n";
        return 1;
    }
    AnalisadorGCI gci(lex.getTokensInfo());
    if (!gci.gerar()) {
        std::cerr << "[FALHA] GCI\n";
        return 1;
    }
    std::cout << "=== GCI BASICO ===\n";
    for (auto& instr : gci.getCodigo()) std::cout << instr << "\n";
    return 0;
}
