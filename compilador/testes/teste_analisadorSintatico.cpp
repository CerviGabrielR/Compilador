#include <iostream>
#include <vector>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorSintatico.hpp"

int main() {

    // --------------------------------------
    // 1. CÓDIGO-FONTE PARA TESTAR
    // --------------------------------------
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



    // --------------------------------------
    // 2. ANALISADOR LÉXICO
    // --------------------------------------
    AnalisadorLexico lex;

    if (!lex.analisar(codigo)) {
        std::cout << "[ERRO LÉXICO] Código inválido!\n";
        return 1;
    }

    std::cout << "=== TOKENS GERADOS ===\n";
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

    std::cout << "\n\n=== INICIANDO ANÁLISE SINTÁTICA ===\n\n";



    // --------------------------------------
    // 3. ANALISADOR SINTÁTICO LL(1)
    // --------------------------------------
    // Ajuste o caminho da gramática se necessário
    std::string caminhoGram = "../gramaticaLL1.txt";

    AnalisadorSintatico sint(caminhoGram, lex.getTokens());

    bool ok = sint.analisar();

    if (ok) {
        std::cout << "\n[SINTÁTICO] ✔️ Código aceito pela gramática!\n";
    } else {
        std::cout << "\n[SINTÁTICO] ❌ Código rejeitado pela gramática!\n";
    }

    return ok ? 0 : 1;
}
