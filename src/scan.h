#pragma once
/* BTCSCAN v2.0 - header compartido */
#define _CRT_SECURE_NO_WARNINGS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <threads.h>

#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>

#define VERSION "2.5"
#define BLOOM_K 14

/* ------------------------------- CLI ---------------------------------- */
typedef struct {
    unsigned char start[32];
    unsigned long long count;   /* 0 = infinito */
    int threads;
    int log_every;              /* 0 = solo HITS */
    char btc_db[512];
    char ltc_db[512];
    char doge_db[512];
    char bch_db[512];
    char eth_db[512];
    char hits[512];
    char nocheck[512];
    char empty[512];
    char enigmalog[512];    /* log estilo EnigmaCracker */
    int enigma;             /* 1 = bloque INFO con Seed+address+balance en candidatos */
    int no_uncompressed;    /* 1 = salta P2PKH sin comprimir (btc_u/bch_u): +~20% vel */
    int auto_update;        /* 1 = revisa dumps nuevos al abrir y los reemplaza */
    char state[512];          /* checkpoint seq */
    char vanity[128];         /* prefijo vanity (modo vanity, sin DB) */
    char words[512];          /* wordlist S4 */
    int random;           /* 1 = CSPRNG 256-bit (default), 0 = secuencial (--seq) */
    int seq;
    int verbose;          /* 1 = imprime lineas de candidatos (empty/nocheck) */
    unsigned long long share_m; /* --share M/N (solo seq): esta instancia es la M */
    unsigned long long share_n; /* N partes (0/1 = sin particion) */
    int vanity_mode;      /* 1 = buscar prefijo en vez de DB */
    int tui;              /* 1 = dashboard TUI (--tui) */
    int lea;              /* 1 = Loteria Estratificada Adaptativa */
    int estrat[4];        /* S1..S4 activas (1/0) */
    int retry;
    int retry_threads;
    int test;
} Opts;

/* ------------------------------ monedas -------------------------------- */
typedef struct {
    const char* name;   /* "BTC", "LTC", "ETH" */
    char sep;           /* '\t' BTC, ',' LTC/ETH */
    int lowercase;      /* 1 = ETH */
    int enabled;
    uint64_t* bits;
    uint64_t nbits;
    int k;
    unsigned long long count;
    unsigned long long fsize;   /* tamanio archivo para % de carga */
} Coin;

#define NCOINS 4

/* ----------------------------- globales -------------------------------- */
extern volatile LONG64 g_next;
extern volatile LONG64 g_checked;
extern volatile LONG64 g_hits;
extern volatile LONG64 g_empty;
extern volatile LONG64 g_nocheck;
extern volatile LONG64 g_hits_coin[4]; /* BTC LTC DOGE ETH */
extern volatile LONG64 g_st_n[4];      /* llaves probadas por estrategia LEA */
extern volatile LONG64 g_st_r[4];      /* recompensa observada (HITs+empties pond.) */
extern volatile LONG64 g_s3c;          /* cursor S3 */
extern volatile LONG64 g_s4c;          /* cursor S4 */
extern volatile LONG64 g_s2c;          /* cursor S2 (seq propio, sin robo entre hilos) */
extern volatile int g_wstrat[32];      /* estrategia por hilo (bandit) */
extern volatile LONG g_stop;
extern volatile LONG g_pause;
extern double g_usd[4];                /* precios USD por moneda */
extern double g_found_usd;             /* USD acumulado (bajo lock io) */
extern Opts g_opt;
extern mtx_t g_io;
extern Coin COINS[NCOINS];
extern HANDLE g_con;

/* ------------------------------ crypto.c ------------------------------- */
void base58_raw(const unsigned char* data, size_t len, char* out, size_t outn);
void base58check(unsigned char ver, const unsigned char* payload, size_t plen,
                 char* out, size_t outn);
void bech32_encode(const char* hrp, int ver, const unsigned char* prog, size_t plen,
                   char* out, size_t outn);
void bech32m_encode(const char* hrp, int ver, const unsigned char* prog, size_t plen,
                    char* out, size_t outn);
void cashaddr_p2pkh(const unsigned char h160[20], char* out, size_t outn,
                    int with_prefix);
int taproot_xonly(const unsigned char seckey[32],
                  EC_GROUP* g, BN_CTX* ctx, unsigned char xout[32]);
void keccak256(const unsigned char* in, size_t len, unsigned char out[32]);
void hash160(const unsigned char* data, size_t len, unsigned char out[20]);
void p2sh_addr(const unsigned char h160[20], unsigned char ver,
               char* out, size_t outn);
void wif_encode(const unsigned char key[32], char* out, size_t outn);
void eth_addr(const unsigned char pubu[65], char* out /* "0x"+40 */);
void bytes_hex(const unsigned char* b, size_t n, char* out);
void hex_bytes(const char* h, unsigned char* out, size_t n);
void key_add(const unsigned char base[32], unsigned long long n, unsigned char out[32]);

/* -------------------------------- db.c --------------------------------- */
void bloom_add(Coin* c, const char* s);
int bloom_maybe(Coin* c, const char* s);
unsigned long long load_db(Coin* c, const char* path);

/* -------------------------------- net.c -------------------------------- */
typedef int (*confirm_fn)(const char*, double*, const char**, char*, size_t);
int confirm_btc(const char* a, double* amt, const char** unit, char* s, size_t n);
int confirm_ltc(const char* a, double* amt, const char** unit, char* s, size_t n);
int confirm_bch(const char* a, double* amt, const char** unit, char* s, size_t n);
int confirm_eth(const char* a, double* amt, const char** unit, char* s, size_t n);
confirm_fn confirm_of(int ci);
void net_init(void);
void fetch_prices(void);

/* -------------------------------- ui.c --------------------------------- */
void color(int c);
void banner(void);
void now_str(char* out, size_t n);
void now_str_ms(char* out, size_t n); /* YYYY-MM-DD HH:MM:SS,mmm estilo Enigma */
void print_hit_block(FILE* f, LONG64 n, const char* ts, const char* coin,
                     const char* addr, const char* hex, const char* wif,
                     double amt, const char* unit, const char* sources,
                     double usd);
void tui_init(void);
void tui_end(void);
void tui_draw(LONG64 checked, LONG64 sps);
int tui_key(void);
void tui_hit_push(const char* coin, const char* addr, double amt, const char* unit);
void tui_log_push(const char* line);

/* ------------------------------- worker.c ------------------------------ */
int worker(void* arg);
int confirmer(void* arg);
void cq_init(void);
void cq_shutdown(void);
void save_hit(const char* coin, const char* addr, const char* hex,
              const char* wif, double amt, const char* unit, const char* sources);

/* ----------------------------- strategies.c ---------------------------- */
int strat_uniform(unsigned char key[32]);
void strat_structured(unsigned long long c, unsigned char key[32]);
int words_load(const char* path);
int words_count(void);
int strat_brain(unsigned long long c, unsigned char key[32]);
void mnemonic_encode(const unsigned char key[32], char* out, size_t outn);
int strat_selftest(void);
int strat_selftest_pool(void);

/* -------------------------------- main.c ------------------------------- */
void exe_dir(char* out, size_t n);
void parse_hex32(const char* h, unsigned char out[32]);

/* ------------------------------- update.c ------------------------------ */
void update_check(int want_btc, int want_bch);
