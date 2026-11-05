/**
 * jogo.c
 * Laço de uma partida: lê o teclado, atualiza o estado e desenha o frame.
 */

#include <time.h>
#include <unistd.h>

#include "jogo.h"
#include "keyboard.h"
#include "timer.h"

/* Dificuldade: a cada nível a queda e o surgimento ficam mais rápidos */
#define SEGUNDOS_POR_NIVEL 10
#define QUEDA_INICIAL_MS 220
#define QUEDA_PASSO_MS 15
#define QUEDA_MINIMA_MS 60
#define SURGIMENTO_INICIAL_MS 600
#define SURGIMENTO_PASSO_MS 45
#define SURGIMENTO_MINIMO_MS 90

#define INVULNERAVEL_MS 1000 /* tempo sem levar dano depois de uma colisão */
#define PISCA_MS 100         /* a nave alterna visível/invisível nesse intervalo */

#define PAINEL_X 47

typedef struct {
    Jogador jogador;
    Obstaculo *obstaculos;
    long tempoMs;
    long acumuladoQuedaMs;
    long acumuladoSurgimentoMs;
    long invulneravelMs;
} Partida;

/* Lê uma tecla sem bloquear. As setas chegam como 3 bytes: ESC [ C ou ESC O C. */
int teclaLer(void)
{
    if (!keyhit()) return TECLA_NENHUMA;

    int ch = readch();
    if (ch != 27) return ch;

    if (!keyhit()) return TECLA_NENHUMA;
    int prefixo = readch();
    if ((prefixo != '[' && prefixo != 'O') || !keyhit()) return TECLA_NENHUMA;

    switch (readch()) {
        case 'D': return TECLA_ESQUERDA;
        case 'C': return TECLA_DIREITA;
        default:  return TECLA_NENHUMA;
    }
}

static long agoraMs(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000L + t.tv_nsec / 1000000L;
}

static int nivelAtual(const Partida *p)
{
    return 1 + p->jogador.pontos / SEGUNDOS_POR_NIVEL;
}

static long intervaloPorNivel(int inicial, int passo, int minimo, int nivel)
{
    int intervalo = inicial - passo * (nivel - 1);
    return intervalo < minimo ? minimo : intervalo;
}

static void moverNave(Jogador *jogador, int deslocamento)
{
    int x = jogador->x + deslocamento;
    if (x < 0) x = 0;
    if (x > COLUNAS - LARGURA_NAVE) x = COLUNAS - LARGURA_NAVE;
    jogador->x = x;
}

static void verificarColisao(Partida *p)
{
    if (p->invulneravelMs > 0) return;

    if (obstaculosColidiu(&p->obstaculos, &p->jogador)) {
        p->jogador.vidas--;
        p->invulneravelMs = INVULNERAVEL_MS;
    }
}

static void atualizar(Partida *p, long dtMs)
{
    p->tempoMs += dtMs;
    p->jogador.pontos = (int)(p->tempoMs / 1000);

    if (p->invulneravelMs > 0) {
        p->invulneravelMs -= dtMs;
        if (p->invulneravelMs < 0) p->invulneravelMs = 0;
    }

    /* Verifica antes de descer para pegar a nave que acabou de se mover para
       cima de um obstáculo, que poderia sair do campo nesta mesma queda. */
    verificarColisao(p);

    int nivel = nivelAtual(p);

    long queda = intervaloPorNivel(QUEDA_INICIAL_MS, QUEDA_PASSO_MS, QUEDA_MINIMA_MS, nivel);
    p->acumuladoQuedaMs += dtMs;
    if (p->acumuladoQuedaMs >= queda) {
        p->acumuladoQuedaMs -= queda;
        obstaculosMover(&p->obstaculos);
        verificarColisao(p);
    }

    long surgimento = intervaloPorNivel(SURGIMENTO_INICIAL_MS, SURGIMENTO_PASSO_MS,
                                        SURGIMENTO_MINIMO_MS, nivel);
    p->acumuladoSurgimentoMs += dtMs;
    if (p->acumuladoSurgimentoMs >= surgimento) {
        p->acumuladoSurgimentoMs -= surgimento;
        Obstaculo *novo = obstaculoCriar();
        if (novo != NULL) obstaculoInserir(&p->obstaculos, novo);
    }
}

