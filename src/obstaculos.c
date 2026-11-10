/**
 * obstaculos.c
 * Lista encadeada dos obstáculos ativos. Cada obstáculo é criado com malloc
 * ao surgir no topo e liberado com free ao sair da tela ou atingir a nave.
 */

#include <stdlib.h>

#include "jogo.h"

typedef struct {
    char simbolo;
    screenColor cor;
} TipoObstaculo;

static const TipoObstaculo TIPOS[] = {
    {'O', BROWN},     /* meteoro */
    {'@', LIGHTRED},  /* meteoro em chamas */
    {'#', LIGHTGRAY}, /* destroço */
    {'*', LIGHTCYAN}, /* cometa */
};
#define TOTAL_TIPOS ((int)(sizeof(TIPOS) / sizeof(TIPOS[0])))

/* Cria um obstáculo em coluna aleatória no topo. Devolve NULL se faltar memória. */
Obstaculo *obstaculoCriar(void)
{
    Obstaculo *novo = malloc(sizeof(Obstaculo));
    if (novo == NULL) return NULL;

    const TipoObstaculo *tipo = &TIPOS[rand() % TOTAL_TIPOS];
    novo->x = rand() % COLUNAS;
    novo->y = 0;
    novo->simbolo = tipo->simbolo;
    novo->cor = tipo->cor;
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

/* Se algum obstáculo tocou a nave, remove esse obstáculo e devolve 1. */
int obstaculosColidiu(Obstaculo **lista, const Jogador *jogador)
{
    Obstaculo **atual = lista;
    while (*atual != NULL) {
        const Obstaculo *o = *atual;
        if (o->y == LINHA_NAVE && o->x >= jogador->x && o->x < jogador->x + LARGURA_NAVE) {
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
