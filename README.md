# Space Run

Jogo em linha de comando, feito em C com a [cli-lib](https://github.com/tgfb/cli-lib/), para a disciplina de Programação Imperativa e Funcional (CESAR School, 2026.2).

Você controla uma nave na parte de baixo da tela e desvia dos objetos espaciais que caem do topo: asteroides de três tamanhos, satélites e cometas. São 3 vidas, e a pontuação é o tempo sobrevivido. A cada 10 segundos, os objetos caem mais rápido e surgem com mais frequência.

O visual usa os recursos da cli-lib: caracteres de desenho (`BOX_BLOCK`, `BOX_DIAMOND`, `BOX_DOT`, `BOX_HLINE`, junções `BOX_T*`) nos objetos e molduras, cores, negrito, texto piscando e texto invertido. O fundo tem um campo de estrelas em duas velocidades, e cada colisão mostra uma explosão animada.

**Vídeo do jogo:** _adicionar o link aqui_

**Equipe:**
- Caio Luis De Oliveira Menezes
- João Victor Moraes Pimentel
- Fernando Antonio Fernandes Sotero

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
| `src/obstaculos.c` | lista encadeada dos obstáculos e colisão entre desenhos |
| `src/sprites.c` | desenhos da nave, dos objetos espaciais e da explosão |
| `src/campo.c` | matriz do campo e desenho na tela |
| `src/ranking.c` | leitura e escrita do arquivo `ranking.txt` |
| `src/telas.c` | menu e tela de fim de jogo |
| `src/screen.c`, `src/keyboard.c`, `src/timer.c` | cli-lib, sem alterações |

## Onde cada requisito é atendido

| Requisito | Onde |
| --- | --- |
| Structs | `Jogador`, `Obstaculo`, `Sprite`, `Celula` e `Recorde` em `include/jogo.h`; `Partida` e `Estrela` em `src/jogo.c` |
| Ponteiros | percurso e remoção na lista com `Obstaculo **` em `src/obstaculos.c` |
| Alocação dinâmica | `malloc` em `obstaculoCriar` e `free` em `removerNo` (`src/obstaculos.c`) |
| Lista encadeada | obstáculos ativos, percorridos a cada queda e a cada verificação de colisão |
| Matriz | `campo[LINHAS][COLUNAS]`, remontada a cada frame em `src/campo.c`; cada objeto é um desenho de até 2x5 células (`src/sprites.c`) |
| Arquivo | `rankingCarregar` e `rankingSalvar` em `src/ranking.c` |
| Colisão | `obstaculosColidiu` em `src/obstaculos.c`, que compara célula a célula o desenho do objeto com o da nave |
