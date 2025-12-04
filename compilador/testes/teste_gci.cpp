#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../AnalisadorLexico.hpp"
#include "../AnalisadorGCI.hpp"

std::string lerArquivo(const std::string& caminho) {
    std::ifstream f(caminho);
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

void rodar(const std::string& nomeArquivo) {
    std::cout << "\n=== GCI para " << nomeArquivo << " ===\n";
    auto codigoFonte = lerArquivo(nomeArquivo);
    AnalisadorLexico lex;
    if (!lex.analisar(codigoFonte)) {
        std::cerr << "Falha léxica em " << nomeArquivo << "\n";
        return;
    }

    AnalisadorGCI gci(lex.getTokensInfo());
    if (!gci.gerar()) {
        std::cerr << "Falha na geração de código em " << nomeArquivo << "\n";
        return;
    }

    for (const auto& instr : gci.getCodigo()) {
        std::cout << instr << "\n";
    }
}

int main() {
    std::vector<std::string> arquivos = {"programa1.ccc", "programa2.ccc", "programa3.ccc"};
    for (const auto& arq : arquivos) {
        rodar(arq);
    }
    return 0;
}
