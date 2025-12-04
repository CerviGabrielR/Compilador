#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorSintatico.hpp"
#include "../AnalisadorSemantico.hpp"

std::string lerArquivo(const std::string& caminho) {
    std::ifstream f(caminho);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool pipeline(const std::string& codigo) {
    AnalisadorLexico lex;
    if (!lex.analisar(codigo)) return false;
    AnalisadorSintatico sint("gramaticaLL1.txt", lex.getTokensInfo());
    if (!sint.analisar()) return false;
    AnalisadorSemantico sem(lex.getTokensInfo());
    return sem.analisar();
}

int main() {
    // esperado falhar
    std::vector<std::string> invalidos = {
        "testes/programa_invalido.ccc",
        "testes/programa_invalido_sintaxe.ccc"
    };
    bool tudoFalhou = true;
    for (auto& arq : invalidos) {
        auto codigo = lerArquivo(arq);
        std::cout << "Rodando pipeline em " << arq << " (deve falhar)...\n";
        bool ok = pipeline(codigo);
        if (ok) {
            std::cout << "[FALHA] Aceitou incorretamente\n";
            tudoFalhou = false;
        } else {
            std::cout << "[OK] Rejeitou como esperado\n";
        }
    }
    return tudoFalhou ? 0 : 1;
}
