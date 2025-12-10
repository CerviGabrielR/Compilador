FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV IN_DOCKER=1

# Dependências: g++ 11.4.x e make para compilar os testes/binários.
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        build-essential \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

# Comando padrão pode ser sobrescrito (ex.: `docker run --rm <img> make tests`)
CMD ["bash"]
