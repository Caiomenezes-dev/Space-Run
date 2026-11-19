/**
 * campo.c
 */

#include "jogo.h"

static Celula campo[LINHAS][COLUNAS];
static Celula anterior[LINHAS][COLUNAS];

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
            campo[l][c].caixa = 0;
            campo[l][c].cor = BLACK;
        }
    }
}

void campoColocarCelula(int x, int y, char simbolo, int caixa, screenColor cor)
{
    if (x < 0 || x >= COLUNAS || y < 0 || y >= LINHAS) return;

    campo[y][x].simbolo = simbolo;
    campo[y][x].caixa = (char)caixa;
    campo[y][x].cor = cor;
}

void campoColocarSprite(const Sprite *sprite, int x, int y, int corForcada)
{
    Celula celula;
    for (int l = 0; l < sprite->altura; l++) {
        for (int c = 0; c < sprite->largura; c++) {
            if (!spriteCelula(sprite, l, c, &celula)) continue;
            if (corForcada >= 0) celula.cor = (screenColor)corForcada;
            campoColocarCelula(x + c, y + l, celula.simbolo, celula.caixa, celula.cor);
        }
    }
}

void campoRenderizar(void)
{
    for (int l = 0; l < LINHAS; l++) {
        for (int c = 0; c < COLUNAS; c++) {
            const Celula *atual = &campo[l][c];
            const Celula *antes = &anterior[l][c];
            if (atual->simbolo == antes->simbolo && atual->caixa == antes->caixa &&
                atual->cor == antes->cor) {
                continue;
            }
            celulaImprimir(ORIGEM_X + c, ORIGEM_Y + l, atual);
            anterior[l][c] = *atual;
        }
    }
}
