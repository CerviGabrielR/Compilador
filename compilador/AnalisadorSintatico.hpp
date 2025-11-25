#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>

class AnalisadorSintatico {
public:

    struct Producao {
        std::string lhs;
        std::vector<std::string> rhs;
    };

    using Gramatica = std::vector<Producao>;
    using TabelaLL1 = std::map<std::string, std::map<std::string, std::vector<std::string>>>;

    AnalisadorSintatico(const std::string& caminhoGramatica,
                        const std::vector<std::string>& tokens);

    bool analisar();

private:
    Gramatica gramatica;
    TabelaLL1 tabela;
    std::vector<std::string> tokens;

    size_t pos = 0;
    std::vector<std::string> pilha;

    // carregamento
    void carregarGramatica(const std::string& caminho);

    // auxiliares
    std::map<std::string, std::set<std::string>> FIRST;
    std::map<std::string, std::set<std::string>> FOLLOW;

    bool ehTerminal(const std::string& s) const;
    bool nullable(const std::vector<std::string>& rhs) const;

    // cálculos
    void calcularFirst();
    void calcularFollow();
    void montarTabela();

    // parser
    std::string tokenAtual() const;
    void consumir();
};
