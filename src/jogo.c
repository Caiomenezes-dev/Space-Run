/**
 * jogo.c
 * Laço de uma partida: lê o teclado, atualiza o estado e desenha o frame.
 */

#include <stdlib.h>
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
#define SURGIMENTO_INICIAL_MS 650
#define SURGIMENTO_PASSO_MS 45
#define SURGIMENTO_MINIMO_MS 120

#define INVULNERAVEL_MS 1000 /* tempo sem levar dano depois de uma colisão */
#define PISCA_MS 100         /* a nave alterna entre vermelho e invisível nesse intervalo */
#define EXPLOSAO_MS 450      /* duração da animação de explosão */

/* Estrelas do fundo: duas camadas que descem mais devagar que os obstáculos,
   a de trás mais lenta que a da frente, dando sensação de profundidade. */
#define TOTAL_ESTRELAS 30
#define CAMADAS_ESTRELAS 2
static const int LENTIDAO_CAMADA[CAMADAS_ESTRELAS] = {4, 2}; /* múltiplo do intervalo de queda */

/* Painel lateral */
#define PAINEL_ESQUERDA 45
#define PAINEL_DIREITA 77
#define PAINEL_X 47
#define BARRA_NIVEL_X 59

typedef struct {
    int x;
    int y;
    int camada;
} Estrela;

typedef struct {
    Jogador jogador;
    Obstaculo *obstaculos;
    Estrela estrelas[TOTAL_ESTRELAS];
    long tempoMs;
    long acumuladoQuedaMs;
    long acumuladoSurgimentoMs;
    long acumuladoEstrelasMs[CAMADAS_ESTRELAS];
    long invulneravelMs;
    long explosaoMs; /* tempo restante da explosão; 0 se não há explosão */
    int explosaoX;
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
    if (intervalo < minimo) {
        return minimo;
    }
    return intervalo;
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
    if (p->invulneravelMs > 0 || p->jogador.vidas == 0) return;

    if (obstaculosColidiu(&p->obstaculos, &p->jogador)) {
        p->jogador.vidas--;
        p->invulneravelMs = INVULNERAVEL_MS;
        p->explosaoMs = EXPLOSAO_MS;
        p->explosaoX = p->jogador.x;
    }
}

static void moverEstrelas(Partida *p, long dtMs, long queda)
{
    for (int camada = 0; camada < CAMADAS_ESTRELAS; camada++) {
        long intervalo = queda * LENTIDAO_CAMADA[camada];
        p->acumuladoEstrelasMs[camada] += dtMs;
        if (p->acumuladoEstrelasMs[camada] < intervalo) continue;
        p->acumuladoEstrelasMs[camada] -= intervalo;

        for (int i = 0; i < TOTAL_ESTRELAS; i++) {
            Estrela *e = &p->estrelas[i];
            if (e->camada != camada) continue;
            if (++e->y >= LINHAS) {
                e->y = 0;
                e->x = rand() % COLUNAS;
            }
        }
    }
}

static long diminuirAteZero(long valor, long dtMs)
{
    if (valor > dtMs) {
        return valor - dtMs;
    }
    return 0;
}

static void atualizar(Partida *p, long dtMs)
{
    p->explosaoMs = diminuirAteZero(p->explosaoMs, dtMs);

    /* Depois da última vida, só a explosão continua até terminar */
    if (p->jogador.vidas == 0) return;

    p->tempoMs += dtMs;
    p->jogador.pontos = (int)(p->tempoMs / 1000);
    p->invulneravelMs = diminuirAteZero(p->invulneravelMs, dtMs);

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

    moverEstrelas(p, dtMs, queda);
}

static void desenharPainelFixo(void)
{
    telaCaixa(PAINEL_ESQUERDA, ORIGEM_Y - 1, PAINEL_DIREITA, ORIGEM_Y + LINHAS, LIGHTBLUE);

    screenSetBold();
    telaEscrever(PAINEL_X + 6, 2, LIGHTCYAN, "S P A C E   R U N");
    screenSetNormal();
    telaSeparador(PAINEL_ESQUERDA, PAINEL_DIREITA, 3, LIGHTBLUE);

    telaSeparador(PAINEL_ESQUERDA, PAINEL_DIREITA, 8, LIGHTBLUE);
    telaEscrever(PAINEL_X, 9, YELLOW, "PERIGOS");
    telaLegenda(PAINEL_X, 10);

    telaSeparador(PAINEL_ESQUERDA, PAINEL_DIREITA, 18, LIGHTBLUE);
    telaEscrever(PAINEL_X, 19, LIGHTGRAY, "<- -> ou A D : mover");
    telaEscrever(PAINEL_X, 20, LIGHTGRAY, "Q            : desistir");
}

