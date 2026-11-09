/**
 * sprites.c
 * Desenhos do jogo. As peças usam os caracteres de desenho da cli-lib
 * (BOX_BLOCK, BOX_DIAMOND, BOX_DOT, BOX_HLINE), que o terminal só mostra
 * entre screenBoxEnable() e screenBoxDisable().
 */

#include <stdlib.h>

#include "jogo.h"

/* Linhas que usam caracteres BOX_* precisam ser montadas como vetor de char */
static const char NAVE_BASE[] = {'/', BOX_BLOCK, '\\', '\0'};
static const char ASTEROIDE_PEQUENO[] = {BOX_DIAMOND, '\0'};
static const char ASTEROIDE_GIGANTE_TOPO[] = {'/', BOX_BLOCK, BOX_BLOCK, '\\', '\0'};
static const char ASTEROIDE_GIGANTE_BASE[] = {'\\', BOX_BLOCK, BOX_BLOCK, '/', '\0'};
static const char SATELITE[] = {BOX_BLOCK, BOX_HLINE, BOX_DIAMOND, BOX_HLINE, BOX_BLOCK, '\0'};
static const char CAUDA_COMETA[] = {BOX_DOT, '\0'};

const Sprite SPRITE_NAVE = {
    "Nave", 2, 3,
    {" ^ ", NAVE_BASE},
    {" T ", "tct"},
    LIGHTGREEN, WHITE,
};

const Sprite SPRITES_OBSTACULO[TOTAL_TIPOS_OBSTACULO] = {
    {"Asteroide", 1, 1,
     {ASTEROIDE_PEQUENO}, {"c"},
     BROWN, BROWN},
    {"Asteroide médio", 1, 3,
     {"(@)"}, {"tTt"},
     LIGHTGRAY, BROWN},
    {"Asteroide gigante", 2, 4,
     {ASTEROIDE_GIGANTE_TOPO, ASTEROIDE_GIGANTE_BASE}, {"tCCt", "tCCt"},
     LIGHTGRAY, BROWN},
    {"Satélite", 1, 5,
     {SATELITE}, {"CcccC"},
     WHITE, LIGHTBLUE},
    {"Cometa", 2, 1,
     {CAUDA_COMETA, "*"}, {"c", "T"},
     CYAN, YELLOW},
};

/* Chance de cada tipo surgir, na mesma ordem de SPRITES_OBSTACULO */
static const int PESOS_OBSTACULO[TOTAL_TIPOS_OBSTACULO] = {30, 25, 15, 15, 15};

const Sprite SPRITES_EXPLOSAO[TOTAL_QUADROS_EXPLOSAO] = {
    {"Explosão", 2, 5,
     {"  *  ", " *#* "}, {"  T  ", " tTt "},
     LIGHTRED, YELLOW},
    {"Explosão", 2, 5,
     {"\\ | /", "- * -"}, {"t t t", "t T t"},
     LIGHTRED, YELLOW},
    {"Explosão", 2, 5,
     {".   .", " . . "}, {"t   t", " t t "},
     DARKGRAY, DARKGRAY},
};

const Sprite *spriteObstaculoAleatorio(void)
{
    int soma = 0;
    for (int i = 0; i < TOTAL_TIPOS_OBSTACULO; i++) soma += PESOS_OBSTACULO[i];

    int sorteio = rand() % soma;
    for (int i = 0; i < TOTAL_TIPOS_OBSTACULO; i++) {
        if (sorteio < PESOS_OBSTACULO[i]) return &SPRITES_OBSTACULO[i];
        sorteio -= PESOS_OBSTACULO[i];
    }
    return &SPRITES_OBSTACULO[0];
}

/* Preenche *saida com a célula (linha, coluna) do sprite. Devolve 0 se ela for vazia. */
int spriteCelula(const Sprite *sprite, int linha, int coluna, Celula *saida)
{
    char estilo = sprite->estilo[linha][coluna];
    if (estilo == ' ') return 0;

    saida->simbolo = sprite->desenho[linha][coluna];
    saida->caixa = (estilo == 'c' || estilo == 'C');
    saida->cor = (estilo == 'T' || estilo == 'C') ? sprite->corDestaque : sprite->cor;
    return 1;
}

void celulaImprimir(int x, int y, const Celula *celula)
{
    screenSetColor(celula->cor, BLACK);
    screenGotoxy(x, y);
    if (celula->caixa) {
        screenBoxEnable();
        printf("%c", celula->simbolo);
        screenBoxDisable();
    } else {
        printf("%c", celula->simbolo);
    }
}

/* Imprime o sprite direto na tela, fora do campo (menu e painel). */
void spriteImprimir(const Sprite *sprite, int x, int y)
{
    Celula celula;
    for (int l = 0; l < sprite->altura; l++) {
        for (int c = 0; c < sprite->largura; c++) {
            if (spriteCelula(sprite, l, c, &celula)) celulaImprimir(x + c, y + l, &celula);
        }
    }
}
