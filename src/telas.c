/**
 * telas.c
 * Telas fora da partida: menu inicial e fim de jogo.
 */

#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "jogo.h"

#define NOME_PADRAO "Anonimo"
#define ESPERA_FIM_DE_JOGO_MS 1000
#define INTERVALO_LEITURA_US 10000

static const char *const TITULO[] = {
    " ___ ___  _   ___ ___   ___ _   _ _  _ ",
    "/ __| _ \\/_\\ / __| __| | _ \\ | | | \\| |",
    "\\__ \\  _/ _ \\ (__| _|  |   / |_| | .` |",
    "|___/_|/_/ \\_\\___|___| |_|_\\\\___/|_|\\_|",
};
#define TITULO_X 20

void telaEscrever(int x, int y, screenColor cor, const char *texto)
{
    screenSetColor(cor, BLACK);
    screenGotoxy(x, y);
    printf("%s", texto);
}

static void limparTela(void)
{
    screenSetColor(LIGHTGRAY, BLACK);
    screenClear();
}

static void desenharRanking(const Recorde *ranking, int total, int x, int y)
{
    telaEscrever(x, y, YELLOW, "MELHORES TEMPOS");

    if (total == 0) {
        telaEscrever(x, y + 2, LIGHTGRAY, "Ainda não há recordes.");
        return;
    }

    screenSetColor(WHITE, BLACK);
    for (int i = 0; i < total; i++) {
        screenGotoxy(x, y + 2 + i);
        printf("%2d. %-12s %5d s", i + 1, ranking[i].nome, ranking[i].pontos);
    }
}

/* Devolve 1 para iniciar uma partida e 0 para sair do programa. */
int telaMenu(const Recorde *ranking, int total)
{
    limparTela();
    for (int i = 0; i < 4; i++) {
        telaEscrever(TITULO_X, 2 + i, LIGHTCYAN, TITULO[i]);
    }

    desenharRanking(ranking, total, 26, 7);
    telaEscrever(25, 21, WHITE, "ENTER  : iniciar partida");
    telaEscrever(25, 22, WHITE, "Q      : sair");
    screenUpdate();

    for (;;) {
        int tecla = teclaLer();
        if (tecla == '\n' || tecla == '\r') return 1;
        if (tecla == 'q' || tecla == 'Q' || tecla == TECLA_CTRL_C) return 0;
        usleep(INTERVALO_LEITURA_US);
    }
}

static int caractereDeNome(int tecla)
{
    return tecla > 0 && tecla < 128 && (isalnum(tecla) || tecla == '_' || tecla == '-');
}

/* Só aceita caracteres sem espaço, pois o arquivo de ranking separa os campos por espaço. */
static void lerNome(char *nome, int x, int y)
{
    int tamanho = 0;
    nome[0] = '\0';

    for (;;) {
        screenSetColor(LIGHTGREEN, BLACK);
        screenGotoxy(x, y);
        printf("%s_ ", nome);
        screenUpdate();

        int tecla;
        while ((tecla = teclaLer()) == TECLA_NENHUMA) {
            usleep(INTERVALO_LEITURA_US);
        }

        if (tecla == '\n' || tecla == '\r' || tecla == TECLA_CTRL_C) break;

        if ((tecla == 127 || tecla == 8) && tamanho > 0) {
            nome[--tamanho] = '\0';
        } else if (tamanho < TAM_NOME - 1 && caractereDeNome(tecla)) {
            nome[tamanho++] = (char)tecla;
            nome[tamanho] = '\0';
        }
    }

    if (tamanho == 0) strcpy(nome, NOME_PADRAO);
}

/* Descarta o que foi teclado durante a espera, para que as teclas de movimento
   ainda sendo apertadas pelo jogador não apareçam no nome do recorde. */
static void esperarEDescartarTeclas(void)
{
    usleep(ESPERA_FIM_DE_JOGO_MS * 1000);
    while (teclaLer() != TECLA_NENHUMA) {
    }
}

void telaFimDeJogo(int pontos, Recorde *ranking, int *total)
{
    limparTela();
    telaEscrever(33, 3, LIGHTRED, "FIM DE JOGO");
    screenSetColor(WHITE, BLACK);
    screenGotoxy(28, 5);
    printf("Você sobreviveu %d s", pontos);
    screenUpdate();

    esperarEDescartarTeclas();

    int salvou = 1;
    if (rankingQualifica(ranking, *total, pontos)) {
        char nome[TAM_NOME];
        telaEscrever(20, 7, YELLOW, "NOVO RECORDE! Digite seu nome: ");
        lerNome(nome, 51, 7);
        rankingInserir(ranking, total, nome, pontos);
        salvou = rankingSalvar(ranking, *total, ARQUIVO_RANKING);
    }

    desenharRanking(ranking, *total, 26, 9);
    if (!salvou) {
        telaEscrever(20, 21, LIGHTRED, "Não foi possível salvar o ranking.");
    }
    telaEscrever(25, 22, WHITE, "ENTER: voltar ao menu");
    screenUpdate();

    for (;;) {
        int tecla = teclaLer();
        if (tecla == '\n' || tecla == '\r' || tecla == TECLA_CTRL_C) return;
        usleep(INTERVALO_LEITURA_US);
    }
}
