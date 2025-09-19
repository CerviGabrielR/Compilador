# -*- coding: utf-8 -*-

def ler_codigo_fonte(caminho_do_arquivo: str) -> str | None:
    
    try:
        with open(caminho_do_arquivo, 'r', encoding='utf-8') as arquivo:
            return arquivo.read()
    except FileNotFoundError:
        print(f"ERRO: O arquivo não foi encontrado.")
        return None

class AnalisadorLexico:

    def __init__(self):
        
        self.tabela_de_simbolos = {}
        self.tokens_identificados = []

    def analisar(self, codigo_fonte: str):
        
        self.tabela_de_simbolos.clear()
        self.tokens_identificados.clear()

        posicao = 0
        linha_atual = 1
        tamanho_codigo = len(codigo_fonte)

        while posicao < tamanho_codigo:
            char_atual = codigo_fonte[posicao]


            if char_atual.isalpha():
                lexema = char_atual
                posicao_seguinte = posicao + 1

                while (posicao_seguinte < tamanho_codigo and
                       codigo_fonte[posicao_seguinte].isalnum()):
                    lexema += codigo_fonte[posicao_seguinte]
                    posicao_seguinte += 1
                

                self.tokens_identificados.append("IDENT")
                

                if lexema not in self.tabela_de_simbolos:
                    self.tabela_de_simbolos[lexema] = []
                if linha_atual not in self.tabela_de_simbolos[lexema]:
                    self.tabela_de_simbolos[lexema].append(linha_atual)
                
                posicao = posicao_seguinte


            elif char_atual == '\n':
                linha_atual += 1
                posicao += 1


            elif char_atual.isspace():
                posicao += 1
            

            else:

                self.tokens_identificados.append("OUTRO")
                posicao += 1


if __name__ == "__main__":
    

    source_code = """ def f (int x) {
                if x < 0
                    x = -x;
            return;
            }
    """

    if source_code:

        analisador = AnalisadorLexico()
        

        analisador.analisar(source_code)
        

        lista_de_tokens = analisador.tokens_identificados
        
        print("-" * 30)
        print("Saída Principal (Lista de Tokens gerada):")
        print(lista_de_tokens)


        print("\n" + "-" * 30)
        print("Estrutura da Tabela de Símbolos (salva no objeto):")
        print(analisador.tabela_de_simbolos)