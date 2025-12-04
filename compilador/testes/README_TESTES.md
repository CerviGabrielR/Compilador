# Testes do compilador

Organização em camadas:
- `test_lexico_basico.cpp`: valida tokens básicos e rejeição de caractere inválido.
- `test_lexico_sintatico.cpp`: passa por léxico+sintático (caso válido e inválido).
- `test_semantico_basico.cpp`: checa regras semânticas (tipos e break fora de laço).
- `test_gci_basico.cpp`: gera código intermediário para um programa simples.
- `test_pipeline_validos.cpp`: pipeline completo (léxico+sintático+semântico+GCI) para `programa1/2/3.ccc`.
- `test_pipeline_invalidos.cpp`: pipeline que deve falhar nos programas inválidos.

Programas de entrada:
- `programa1.ccc`, `programa2.ccc`, `programa3.ccc`: válidos (100+ linhas).
- `programa_invalido.ccc`: erro semântico (atribuição string em int e break fora de laço).
- `programa_invalido_sintaxe.ccc`: erro sintático (falta ';' e expressão).

Compilação rápida (exemplos):
```
# Léxico
g++ -std=c++17 -I.. test_lexico_basico.cpp ../AnalisadorLexico.cpp -o test_lexico_basico

# Léxico + Sintático
g++ -std=c++17 -I.. test_lexico_sintatico.cpp ../AnalisadorLexico.cpp ../AnalisadorSintatico.cpp -o test_lexico_sintatico

# Semântico
g++ -std=c++17 -I.. test_semantico_basico.cpp ../AnalisadorLexico.cpp ../AnalisadorSemantico.cpp -o test_semantico_basico

# GCI
g++ -std=c++17 -I.. test_gci_basico.cpp ../AnalisadorLexico.cpp ../AnalisadorGCI.cpp -o test_gci_basico

# Pipelines
g++ -std=c++17 -I.. test_pipeline_validos.cpp ../AnalisadorLexico.cpp ../AnalisadorSintatico.cpp ../AnalisadorSemantico.cpp ../AnalisadorGCI.cpp -o test_pipeline_validos
g++ -std=c++17 -I.. test_pipeline_invalidos.cpp ../AnalisadorLexico.cpp ../AnalisadorSintatico.cpp ../AnalisadorSemantico.cpp -o test_pipeline_invalidos
```
