/**
 * telas.c
 * Telas fora da partida (menu e fim de jogo) e elementos de tela reutilizados.
 */

#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "jogo.h"

#define NOME_PADRAO "Anonimo"
#define ESPERA_FIM_DE_JOGO_MS 1000
#define INTERVALO_LEITURA_US 10000

/* Moldura das telas de menu e fim de jogo */
#define TELA_ESQUERDA 2
#define TELA_DIREITA 77
#define TELA_TOPO 1
#define TELA_BASE 22

static const char *const TITULO[] = {
    " ___ ___  _   ___ ___   ___ _   _ _  _ ",
    "/ __| _ \\/_\\ / __| __| | _ \\ | | | \\| |",
    "\\__ \\  _/ _ \\ (__| _|  |   / |_| | .` |",
    "|___/_|/_/ \\_\\___|___| |_|_\\\\___/|_|\\_|",
};
static const screenColor CORES_TITULO[] = {LIGHTCYAN, LIGHTCYAN, CYAN, LIGHTBLUE};

static const char *const FIM_DE_JOGO[] = {
    " ___   ___   __  __     ___    ___        _    ___     ___    ___",
    "| __| |_ _| |  \\/  |   |   \\  | __|    _ | |  / _ \\   / __|  / _ \\",
    "| _|   | |  | |\\/| |   | |) | | _|    | || | | (_) | | (_ | | (_) |",
    "|_|   |___| |_|  |_|   |___/  |___|    \\__/   \\___/   \\___|  \\___/",
};
#define LINHAS_LETREIRO 4

void telaEscrever(int x, int y, screenColor cor, const char *texto)
{
    screenSetColor(cor, BLACK);
    screenGotoxy(x, y);
    printf("%s", texto);
}

static void imprimirCaixa(int x, int y, int caractere)
{
    screenGotoxy(x, y);
    printf("%c", caractere);
}

