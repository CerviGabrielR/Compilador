# Compilador ConvCC-2025-2

Este projeto implementa as fases de análise (léxica, sintática, semântica) e geração de código intermediário (3 endereços) para a linguagem ConvCC-2025-2.

## Requisitos
- C++17 (`g++` 11+)
- Docker instalado (para usar o fluxo com container)

## Estrutura principal
- `AnalisadorLexico.*`, `AnalisadorSintatico.*`, `AnalisadorSemantico.*`, `AnalisadorGCI.*`
- `compilador.cpp`: driver com menu que roda o pipeline completo.
- `gramaticaLL1.txt`: gramática usada pelo sintático.
- `codigos/`: programas de exemplo (.ccc) válidos e inválidos.
- `testes/`: bateria de testes automatizados.

## Como compilar e executar
Requer toolchain compatível com o enunciado (g++ 11.4). Se possível, use Docker para garantir a versão.

```bash
# Build da imagem (Ubuntu 22.04 com g++ 11.4)
docker build -t compilador .

# Compilar e rodar todos os testes
docker run --rm compilador make tests

# Trabalhar dentro do container (código da imagem criada no build)
docker run --rm -it compilador bash
# dentro do container: make compilador && ./compilador

# Opcional: montar o diretório local no container (para editar sem rebuild)
docker run --rm -it -v "$PWD":/app compilador bash
# dentro do container: make tests
```

### Ambiente local (sem Docker)
Certifique-se de ter `g++-11` (ou `g++` >= 11.4) disponível no PATH.
```bash
make tests       # compila e roda todos os testes
make compilador  # compila o binário
./compilador     # menu interativo
```

### Selecionando arquivos manualmente
Após compilar (`make compilador` no ambiente escolhido), você pode passar arquivos na linha de comando para o binário:
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

Use `make tests` (no container ou local) para compilar e executar todos; os binários ficam em `build/tests/`.

## Saídas esperadas
- Em caso de sucesso: tabela de símbolos (nome->tipo) e código intermediário gerado.
- Em caso de erro: primeira falha encontrada (léxica, sintática ou semântica) é reportada com linha/coluna. As mensagens do sintático são silenciosas (retornam false).

## Gramática ConvCC-2025-2
A gramática originalmente disponibilizada para o trabalho (CC-2025-2) estava na forma BNF (Backus-Naur Form) e para o trabalho era necessário que a gramática utilizada estivesse de acordo com LL1, portanto foram feitas as seguintes alterações na gramática para transformá-la na ConvCC-2025-2 com as características pedidas:
1. Deixar a gramática na forma convencional através da remoção dos operadores opcional(?), Fechamentos de Kleene (*) e Fechamentos Positivo (+)
2. Separação produções com o operador ou (|) em produções diferentes. Ex: S -> (a|b) => S -> a  S -> b
3. Transformação para LL1 por meio da remoção de recursão à esquerda e aplicação da fatoração à esquerda
4. Resolução de ambiguidades de análise da tabela LL1, isto é, células com múltiplas produções possíveis para um par não-terminal/terminal
   - `ATRIB_TAIL` apresentava conflito de *lookahead* ao tentar derivar `ident`, que poderia pertencer a uma **variável simples** (via `EXPRESSION`) ou a uma **chamada de função** (via `FUNCCALL`).
        * **Solução:** A produção que gerava `ident` foi "subida" e refatorada para `ident FUNCCALL_OR_ATRIB`. As produções de `EXPRESSION` e `FUNCCALL` que geravam o conflito foram reestruturadas e movidas para que o analisador pudesse decidir, com um *lookahead* (o próximo *token*), o caminho correto a seguir.
    - Ambiguidade do `if-else` (Dangling Else): O clássico problema do `if-else` ambíguo (não saber a qual `if` um `else` aninhado pertence).
        * **Solução:** Implementamos uma solução que força o emparelhamento, garantindo que o `else` pertença ao `if` mais próximo:
            1.  O não-terminal `IFSTAT` sempre deriva um `ELSESTAT`.
            2.  `ELSESTAT` é definido para derivar `else STATEMENT` (para a cláusula `else` presente) **ou** o **símbolo vazio ($\epsilon$)** (para a cláusula ausente).
            3.  O `STATEMENT` decorrente de `IFSTAT` foi restrito a `'{ STATELIST }'`, o que garante que o bloco do `if` sempre crie um novo escopo. Isso ajuda a estruturar a gramática para que o par `if-else` seja resolvido no escopo correto.
