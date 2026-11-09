/**
 * ranking.c
 * Melhores tempos, guardados em um arquivo de texto com uma linha "nome pontos"
 * por recorde. O vetor em memória fica sempre em ordem decrescente de pontos.
 */

#include <stdio.h>
#include <string.h>

#include "jogo.h"

/* Devolve a posição em que o recorde entrou, ou -1 se ele ficou fora do top. */
int rankingInserir(Recorde *ranking, int *total, const char *nome, int pontos)
{
    int pos = 0;
    while (pos < *total && ranking[pos].pontos >= pontos) {
        pos++; /* em caso de empate, quem chegou primeiro fica na frente */
    }
    if (pos >= TOP_RANKING) return -1;

    if (*total < TOP_RANKING) (*total)++;
    for (int i = *total - 1; i > pos; i--) {
        ranking[i] = ranking[i - 1];
    }

    strncpy(ranking[pos].nome, nome, TAM_NOME - 1);
    ranking[pos].nome[TAM_NOME - 1] = '\0';
    ranking[pos].pontos = pontos;
    return pos;
}

/* Devolve quantos recordes foram lidos (0 se o arquivo ainda não existir). */
int rankingCarregar(Recorde *ranking, const char *arquivo)
{
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) return 0;

    int total = 0;
    char nome[TAM_NOME];
    int pontos;
    while (fscanf(f, "%12s %d", nome, &pontos) == 2) {
        rankingInserir(ranking, &total, nome, pontos);
    }

    fclose(f);
    return total;
}

/* Devolve 1 se salvou e 0 se não foi possível escrever no arquivo. */
int rankingSalvar(const Recorde *ranking, int total, const char *arquivo)
{
    FILE *f = fopen(arquivo, "w");
    if (f == NULL) return 0;

    for (int i = 0; i < total; i++) {
        fprintf(f, "%s %d\n", ranking[i].nome, ranking[i].pontos);
    }

    return fclose(f) == 0;
}

int rankingQualifica(const Recorde *ranking, int total, int pontos)
{
    if (pontos <= 0) return 0;
    return total < TOP_RANKING || pontos > ranking[total - 1].pontos;
}
