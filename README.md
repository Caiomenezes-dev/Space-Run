# Space Run

Jogo em linha de comando, feito em C com a [cli-lib](https://github.com/tgfb/cli-lib/), para a disciplina de Programação Imperativa e Funcional (CESAR School, 2026.2).

Você controla uma nave na parte de baixo da tela e desvia dos obstáculos que caem do topo. São 3 vidas, e a pontuação é o tempo sobrevivido. A cada 10 segundos, os obstáculos caem mais rápido e surgem com mais frequência.

**Vídeo do jogo:** _adicionar o link aqui_

**Equipe:** _adicionar os nomes aqui_

## Como compilar e executar

É preciso ter Linux, macOS ou WSL, com `gcc` e `make`. O terminal deve ter pelo menos 80 colunas por 24 linhas.

```
make
make run
```

Sem o `make`, é possível compilar com:

```
gcc src/*.c -Iinclude -o space-run
./space-run
```

## Controles

| Tecla | Ação |
| --- | --- |
| `←` ou `A` | mover para a esquerda |
| `→` ou `D` | mover para a direita |
| `Enter` | iniciar partida (no menu) |
| `Q` | desistir da partida ou sair (no menu) |

## Organização do código

| Arquivo | Conteúdo |
| --- | --- |
| `src/main.c` | inicialização e fluxo entre menu, partida e fim de jogo |
| `src/jogo.c` | laço da partida, movimento, colisão e dificuldade |
| `src/obstaculos.c` | lista encadeada dos obstáculos |
| `src/campo.c` | matriz do campo e desenho na tela |
| `src/ranking.c` | leitura e escrita do arquivo `ranking.txt` |
| `src/telas.c` | menu e tela de fim de jogo |
| `src/screen.c`, `src/keyboard.c`, `src/timer.c` | cli-lib, sem alterações |

## Onde cada requisito é atendido

| Requisito | Onde |
| --- | --- |
| Structs | `Jogador`, `Obstaculo`, `Celula` e `Recorde` em `include/jogo.h`; `Partida` em `src/jogo.c` |
| Ponteiros | percurso e remoção na lista com `Obstaculo **` em `src/obstaculos.c` |
| Alocação dinâmica | `malloc` em `obstaculoCriar` e `free` em `removerNo` (`src/obstaculos.c`) |
| Lista encadeada | obstáculos ativos, percorridos a cada queda e a cada verificação de colisão |
| Matriz | `campo[LINHAS][COLUNAS]`, remontada a cada frame em `src/campo.c` |
| Arquivo | `rankingCarregar` e `rankingSalvar` em `src/ranking.c` |
| Colisão | `obstaculosColidiu` em `src/obstaculos.c` |
