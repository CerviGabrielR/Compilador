#include <iostream>
#include <vector>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorSintatico.hpp" // Não esqueça de incluir o header do sintático

int main() {
    // 1. Instancia os dois analisadores
    AnalisadorLexico lex;
    AnalisadorSintatico sintatico("../gramaticaLL1.txt", lex.getTokensInfo());


    // Código de teste (Válido segundo sua gramática)
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

    std::cout << "=== 1. CÓDIGO FONTE ===\n";
    std::cout << codigo << "\n\n";

    // 2. Executa a Análise Léxica
    std::cout << "=== 2. ANÁLISE LÉXICA ===\n";
    if (!lex.analisar(codigo)) {
        std::cerr << "[ERRO] Falha na análise léxica. O processo foi interrompido.\n";
        return 1;
    }
    std::cout << ">> Sucesso! Tokens gerados.\n";

    // (Opcional) Mostra os tokens para debug
    // for (const auto& t : lex.getTokens()) { std::cout << "[" << t << "] "; }
    // std::cout << "\n\n";

    // 3. Executa a Análise Sintática
    // Passamos a lista de tokens gerada pelo léxico para o sintático
    std::cout << "\n=== 3. ANÁLISE SINTÁTICA ===\n";
    
    bool sintaticoSucesso = sintatico.analisar();

    if (sintaticoSucesso) {
        std::cout << "\n[SUCESSO] O código foi aceito pela gramática!\n";
        
        // Imprime a tabela de símbolos (que foi populada pelo Léxico)
        std::cout << "\n=== TABELA DE SÍMBOLOS (Estado Final) ===\n";
        const auto& tabela = lex.getSymbolTable();
        for (const auto& [nome, simb] : tabela) {
            std::cout << "Nome: " << nome 
                      << " | Endereço: " << simb.address << "\n";
        }
        
        return 0;
    } else {
        std::cerr << "\n[FALHA] Erros sintáticos encontrados.\n";
        return 1;
    }
}
