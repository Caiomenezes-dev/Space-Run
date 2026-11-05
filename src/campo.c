/**
 * campo.c
 * Matriz do campo de jogo. A cada frame ela é remontada a partir das posições
 * dos elementos e comparada com o frame anterior: só as células que mudaram
 * são enviadas ao terminal, o que evita que a tela pisque.
 */

#include "jogo.h"

static Celula campo[LINHAS][COLUNAS];
static Celula anterior[LINHAS][COLUNAS];

static void imprimirCaixa(int x, int y, int caractere)
{
    screenGotoxy(x, y);
    printf("%c", caractere);
}

void campoDesenharMoldura(void)
{
    int esquerda = ORIGEM_X - 1, direita = ORIGEM_X + COLUNAS;
    int topo = ORIGEM_Y - 1, base = ORIGEM_Y + LINHAS;

    screenSetColor(LIGHTBLUE, BLACK);
    screenBoxEnable();
    for (int x = esquerda + 1; x < direita; x++) {
        imprimirCaixa(x, topo, BOX_HLINE);
        imprimirCaixa(x, base, BOX_HLINE);
    }
    for (int y = topo + 1; y < base; y++) {
        imprimirCaixa(esquerda, y, BOX_VLINE);
        imprimirCaixa(direita, y, BOX_VLINE);
    }
    imprimirCaixa(esquerda, topo, BOX_UPLEFT);
    imprimirCaixa(direita, topo, BOX_UPRIGHT);
    imprimirCaixa(esquerda, base, BOX_DWNLEFT);
    imprimirCaixa(direita, base, BOX_DWNRIGHT);
    screenBoxDisable();
}

/* Faz o próximo campoRenderizar redesenhar todas as células. */
void campoInvalidar(void)
{
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            anterior[l][c].simbolo = '\0';
        }
    }
}

void campoLimpar(void)
{
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            campo[l][c].simbolo = ' ';
            campo[l][c].cor = BLACK;
        }
    }
}

void campoColocarObstaculos(const Obstaculo *lista)
{
    for (const Obstaculo *o = lista; o != NULL; o = o->proximo) {
        campo[o->y][o->x].simbolo = o->simbolo;
        campo[o->y][o->x].cor = o->cor;
    }
}

void campoColocarNave(const Jogador *jogador)
{
    static const char DESENHO[LARGURA_NAVE] = {'/', '^', '\\'};

    for (int i = 0; i < LARGURA_NAVE; i++) {
        campo[LINHA_NAVE][jogador->x + i].simbolo = DESENHO[i];
        campo[LINHA_NAVE][jogador->x + i].cor = LIGHTGREEN;
    }
}

void campoRenderizar(void)
{
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            if (campo[l][c].simbolo == anterior[l][c].simbolo &&
                campo[l][c].cor == anterior[l][c].cor) {
                continue;
            }
            screenSetColor(campo[l][c].cor, BLACK);
            screenGotoxy(ORIGEM_X + c, ORIGEM_Y + l);
            printf("%c", campo[l][c].simbolo);
            anterior[l][c] = campo[l][c];
        }
    }
}
