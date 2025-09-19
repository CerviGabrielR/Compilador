def ler_codigo_fonte(caminho_do_arquivo: str) -> str | None:
    
    try:
        with open(caminho_do_arquivo, 'r', encoding='utf-8') as arquivo:
            return arquivo.read()
    except FileNotFoundError:
        print(f"ERRO: O arquivo não encontrado.")
        return None

class AnalisadorLexico:
    
    def analisar(self, codigo_fonte: str) -> list[str]:
        tokens = []
        posicao = 0
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
                
                tokens.append("IDENT")
                
                
                posicao = posicao_seguinte

            elif char_atual.isspace():

                posicao += 1
            
            else:

                tokens.append("OUTRO")
                posicao += 1
        
        return tokens


if __name__ == "__main__":
    


    nome_do_arquivo = "cogidoFonteAnalisadorLexico.txt"
    
    # Para pegar o código fonte da aula passada
    #source_code = ler_codigo_fonte(nome_do_arquivo)


    # hardcoding
    source_code = """
            def f (int a) {
            x = x + a;
            return;
            }
    """

    if source_code:
        analisador = AnalisadorLexico()
        lista_de_tokens = analisador.analisar(source_code)
        

        
        #print(" ".join(lista_de_tokens))

        print("-" * 30)
        print("Lista de Tokens gerada:")
        print(lista_de_tokens)