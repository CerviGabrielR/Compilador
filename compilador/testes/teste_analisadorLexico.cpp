#include <iostream>
#include "../AnalisadorLexico.hpp"

int main() {

    AnalisadorLexico lex;

    std::string codigo = R"(

        def main() {
            int x;
            x = 10;

            if (x > 5) {
                print x;
            }

            return;
        }

        )";


    std::cout << "=== Código sendo analisado ===\n";
    std::cout << codigo << "\n\n";


    if (!lex.analisar(codigo)) {
        std::cout << "Erro léxico encontrado!\n";
        return 1;
    }

    std::cout << "=== TOKENS ===\n";
    for (const auto& t : lex.getTokens()) {
        std::cout << t << "\n";
    }

    std::cout << "\n=== TABELA DE SÍMBOLOS ===\n";

    const auto& tabela = lex.getSymbolTable();
    for (const auto& [nome, simb] : tabela) {
        std::cout << "Nome: " << nome
                  << "  | endereço: " << simb.address
                  << "  | ocorrências: ";

        for (auto& occ : simb.occurrences) {
            std::cout << "(" << occ.line << "," << occ.column << ") ";
        }
        std::cout << "\n";
    }

    return 0;
}