void telaCaixa(int esquerda, int topo, int direita, int base, screenColor cor)
{
    screenSetColor(cor, BLACK);
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

/* Linha horizontal que se encaixa nas laterais de uma caixa: ├────┤ */
void telaSeparador(int esquerda, int direita, int y, screenColor cor)
{
    screenSetColor(cor, BLACK);
    screenBoxEnable();
    imprimirCaixa(esquerda, y, BOX_TLEFT);
    for (int x = esquerda + 1; x < direita; x++) imprimirCaixa(x, y, BOX_HLINE);
    imprimirCaixa(direita, y, BOX_TRIGHT);
    screenBoxDisable();
}

/* Divisória vertical entre dois separadores/bordas: ┬ │ ┴ */
static void divisoriaVertical(int x, int topo, int base, screenColor cor)
{
    screenSetColor(cor, BLACK);
    screenBoxEnable();
    imprimirCaixa(x, topo, BOX_TUP);
    for (int y = topo + 1; y < base; y++) imprimirCaixa(x, y, BOX_VLINE);
    imprimirCaixa(x, base, BOX_TDOWN);
    screenBoxDisable();
}

/* Cada objeto espacial desenhado ao lado do seu nome. Ocupa 7 linhas. */
void telaLegenda(int x, int y)
{
    for (int i = 0; i < TOTAL_TIPOS_OBSTACULO; i++) {
        const Sprite *s = &SPRITES_OBSTACULO[i];
        spriteImprimir(s, x, y);
        telaEscrever(x + SPRITE_MAX_LARGURA + 2, y, LIGHTGRAY, s->nome);
        y += s->altura;
    }
}

/* Título com fundo invertido (screenSetReverse da cli-lib) */
static void escreverCabecalho(int x, int y, screenColor cor, const char *texto)
{
    screenSetColor(cor, BLACK);
    screenSetReverse();
    screenGotoxy(x, y);
    printf(" %s ", texto);
    screenSetNormal();
}

static void limparTela(void)
{
    screenSetNormal();
    screenSetColor(LIGHTGRAY, BLACK);
    screenClear();
}

/* Lista o ranking abaixo do cabeçalho. A linha "destaque" (ou -1) aparece em verde. */
static void desenharRanking(const Recorde *ranking, int total, int x, int y, int destaque)
{
    escreverCabecalho(x, y, YELLOW, "MELHORES TEMPOS");

    if (total == 0) {
        telaEscrever(x, y + 2, LIGHTGRAY, "Ainda não há recordes.");
        return;
    }

    for (int i = 0; i < total; i++) {
        screenColor cor = (i == destaque) ? LIGHTGREEN : (i == 0) ? YELLOW : WHITE;
        screenSetColor(cor, BLACK);
        screenGotoxy(x, y + 1 + i);
        printf("%2d. %-12s %5d s", i + 1, ranking[i].nome, ranking[i].pontos);
    }
}

/* Devolve 1 para iniciar uma partida e 0 para sair do programa. */
int telaMenu(const Recorde *ranking, int total)
{
    limparTela();
    telaCaixa(TELA_ESQUERDA, TELA_TOPO, TELA_DIREITA, TELA_BASE, LIGHTBLUE);

    for (int i = 0; i < 4; i++) {
        telaEscrever(20, 2 + i, CORES_TITULO[i], TITULO[i]);
    }
    telaEscrever(11, 7, LIGHTGRAY, "Desvie dos objetos espaciais e sobreviva o máximo que puder!");

    telaSeparador(TELA_ESQUERDA, TELA_DIREITA, 8, LIGHTBLUE);
    divisoriaVertical(38, 8, TELA_BASE, LIGHTBLUE);

    desenharRanking(ranking, total, 7, 10, -1);

    escreverCabecalho(42, 10, LIGHTCYAN, "OBJETOS ESPACIAIS");
    telaLegenda(43, 12);

    screenSetColor(LIGHTGREEN, BLACK);
    screenSetBlink();
    screenGotoxy(43, 20);
    printf("ENTER : jogar");
    screenSetNormal();
    telaEscrever(43, 21, LIGHTGRAY, "Q     : sair");
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
    telaCaixa(TELA_ESQUERDA, TELA_TOPO, TELA_DIREITA, TELA_BASE, LIGHTRED);
    for (int i = 0; i < LINHAS_LETREIRO; i++) {
        telaEscrever(7, 2 + i, i < 2 ? LIGHTRED : RED, FIM_DE_JOGO[i]);
    }

    screenSetColor(WHITE, BLACK);
    screenGotoxy(29, 7);
    printf("Você sobreviveu %d s", pontos);
    screenUpdate();

    esperarEDescartarTeclas();

    int posicao = -1;
    int salvou = 1;
    if (rankingQualifica(ranking, *total, pontos)) {
        char nome[TAM_NOME];
        screenSetColor(YELLOW, BLACK);
        screenSetBlink();
        screenGotoxy(18, 9);
        printf("NOVO RECORDE!");
        screenSetNormal();
        telaEscrever(33, 9, WHITE, "Digite seu nome: ");
        lerNome(nome, 50, 9);
        posicao = rankingInserir(ranking, total, nome, pontos);
        salvou = rankingSalvar(ranking, *total, ARQUIVO_RANKING);
    }

    telaSeparador(TELA_ESQUERDA, TELA_DIREITA, 10, LIGHTRED);
    desenharRanking(ranking, *total, 7, 11, posicao);

    if (!salvou) {
        telaEscrever(46, 19, LIGHTRED, "Não foi possível salvar.");
    }
    telaEscrever(46, 21, WHITE, "ENTER: voltar ao menu");
    screenUpdate();

    for (;;) {
        int tecla = teclaLer();
        if (tecla == '\n' || tecla == '\r' || tecla == TECLA_CTRL_C) return;
        usleep(INTERVALO_LEITURA_US);
    }
}
