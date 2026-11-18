/**
 * main.c
 * Space Run: desvie dos obstáculos e sobreviva o máximo de tempo possível.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "jogo.h"
#include "keyboard.h"
#include "timer.h"

#define TICK_MS 30
#define TERMINAL_MIN_COLUNAS 80
#define TERMINAL_MIN_LINHAS 24

/* O desenho fica embaralhado em um terminal menor que a área usada pelo jogo. */
static int terminalPequeno(void)
{
    struct winsize tamanho;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &tamanho) != 0) return 0;
    return tamanho.ws_col < TERMINAL_MIN_COLUNAS || tamanho.ws_row < TERMINAL_MIN_LINHAS;
}

int main(void)
{
    if (terminalPequeno()) {
        fprintf(stderr, "Aumente o terminal para pelo menos %dx%d e execute de novo.\n",
                TERMINAL_MIN_COLUNAS, TERMINAL_MIN_LINHAS);
        return 1;
    }

    srand((unsigned int)time(NULL));

    /* Junta cada frame em uma única escrita, enviada por screenUpdate(). */
    setvbuf(stdout, NULL, _IOFBF, 1 << 16);

    Recorde ranking[TOP_RANKING];
    int total = rankingCarregar(ranking, ARQUIVO_RANKING);

    screenInit(0);
    keyboardInit();
    timerInit(TICK_MS);

    while (telaMenu(ranking, total)) {
        int recorde = 0;
        if (total > 0) {
            recorde = ranking[0].pontos;
        }
        int pontos;
        if (jogoExecutar(recorde, &pontos)) {
            telaFimDeJogo(pontos, ranking, &total);
        }
    }

    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}
