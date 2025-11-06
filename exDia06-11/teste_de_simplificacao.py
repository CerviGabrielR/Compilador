class Escopo:
    #Nó da árvore de escopos
    _id = 0

    def __init__(self, pai=None):
        self.id = Escopo._id
        Escopo._id += 1
        self.pai = pai
        self.filhos = []
        if pai:
            pai.filhos.append(self)

    def imprimir(self, prefixo=""):
        if self.pai is None:
            print(f"Escopo-Raiz (id={self.id})")
        else:
            print(f"{prefixo}└── Escopo(id={self.id}, pai={self.pai.id})")
        for i, f in enumerate(self.filhos):
            f.imprimir(prefixo + ("    " if i == len(self.filhos)-1 else "│   "))


class Parser:
    #Analisador LL1 com SDT para construir a árvore de escopos.
    def __init__(self, entrada):
        self.tokens = [c for c in entrada if c in "{}"]
        self.pos = 0
        Escopo._id = 0
        self.raiz = Escopo(None)

    def lookahead(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else '$'

    def match(self, t):
        if self.lookahead() == t:
            self.pos += 1
        else:
            raise SyntaxError(f"Esperava '{t}', encontrou '{self.lookahead()}'")

    def analisar(self):
        self.E(self.raiz)
        if self.lookahead() != '$':
            raise SyntaxError("Entrada não totalmente consumida.")
        return self.raiz

    # E → A
    def E(self, escopo):
        self.A(escopo)

    # A → { E } E | ε
    def A(self, escopo):
        if self.lookahead() == '{':
            self.match('{')
            novo = Escopo(escopo)
            self.E(novo)
            self.match('}')
            self.E(escopo)
        elif self.lookahead() in ['}', '$']:
            return  # Produção vazia
        else:
            raise SyntaxError(f"Token inesperado: {self.lookahead()}")

# Execução
if __name__ == "__main__":
    for palavra in ["{}{{}}", "{{{}}}"]:
        print(f"\nEntrada: {palavra}")
        try:
            parser = Parser(palavra)
            raiz = parser.analisar()
            raiz.imprimir()
        except SyntaxError as e:
            print("Erro:", e)
