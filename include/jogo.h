/**
 * jogo.h
 * Tipos, constantes e funções compartilhadas do Space Run.
 */

#ifndef JOGO_H
#define JOGO_H

#include "screen.h"

/* Campo de jogo (área interna da moldura) */
#define LINHAS 20
#define COLUNAS 40

/* Posição na tela da célula [0][0] do campo. A moldura fica em volta dela. */
#define ORIGEM_X 3
#define ORIGEM_Y 2

/* Jogador */
#define VIDAS_INICIAIS 3
#define LARGURA_NAVE 3
#define ALTURA_NAVE 2
#define PASSO_NAVE 2 /* precisa ser menor que LARGURA_NAVE para a nave não "pular" um obstáculo */
#define LINHA_NAVE (LINHAS - ALTURA_NAVE) /* linha de cima da nave */

/* Ranking */
#define ARQUIVO_RANKING "ranking.txt"
#define TOP_RANKING 10
#define TAM_NOME 13 /* 12 caracteres + '\0' */

/* Teclas especiais devolvidas por teclaLer() */
#define TECLA_NENHUMA 0
#define TECLA_CTRL_C 3
#define TECLA_ESQUERDA 1000
#define TECLA_DIREITA 1001

/* Uma posição da matriz do campo */
typedef struct {
    char simbolo;
    char caixa; /* 1 se simbolo deve ser impresso como caractere de desenho (BOX_*) da cli-lib */
    screenColor cor;
} Celula;

#define SPRITE_MAX_ALTURA 2
#define SPRITE_MAX_LARGURA 5

/*
 * Desenho de até 2x5 células. Para cada célula, "estilo" diz como imprimir:
 *   ' ' vazio (transparente e sem colisão)
 *   't' texto na cor principal      'T' texto na cor de destaque
 *   'c' caixa na cor principal      'C' caixa na cor de destaque
 */
typedef struct {
    const char *nome;
    int altura;
    int largura;
    const char *desenho[SPRITE_MAX_ALTURA];
    const char *estilo[SPRITE_MAX_ALTURA];
    screenColor cor;
    screenColor corDestaque;
} Sprite;

typedef struct {
    int x;      /* coluna da ponta esquerda da nave */
    int pontos; /* segundos sobrevividos */
    int vidas;
} Jogador;

/* Nó da lista encadeada de obstáculos ativos */
typedef struct Obstaculo {
    int x;
    int y; /* linha de cima do desenho; negativa enquanto ainda está entrando no campo */
    const Sprite *sprite;
    struct Obstaculo *proximo;
} Obstaculo;

typedef struct {
    char nome[TAM_NOME];
    int pontos;
} Recorde;

/* sprites.c: desenhos da nave, dos objetos espaciais e da explosão */
#define TOTAL_TIPOS_OBSTACULO 5
#define TOTAL_QUADROS_EXPLOSAO 3
extern const Sprite SPRITE_NAVE;
extern const Sprite SPRITES_OBSTACULO[TOTAL_TIPOS_OBSTACULO];
extern const Sprite SPRITES_EXPLOSAO[TOTAL_QUADROS_EXPLOSAO];
const Sprite *spriteObstaculoAleatorio(void);
int spriteCelula(const Sprite *sprite, int linha, int coluna, Celula *saida);
void spriteImprimir(const Sprite *sprite, int x, int y);
void celulaImprimir(int x, int y, const Celula *celula);

/* obstaculos.c: lista encadeada */
Obstaculo *obstaculoCriar(void);
void obstaculoInserir(Obstaculo **lista, Obstaculo *novo);
void obstaculosMover(Obstaculo **lista);
int obstaculosColidiu(Obstaculo **lista, const Jogador *jogador);
void obstaculosLiberar(Obstaculo **lista);

/* campo.c: matriz do campo e desenho na tela */
void campoInvalidar(void);
void campoLimpar(void);
void campoColocarCelula(int x, int y, char simbolo, int caixa, screenColor cor);
void campoColocarSprite(const Sprite *sprite, int x, int y, int corForcada);
void campoRenderizar(void);

/* ranking.c: leitura e escrita do arquivo de recordes */
int rankingCarregar(Recorde *ranking, const char *arquivo);
int rankingSalvar(const Recorde *ranking, int total, const char *arquivo);
int rankingQualifica(const Recorde *ranking, int total, int pontos);
int rankingInserir(Recorde *ranking, int *total, const char *nome, int pontos);

/* jogo.c: uma partida */
int teclaLer(void);
int jogoExecutar(int recorde, int *pontosFinais);

/* telas.c: menu, fim de jogo e elementos de tela reutilizados */
void telaEscrever(int x, int y, screenColor cor, const char *texto);
void telaCaixa(int esquerda, int topo, int direita, int base, screenColor cor);
void telaSeparador(int esquerda, int direita, int y, screenColor cor);
void telaLegenda(int x, int y);
int telaMenu(const Recorde *ranking, int total);
void telaFimDeJogo(int pontos, Recorde *ranking, int *total);

#endif /* JOGO_H */
