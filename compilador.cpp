// Integrantes: Antônio Torres; Gustavo Bodi; Lucas Brand; Ewaldo Uhlmann; Gabriel Cervi
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "AnalisadorLexico.hpp"
#include "AnalisadorSintatico.hpp"
#include "AnalisadorSemantico.hpp"
#include "AnalisadorGCI.hpp"
#include "Simbolo.hpp"

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

static void imprimirTabelaSimbolosSemantico(const std::vector<AnalisadorSemantico::ScopeSymbols>& scopes) {
    std::cout << "\nTABELA DE SIMBOLOS POR ESCOPO (nome -> tipo)\n";
    for (std::size_t i = 0; i < scopes.size(); ++i) {
        std::cout << " Escopo #" << i << "\n";
        for (const auto& [nome, tipo] : scopes[i]) {
            std::cout << "  - " << nome << " : " << tipo << "\n";
        }
    }
}

static void imprimirTabelaSimbolosLexico(const std::unordered_map<std::string, Simbolo>& tabelaLex,
                                         const std::unordered_map<std::string, std::string>& tipos) {
    std::cout << "\nTABELA DE SIMBOLOS (lexico) com ocorrencias e tipo inferido\n";
    for (const auto& [nome, simb] : tabelaLex) {
        auto it = tipos.find(nome);
        std::string tipo = it != tipos.end() ? it->second : "?";
        std::cout << " - " << nome << " : tipo=" << tipo << " | ocorrencias=";
        for (std::size_t i = 0; i < simb.occurrences.size(); ++i) {
            const auto& oc = simb.occurrences[i];
            std::cout << "(" << oc.line << "," << oc.column << ")";
            if (i + 1 < simb.occurrences.size()) std::cout << ", ";
        }
        std::cout << "\n";
    }
}

static void coletarPreOrdem(const std::shared_ptr<AnalisadorSemantico::ExprNode>& node,
                            std::vector<std::string>& out) {
    if (!node) return;
    out.push_back(node->label);
    coletarPreOrdem(node->left, out);
    coletarPreOrdem(node->right, out);
}

static void imprimirArvoreExpressao(const std::shared_ptr<AnalisadorSemantico::ExprNode>& node,
                                    std::size_t idx) {
    std::vector<std::string> ordem;
    coletarPreOrdem(node, ordem);
    std::cout << "Expr " << idx << ":";
    for (const auto& s : ordem) {
        std::cout << " " << s;
    }
    std::cout << "\n";
}

static bool pipeline(const std::string& codigoFonte, const std::string& gramatica) {
    AnalisadorLexico lex;
    if (!lex.analisar(codigoFonte)) {
        std::cerr << "[FALHA] Erro na análise léxica.\n";
        return false;
    }
    std::cout << "[OK] Análise léxica\n";

    AnalisadorSintatico sint(gramatica, lex.getTokensInfo());
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
    auto tipos = sem.getCollectedSymbols();
    imprimirTabelaSimbolosLexico(lex.getSymbolTable(), tipos);
    imprimirTabelaSimbolosSemantico(sem.getScopesSymbols());
    std::cout << "[OK] Expressões aritméticas válidas\n";
    std::cout << "[OK] Declarações por escopo válidas\n";
    std::cout << "[OK] Todos os break estão dentro de laços\n";
    const auto& arvores = sem.getExpressionTrees();
    if (!arvores.empty()) {
        std::cout << "\nARVORES DE EXPRESSAO (pre-ordem raiz-esq-dir):\n";
        std::size_t idx = 1;
        for (const auto& arv : arvores) {
            imprimirArvoreExpressao(arv, idx++);
        }
    }

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
    const std::string pastaCodigos = "codigos/";
    std::vector<std::string> arquivos = {
        pastaCodigos + "programa1.ccc",
        pastaCodigos + "programa2.ccc",
        pastaCodigos + "programa3.ccc",
        pastaCodigos + "programa_invalido.ccc",        // erro semantico
        pastaCodigos + "programa_invalido_sintaxe.ccc",// erro sintatico
        pastaCodigos + "programa_invalido_lexico.ccc"  // erro lexico
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
