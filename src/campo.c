/**
 * campo.c
 * Matriz do campo de jogo. A cada frame ela é remontada a partir das posições
 * dos elementos e comparada com o frame anterior: só as células que mudaram
 * são enviadas ao terminal, o que evita que a tela pisque.
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

/* Copia o sprite para a matriz com o canto superior esquerdo em (x, y).
   As partes fora do campo são cortadas. Se corForcada >= 0, ela substitui
   as cores do sprite (usado para a nave piscar em vermelho). */
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
