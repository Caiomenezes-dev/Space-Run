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
#define PASSO_NAVE 2 /* precisa ser menor que LARGURA_NAVE para a nave não "pular" um obstáculo */
#define LINHA_NAVE (LINHAS - 1)

/* Ranking */
#define ARQUIVO_RANKING "ranking.txt"
#define TOP_RANKING 10
#define TAM_NOME 13 /* 12 caracteres + '\0' */

/* Teclas especiais devolvidas por teclaLer() */
#define TECLA_NENHUMA 0
#define TECLA_CTRL_C 3
#define TECLA_ESQUERDA 1000
#define TECLA_DIREITA 1001

typedef struct {
    int x;      /* coluna da ponta esquerda da nave */
    int pontos; /* segundos sobrevividos */
    int vidas;
} Jogador;

/* Nó da lista encadeada de obstáculos ativos */
typedef struct Obstaculo {
    int x;
    int y;
    char simbolo;
    screenColor cor;
    struct Obstaculo *proximo;
} Obstaculo;

/* Uma posição da matriz do campo */
typedef struct {
    char simbolo;
    screenColor cor;
} Celula;

typedef struct {
    char nome[TAM_NOME];
    int pontos;
} Recorde;

/* obstaculos.c: lista encadeada */
Obstaculo *obstaculoCriar(void);
void obstaculoInserir(Obstaculo **lista, Obstaculo *novo);
void obstaculosMover(Obstaculo **lista);
int obstaculosColidiu(Obstaculo **lista, const Jogador *jogador);
void obstaculosLiberar(Obstaculo **lista);

/* campo.c: matriz do campo e desenho na tela */
void campoDesenharMoldura(void);
void campoInvalidar(void);
void campoLimpar(void);
void campoColocarObstaculos(const Obstaculo *lista);
void campoColocarNave(const Jogador *jogador);
void campoRenderizar(void);

/* ranking.c: leitura e escrita do arquivo de recordes */
int rankingCarregar(Recorde *ranking, const char *arquivo);
int rankingSalvar(const Recorde *ranking, int total, const char *arquivo);
int rankingQualifica(const Recorde *ranking, int total, int pontos);
void rankingInserir(Recorde *ranking, int *total, const char *nome, int pontos);

/* jogo.c: uma partida */
int teclaLer(void);
int jogoExecutar(int recorde, int *pontosFinais);

/* telas.c: menu e fim de jogo */
void telaEscrever(int x, int y, screenColor cor, const char *texto);
int telaMenu(const Recorde *ranking, int total);
void telaFimDeJogo(int pontos, Recorde *ranking, int *total);

#endif /* JOGO_H */
