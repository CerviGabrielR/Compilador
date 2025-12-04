#include <iostream>
#include <vector>
#include "../AnalisadorSintatico.hpp"

int main() {
    // ==========================================
    // TESTE 1: Sentença Válida
    // Expressão: id + id * id
    // ==========================================
    std::cout << "--- TESTE 1: Sentença Valida (id + id * id) ---\n";
    
    std::vector<std::string> tokensValidos = {"id", "+", "id", "*", "id"};
    
    // Instancia o analisador passando o arquivo da gramática e os tokens
    AnalisadorSintatico parser1("gramatica.txt", tokensValidos);
    
    if (parser1.analisar()) {
        std::cout << ">> SUCESSO: A cadeia foi aceita!\n";
    } else {
        std::cout << ">> FALHA: A cadeia foi rejeitada.\n";
    }

    std::cout << "\n";

    // ==========================================
    // TESTE 2: Sentença Inválida
    // Expressão: id + * id (erro de sintaxe)
    // ==========================================
    std::cout << "--- TESTE 2: Sentença Invalida (id + * id) ---\n";
    
    std::vector<std::string> tokensInvalidos = {"id", "+", "*", "id"};
    
    AnalisadorSintatico parser2("gramatica.txt", tokensInvalidos);
    
    if (parser2.analisar()) {
        std::cout << ">> SUCESSO: A cadeia foi aceita! (Inesperado)\n";
    } else {
        std::cout << ">> SUCESSO DO TESTE: A cadeia foi rejeitada corretamente.\n";
    }

    return 0;
}