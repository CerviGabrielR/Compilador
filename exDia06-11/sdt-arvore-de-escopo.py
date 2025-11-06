import sys

# ----------------------------------------------------
# 1. Classe para a Árvore de Escopos
# ----------------------------------------------------
class EscopoNode:
    """Representa um nó na árvore de escopos."""
    
    # Contador estático para dar um ID único a cada escopo
    _id_counter = 0
    
    def __init__(self, pai=None):
        self.id = EscopoNode._id_counter
        EscopoNode._id_counter += 1
        
        self.pai = pai
        self.filhos = []
        
        # Vincula-se ao pai, se ele existir
        if self.pai:
            self.pai.filhos.append(self)
            
    def __repr__(self):
        """Representação em string para depuração."""
        if self.pai is None:
            return f"Escopo(id={self.id}, [RAIZ])"
        return f"Escopo(id={self.id}, pai={self.pai.id})"

    def imprimir_arvore(self, prefixo=""):
        """Imprime a árvore de escopos de forma legível."""
        conector = "└── " if prefixo.count("│") > 0 else "    "
        if self.pai is None:
            print("Escopo-Raiz (id=0)")
            conector = ""
        else:
            print(f"{prefixo}{conector}{self}")
            
        for i, filho in enumerate(self.filhos):
            novo_prefixo = prefixo
            if self.pai is not None:
                novo_prefixo += "    " if i == len(self.filhos) - 1 else "│   "
            
            filho.imprimir_arvore(novo_prefixo)

# ----------------------------------------------------
# 2. Classe do Analisador Sintático (Parser)
# ----------------------------------------------------
class AnalisadorSintatico:
    """
    Implementa o Analisador LL(1) e a SDT para construir 
    a árvore de escopos.
    """
    
    def __init__(self, entrada):
        self.entrada = entrada + '$' # Adiciona marcador de fim
        self.posicao = 0
        self.raiz_escopo = None
        
        # Reseta o contador de IDs para cada nova análise
        EscopoNode._id_counter = 0

    def lookahead(self):
        """Retorna o token atual sem consumi-lo."""
        if self.posicao < len(self.entrada):
            return self.entrada[self.posicao]
        return '$' # Fim da entrada

    def match(self, terminal_esperado):
        """Consome o token atual se ele for o esperado."""
        if self.lookahead() == terminal_esperado:
            print(f"Match: consumiu '{terminal_esperado}'")
            self.posicao += 1
        else:
            raise SyntaxError(
                f"Erro de sintaxe: Esperava '{terminal_esperado}', "
                f"encontrou '{self.lookahead()}' na posição {self.posicao}"
            )

    # --- Ação Semântica ---
    def criaEscopo(self, escopo_pai):
        """
        Função da SDT: Cria um novo nó de escopo
        e o retorna.
        """
        novo_escopo = EscopoNode(pai=escopo_pai)
        print(f"--- AÇÃO: criei {novo_escopo} ---")
        return novo_escopo

    # --- Ponto de Entrada ---
    def analisar(self):
        """
        Inicia a análise sintática.
        Implementa a "Inicialização" da SDT.
        """
        print(f"Iniciando análise para: '{self.entrada}'")
        
        # Ação de inicialização: { raiz = criaEscopo(null); }
        self.raiz_escopo = self.criaEscopo(None)
        
        # Chama o símbolo inicial S (que é E), passando o escopo raiz.
        # E.escopo = self.raiz_escopo
        self.E(self.raiz_escopo)
        
        # Verifica se toda a entrada foi consumida
        if self.lookahead() == '$':
            print("Análise concluída com sucesso.")
        else:
            raise SyntaxError(
                f"Erro: entrada não consumida. "
                f"Parou em '{self.lookahead()}'"
            )
        
        return self.raiz_escopo

    # --- Regras da Gramática (SDT) ---
    
    def E(self, E_escopo):
        """
        Produção 1: E -> A
        SDT: E -> { A.escopo = E.escopo; } A
        """
        print(f"-> Analisando E(escopo={E_escopo})")
        
        # Ação: A.escopo = E.escopo
        A_escopo = E_escopo
        self.A(A_escopo)

    def A(self, A_escopo):
        """
        Produção 2: A -> { E1 } E2
        SDT: A -> { { E1.escopo = criaEscopo(A.escopo); } E1 } 
                   { { E2.escopo = A.escopo; } E2 }
        
        Produção 3: A -> ε
        SDT: A -> ε { }
        """
        print(f"-> Analisando A(escopo={A_escopo})")
        
        token = self.lookahead()
        
        if token == '{':
            # Aplicar Produção 2: A -> { E1 } E2
            self.match('{')
            
            # Ação 1: { E1.escopo = criaEscopo(A.escopo); }
            E1_escopo = self.criaEscopo(A_escopo)
            self.E(E1_escopo) # Chama E1
            
            self.match('}')
            
            # Ação 2: { E2.escopo = A.escopo; }
            E2_escopo = A_escopo
            self.E(E2_escopo) # Chama E2
            
        elif token == '}' or token == '$':
            # Aplicar Produção 3: A -> ε
            # Ação: { } (não faz nada)
            print(f"-> Reconheceu A -> ε (lookahead='{token}')")
            pass
        
        else:
            raise SyntaxError(
                f"Erro de sintaxe: Inesperado '{token}' na regra A"
            )

# ----------------------------------------------------
# 3. Execução
# ----------------------------------------------------
if __name__ == "__main__":
    
    # Palavra de exemplo do exercício
    palavra = "{}{{}}"
    
    try:
        analisador = AnalisadorSintatico(palavra)
        arvore_final = analisador.analisar()
        
        print("\n" + "="*40)
        print("      ÁRVORE DE ESCOPOS RESULTANTE")
        print("="*40)
        arvore_final.imprimir_arvore()
        
        print("\n" + "="*40)
        print("      EXEMPLO 2: aninhado")
        print("="*40)
        
        palavra_2 = "{{{}}}"
        analisador_2 = AnalisadorSintatico(palavra_2)
        arvore_2 = analisador_2.analisar()
        print("\n--- Árvore 2 ---")
        arvore_2.imprimir_arvore()

    except (SyntaxError, Exception) as e:
        print(f"\nErro durante a análise: {e}", file=sys.stderr)