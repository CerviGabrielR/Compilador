#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorSintatico.hpp"
#include "../AnalisadorSemantico.hpp"
#include "../AnalisadorGCI.hpp"

bool pipeline(const std::string& codigo) {
    AnalisadorLexico lex;
    if (!lex.analisar(codigo)) return false;
    AnalisadorSintatico sint("gramaticaLL1.txt", lex.getTokensInfo());
    if (!sint.analisar()) return false;
    AnalisadorSemantico sem(lex.getTokensInfo());
    if (!sem.analisar()) return false;
    AnalisadorGCI gci(lex.getTokensInfo());
    return gci.gerar();
}

std::string lerArquivo(const std::string& caminho) {
    std::ifstream f(caminho);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

int main() {
    std::vector<std::string> arquivos = {
        "programa1.ccc",
        "programa2.ccc",
        "programa3.ccc"
    };

    bool ok = true;
    for (auto& arq : arquivos) {
        auto codigo = lerArquivo(arq);
        std::cout << "Rodando pipeline em " << arq << "...\n";
        bool r = pipeline(codigo);
        std::cout << (r ? "[OK]\n" : "[FALHA]\n");
        ok = ok && r;
    }
    return ok ? 0 : 1;
}
