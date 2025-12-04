#include "../AnalisadorSintatico.hpp"
#include <iostream>
#include <vector>
using namespace std;

// =======================================================
// TOKENS VÁLIDOS: pequeno programa correto
// int x;
// x = 5;
// =======================================================
vector<string> gerarTokensValidos() {
    return {
        "int", "id", ";",
        "int", "id", ";",
        "id", "=", "int_constant", ";",
        "id", "=", "id", "+", "int_constant", ";",
        "print", "id", ";",
        "$"
    };
}

// =======================================================
// TOKENS INVÁLIDOS: faltou ';'
// int x
// =======================================================
vector<string> gerarTokensInvalidos() {
    return {
        "int", "id",
        "$"
    };
}

int main() {
    string gramatica = "../gramaticaLL1.txt"; 
    // <-- um nível acima da pasta src

    cout << "=== TESTE 1: código válido ===\n";
    AnalisadorSintatico as1(gramatica, gerarTokensValidos());
    bool ok1 = as1.analisar();
    cout << (ok1 ? "ACEITO\n" : "REJEITADO\n");

    cout << "\n=== TESTE 2: código inválido ===\n";
    AnalisadorSintatico as2(gramatica, gerarTokensInvalidos());
    bool ok2 = as2.analisar();
    cout << (ok2 ? "ACEITO (ERRO!)\n" : "REJEITADO (OK)\n");

    return 0;
}