static void desenharPainelFixo(void)
{
    telaEscrever(PAINEL_X, 2, LIGHTCYAN, "S P A C E   R U N");
    telaEscrever(PAINEL_X, 9, LIGHTGRAY, "Recorde:");
    telaEscrever(PAINEL_X, 13, WHITE, "Controles");
    telaEscrever(PAINEL_X, 14, LIGHTGRAY, "<- ou A : esquerda");
    telaEscrever(PAINEL_X, 15, LIGHTGRAY, "-> ou D : direita");
    telaEscrever(PAINEL_X, 16, LIGHTGRAY, "Q       : desistir");
}

static void desenharPainel(const Partida *p, int recorde)
{
    int melhor = p->jogador.pontos > recorde ? p->jogador.pontos : recorde;

    screenSetColor(WHITE, BLACK);
    screenGotoxy(PAINEL_X, 4);
    printf("Tempo: %d s   ", p->jogador.pontos);
    screenGotoxy(PAINEL_X, 5);
    printf("Nível: %d   ", nivelAtual(p));

    screenSetColor(LIGHTGREEN, BLACK);
    screenGotoxy(PAINEL_X, 7);
    printf("Vidas: ");
    for (int i = 0; i < VIDAS_INICIAIS; i++) {
        printf("%s", i < p->jogador.vidas ? "/^\\ " : "    ");
    }

    screenSetColor(YELLOW, BLACK);
    screenGotoxy(PAINEL_X + 9, 9);
    printf("%d s   ", melhor);
}

static void desenharFrame(const Partida *p, int recorde)
{
    campoLimpar();
    campoColocarObstaculos(p->obstaculos);

    /* Depois de uma colisão, a nave pisca enquanto estiver invulnerável */
    if (p->invulneravelMs == 0 || (p->invulneravelMs / PISCA_MS) % 2 == 0) {
        campoColocarNave(&p->jogador);
    }

    campoRenderizar();
    desenharPainel(p, recorde);
    screenUpdate();
}

/*
 * Joga uma partida até acabarem as vidas ou o jogador desistir.
 * Devolve 1 se a partida terminou por morte (com os pontos em *pontosFinais)
 * e 0 se o jogador desistiu.
 */
int jogoExecutar(int recorde, int *pontosFinais)
{
    Partida partida = {0};
    partida.jogador.x = (COLUNAS - LARGURA_NAVE) / 2;
    partida.jogador.vidas = VIDAS_INICIAIS;

    screenSetColor(LIGHTGRAY, BLACK);
    screenClear();
    campoDesenharMoldura();
    desenharPainelFixo();
    campoInvalidar();
    desenharFrame(&partida, recorde);

    int desistiu = 0;
    long ultimoMs = agoraMs();

    while (!desistiu && partida.jogador.vidas > 0) {
        int tecla;
        while ((tecla = teclaLer()) != TECLA_NENHUMA) {
            if (tecla == TECLA_ESQUERDA || tecla == 'a' || tecla == 'A') {
                moverNave(&partida.jogador, -PASSO_NAVE);
            } else if (tecla == TECLA_DIREITA || tecla == 'd' || tecla == 'D') {
                moverNave(&partida.jogador, PASSO_NAVE);
            } else if (tecla == 'q' || tecla == 'Q' || tecla == TECLA_CTRL_C) {
                desistiu = 1;
            }
        }

        if (timerTimeOver()) {
            /* O tempo real decorrido é medido porque o timer da biblioteca
               sempre dispara um pouco depois do intervalo pedido. */
            long agora = agoraMs();
            atualizar(&partida, agora - ultimoMs);
            ultimoMs = agora;
            desenharFrame(&partida, recorde);
        }

        usleep(1000); /* evita ocupar 100% da CPU enquanto espera */
    }

    *pontosFinais = partida.jogador.pontos;
    obstaculosLiberar(&partida.obstaculos);
    return !desistiu;
}