/* Barra que enche a cada segundo e esvazia ao subir de nível */
static void desenharBarraNivel(int segundosNoNivel)
{
    screenGotoxy(BARRA_NIVEL_X, 5);
    screenBoxEnable();
    for (int i = 0; i < SEGUNDOS_POR_NIVEL; i++) {
        if (i < segundosNoNivel) {
            screenSetColor(LIGHTCYAN, BLACK);
            printf("%c", BOX_BLOCK);
        } else {
            screenSetColor(DARKGRAY, BLACK);
            printf("%c", BOX_DOT);
        }
    }
    screenBoxDisable();
}

static void desenharPainel(const Partida *p, int recorde)
{
    int pontos = p->jogador.pontos;

    screenSetColor(WHITE, BLACK);
    screenGotoxy(PAINEL_X, 4);
    printf("Tempo: %d s   ", pontos);
    screenGotoxy(PAINEL_X, 5);
    printf("Nível: %-3d", nivelAtual(p));
    desenharBarraNivel(pontos % SEGUNDOS_POR_NIVEL);

    screenGotoxy(PAINEL_X, 6);
    screenSetColor(WHITE, BLACK);
    printf("Vidas: ");
    for (int i = 0; i < VIDAS_INICIAIS; i++) {
        if (i < p->jogador.vidas) {
            screenSetColor(LIGHTGREEN, BLACK);
            printf("/^\\ ");
        } else {
            screenSetColor(DARKGRAY, BLACK);
            printf(" x  ");
        }
    }

    screenSetColor(YELLOW, BLACK);
    screenGotoxy(PAINEL_X, 7);
    int melhor = recorde;
    if (pontos > recorde) {
        melhor = pontos;
    }
    printf("Recorde: %d s   ", melhor);
    screenGotoxy(PAINEL_X + 17, 7);
    if (recorde > 0 && pontos > recorde) {
        screenSetBlink();
        printf("NOVO!");
        screenSetNormal();
    } else {
        printf("     ");
    }
}

static void desenharFrame(const Partida *p, int recorde)
{
    campoLimpar();

    for (int i = 0; i < TOTAL_ESTRELAS; i++) {
        const Estrela *e = &p->estrelas[i];
        if (e->camada == 0) {
            campoColocarCelula(e->x, e->y, '.', 0, DARKGRAY);
        } else {
            campoColocarCelula(e->x, e->y, BOX_DOT, 1, LIGHTGRAY);
        }
    }

    for (const Obstaculo *o = p->obstaculos; o != NULL; o = o->proximo) {
        campoColocarSprite(o->sprite, o->x, o->y, -1);
    }

    /* Depois de uma colisão, a nave pisca em vermelho enquanto estiver invulnerável */
    if (p->jogador.vidas > 0) {
        if (p->invulneravelMs == 0) {
            campoColocarSprite(&SPRITE_NAVE, p->jogador.x, LINHA_NAVE, -1);
        } else if ((p->invulneravelMs / PISCA_MS) % 2 == 0) {
            campoColocarSprite(&SPRITE_NAVE, p->jogador.x, LINHA_NAVE, LIGHTRED);
        }
    }

    if (p->explosaoMs > 0) {
        int quadro = (int)((EXPLOSAO_MS - p->explosaoMs) * TOTAL_QUADROS_EXPLOSAO / EXPLOSAO_MS);
        campoColocarSprite(&SPRITES_EXPLOSAO[quadro], p->explosaoX - 1, LINHA_NAVE, -1);
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
    for (int i = 0; i < TOTAL_ESTRELAS; i++) {
        partida.estrelas[i].x = rand() % COLUNAS;
        partida.estrelas[i].y = rand() % LINHAS;
        partida.estrelas[i].camada = i % CAMADAS_ESTRELAS;
    }

    screenSetColor(LIGHTGRAY, BLACK);
    screenClear();
    telaCaixa(ORIGEM_X - 1, ORIGEM_Y - 1, ORIGEM_X + COLUNAS, ORIGEM_Y + LINHAS, LIGHTBLUE);
    desenharPainelFixo();
    campoInvalidar();
    desenharFrame(&partida, recorde);

    int desistiu = 0;
    long ultimoMs = agoraMs();

    /* Ao perder a última vida, o laço continua até a explosão terminar */
    while (!desistiu && (partida.jogador.vidas > 0 || partida.explosaoMs > 0)) {
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
