# moviedb

Trabalho final da disciplina de classificação e pesquisa de dados.

# Integrante(s)

- Bruno Corrêa Zimmermann, Turma B

# Descompactação de Arquivos

Favor, descompactar arquivos na pasta "data/".

# Execução

Para compilar e executar o programa, basta executar em um shell Unix:
```
$ sh run.sh
```

Se já tiver compilado (veja abaixo) o programa, para executar diretamente,
em um shell Unix basta executar:
```
$ ./build/release/moviedb
```

# Compilação

Para somente compilar, num shell Unix, basta executar:
```
$ sh build.sh release
```

Ou invocar o comando `make` manualmente:
```
$ make
```

# Estrutura Do Projeto

No diretório `src/` estão códigos fonte (`.c`) e cabeçalhos (`.h`).

No diretório `src/test/` estão códigos fonte de programas testes.

No diretório `build/` estão artefatos de compilação, como arquivos objeto (`.o`)
e programas executáveis. Este diretório é gerado automaticamente pelo programa
`make`.

O shell script `build.sh` é responsável por automatizar a compilação.

O shell script `run.sh` é responsável por automatizar o processo de
compilar + executar.

O shell script `watch.sh` observa mudanças nos códigos fonte e executa um
comando (por padrão é `sh watch.sh sh build.sh release`) a cada mudança.
NÃO é NECESSÁRIO usar este script, é apenas uma facilidade de desenvolvimento.

Por fim, o arquivo `Makefile` especifica as regras de _build_, usado pelo
programa `make` para compilar o projeto.

# NOTA

O projeto não automatiza a compilação usando ferramentas nativas do Windows.
A automatização acontece usando ferramentas nativas do Linux e do MacOS. Para
construir o programa no Windows, é necessário usar um ambiente que emule o
ambiente Linux/MacOS, tais como MinGw ou WSL. Outra alternativa seria compilar
cada arquivo manualmente.
