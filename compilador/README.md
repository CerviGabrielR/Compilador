# Compilador ConvCC-2025-2

Este projeto implementa as fases de análise (léxica, sintática, semântica) e geração de código intermediário (3 endereços) para a linguagem ConvCC-2025-2.

## Requisitos
- C++17 (`g++` 11+)

## Estrutura principal
- `AnalisadorLexico.*`, `AnalisadorSintatico.*`, `AnalisadorSemantico.*`, `AnalisadorGCI.*`
- `compilador.cpp`: driver com menu que roda o pipeline completo.
- `gramaticaLL1.txt`: gramática usada pelo sintático.
- `codigos/`: programas de exemplo (.ccc) válidos e inválidos.
- `testes/`: bateria de testes automatizados.

## Como compilar e executar
```bash
# Na pasta compilador/
make            # compila e roda todos os testes, depois executa o compilador nos 3 programas válidos

# Apenas compilar e rodar o compilador com menu
make compilador
./compilador          # menu interativo (opções para programa1/2/3 válidos e programa_invalido)

# Rodar todos os testes
make tests
```

### Selecionando arquivos manualmente
Você pode passar arquivos na linha de comando para o binário:
```bash
./compilador codigos/programa1.ccc codigos/programa2.ccc
```
Caso não informe, o menu interativo permite escolher entre os programas de exemplo.

## Testes
A pasta `codigos/` contém os programas de entrada:
- Válidos: `programa1.ccc`, `programa2.ccc`, `programa3.ccc` (100+ linhas).
- Inválidos: `programa_invalido.ccc` (erro semântico), `programa_invalido_sintaxe.ccc` (erro sintático), `programa_invalido_lexico.ccc` (erro léxico).

Os testes em `testes/` são:
- `test_lexico_basico`, `test_semantico_basico`, `test_pipeline_validos`, `test_pipeline_invalidos`.

Use `make tests` para compilar e executar todos; os binários ficam em `build/tests/`.

## Saídas esperadas
- Em caso de sucesso: tabela de símbolos (nome->tipo) e código intermediário gerado.
- Em caso de erro: primeira falha encontrada (léxica, sintática ou semântica) é reportada com linha/coluna. As mensagens do sintático são silenciosas (retornam false).
