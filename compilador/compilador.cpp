#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "AnalisadorLexico.hpp"
#include "AnalisadorSintatico.hpp"
#include "AnalisadorSemantico.hpp"
#include "AnalisadorGCI.hpp"

static std::string lerArquivo(const std::string& caminho) {
    std::ifstream f(caminho);
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

static void listarOpcoes(const std::vector<std::string>& arquivos) {
    std::cout << "\nSelecione o programa para rodar:\n";
    for (std::size_t i = 0; i < arquivos.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << arquivos[i] << "\n";
    }
    std::cout << "  [0] Sair\n";
}

static void imprimirTabelaSimbolos(const std::unordered_map<std::string, std::string>& tabela) {
    std::cout << "\nTABELA DE SIMBOLOS (nome -> tipo)\n";
    for (const auto& [nome, tipo] : tabela) {
        std::cout << " - " << nome << " : " << tipo << "\n";
    }
}

static bool pipeline(const std::string& codigoFonte, const std::string& gramatica) {
    AnalisadorLexico lex;
    if (!lex.analisar(codigoFonte)) {
        std::cerr << "[FALHA] Erro na análise léxica.\n";
        return false;
    }
    std::cout << "[OK] Análise léxica\n";

    AnalisadorSintatico sint(gramatica, lex.getTokens());
    if (!sint.analisar()) {
        std::cerr << "[FALHA] Erro na análise sintática.\n";
        return false;
    }
    std::cout << "[OK] Análise sintática\n";

    AnalisadorSemantico sem(lex.getTokensInfo());
    if (!sem.analisar()) {
        std::cerr << "[FALHA] Erro na análise semântica.\n";
        return false;
    }
    std::cout << "[OK] Análise semântica\n";
    imprimirTabelaSimbolos(sem.getCollectedSymbols());

    AnalisadorGCI gci(lex.getTokensInfo());
    if (!gci.gerar()) {
        std::cerr << "[FALHA] Erro na geração de código intermediário.\n";
        return false;
    }
    std::cout << "\nCODIGO INTERMEDIARIO (3 enderecos):\n";
    for (const auto& instr : gci.getCodigo()) {
        std::cout << instr << "\n";
    }
    std::cout << "\n[SUCESSO] Código aceito e GCI gerado.\n";
    return true;
}

int main(int argc, char** argv) {
    std::vector<std::string> arquivos = {
        "testes/programa1.ccc",
        "testes/programa2.ccc",
        "testes/programa3.ccc",
        "testes/programa_invalido.ccc"
    };

    if (argc > 1) {
        arquivos.clear();
        for (int i = 1; i < argc; ++i) {
            arquivos.emplace_back(argv[i]);
        }
    }

    std::string gramatica = "gramaticaLL1.txt";
    bool tudoOk = true;

    while (true) {
        if (arquivos.empty()) {
            std::cerr << "Nenhum arquivo especificado.\n";
            return 1;
        }
        listarOpcoes(arquivos);
        std::cout << "Digite a opção: ";
        int opcao;
        if (!(std::cin >> opcao)) {
            std::cerr << "Entrada inválida.\n";
            return 1;
        }
        if (opcao == 0) break;
        if (opcao < 1 || opcao > static_cast<int>(arquivos.size())) {
            std::cout << "Opção inválida.\n";
            continue;
        }
        const auto& caminho = arquivos[opcao - 1];
        std::cout << "\n==============================\n";
        std::cout << "Arquivo: " << caminho << "\n";
        auto fonte = lerArquivo(caminho);
        if (fonte.empty()) {
            std::cerr << "[FALHA] Não foi possível ler " << caminho << "\n";
            tudoOk = false;
            continue;
        }
        bool ok = pipeline(fonte, gramatica);
        tudoOk = tudoOk && ok;
    }

    return tudoOk ? 0 : 1;
}
