/**
 * obstaculos.c
 * Lista encadeada dos obstáculos ativos. Cada obstáculo é criado com malloc
 * ao surgir no topo e liberado com free ao sair da tela ou atingir a nave.
 */

#include <stdlib.h>

#include "jogo.h"

/* Cria um objeto espacial sorteado, em coluna aleatória, logo acima do campo.
   Devolve NULL se faltar memória. */
Obstaculo *obstaculoCriar(void)
{
    Obstaculo *novo = malloc(sizeof(Obstaculo));
    if (novo == NULL) return NULL;

    novo->sprite = spriteObstaculoAleatorio();
    novo->x = rand() % (COLUNAS - novo->sprite->largura + 1);
    novo->y = 1 - novo->sprite->altura; /* só a linha de baixo aparece no primeiro frame */
    novo->proximo = NULL;
    return novo;
}

void obstaculoInserir(Obstaculo **lista, Obstaculo *novo)
{
    novo->proximo = *lista;
    *lista = novo;
}

/*
 * As funções abaixo percorrem a lista com um ponteiro para ponteiro: "atual"
 * aponta para o campo que guarda o nó (a cabeça ou o "proximo" do anterior).
 * Assim, remover o primeiro nó ou um do meio é exatamente o mesmo código.
 */
static void removerNo(Obstaculo **atual)
{
    Obstaculo *removido = *atual;
    *atual = removido->proximo;
    free(removido);
}

/* Desce todos os obstáculos uma linha e remove os que saíram do campo. */
void obstaculosMover(Obstaculo **lista)
{
    Obstaculo **atual = lista;
    while (*atual != NULL) {
        (*atual)->y++;
        if ((*atual)->y >= LINHAS) {
            removerNo(atual);
        } else {
            atual = &(*atual)->proximo;
        }
    }
}

/* Há colisão quando alguma célula preenchida de um sprite ocupa a mesma
   posição do campo que uma célula preenchida do outro. */
static int spritesSeTocam(const Sprite *a, int ax, int ay, const Sprite *b, int bx, int by)
{
    for (int l = 0; l < a->altura; l++) {
        for (int c = 0; c < a->largura; c++) {
            if (a->estilo[l][c] == ' ') continue;

            int lb = ay + l - by;
            int cb = ax + c - bx;
            if (lb >= 0 && lb < b->altura && cb >= 0 && cb < b->largura &&
                b->estilo[lb][cb] != ' ') {
                return 1;
            }
        }
    }
    return 0;
}

/* Se algum obstáculo tocou a nave, remove esse obstáculo e devolve 1. */
int obstaculosColidiu(Obstaculo **lista, const Jogador *jogador)
{
    Obstaculo **atual = lista;
    while (*atual != NULL) {
        const Obstaculo *o = *atual;
        if (spritesSeTocam(o->sprite, o->x, o->y, &SPRITE_NAVE, jogador->x, LINHA_NAVE)) {
            removerNo(atual);
            return 1;
        }
        atual = &(*atual)->proximo;
    }
    return 0;
}

void obstaculosLiberar(Obstaculo **lista)
{
    while (*lista != NULL) {
        removerNo(lista);
    }
}
