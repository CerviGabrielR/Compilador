#pragma once
#include <string>

#pragma once
#include <string>
#include <vector>

struct Ocorrencia {
    std::size_t line;
    std::size_t column;
};

struct Simbolo {
    std::string type;
    std::size_t address;
    std::vector<Ocorrencia> occurrences;
};
