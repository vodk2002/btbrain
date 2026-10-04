#ifndef BTSCAN_H
#define BTSCAN_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "secp256k1.h"

typedef long long LONG64;

extern volatile LONG64 g_stop;
extern volatile LONG64 g_checked;   /* llaves probadas */
extern volatile LONG64 g_wins;      /* matches con saldo */
extern volatile LONG64 g_empty;     /* matches confirmados sin saldo */
extern volatile LONG64 g_m[5];      /* matches por tipo */
extern HANDLE g_con;
extern int g_threads;
extern char g_db[512];
extern CRITICAL_SECTION g_cs;
extern char g_phrase[256];              /* ultima frase, "" = ocultar */
extern volatile LONG64 g_saved_tick;    /* tick del ultimo guardado */
extern int g_mode;                      /* 0 = brain, 1 = azar */

/* consola */
void color(int c);
int con_w(void);

/* bloom.c */
void bloom_init(void);
void bloom_add(const char* s);
int bloom_maybe(const char* s);
unsigned long long bloom_load(const char* path); /* barra de bloques */

/* gen.c */
int gen_key(unsigned char key[32]); /* CSPRNG, 0 = reintentar */

/* addr.c: a[5] = P2PKH, P2WPKH, P2SH, P2WSH, P2WSH-P2SH. 0 = llave invalida */
int addr_derive(const unsigned char key[32], secp256k1_context* ctx,
                char a[5][96]);
int addr_selftest(void);

/* net.c: saldo on-chain en satoshis. <0 sin red */
long long chain_balance(const char* addr);

/* banner.c */
void banner(void);          /* demonio + cuadro INFO al lado */
void banner_brain(void);    /* demonio + cuadro INFO brain */
void menu_draw(int dbok);   /* demonio + menu principal */

/* tui.c */
void prog_draw(void);

/* gen5.c */
void gen5_phrase(unsigned long long n, int cls, char* out, size_t outn);
unsigned long long gen5_space(int cls); /* 0 = infinita */
void phrase_key(const char* phrase, unsigned char key[32]);

/* win.c */
void win_box(const char a[5][96], const char* hex, const char* wif,
             long long sat);

#endif
