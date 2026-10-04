/* addr.c - derivacion de los 5 tipos por llave + selftest */
#include "btscan.h"
#include <openssl/sha.h>

/* de crypto.c (se compila junto, sin modificar) */
void base58check(unsigned char ver, const unsigned char* payload, size_t plen,
                 char* out, size_t outn);
void bech32_encode(const char* hrp, int ver, const unsigned char* prog, size_t plen,
                   char* out, size_t outn);
void hash160(const unsigned char* data, size_t len, unsigned char out[20]);
void wif_encode(const unsigned char key[32], char* out, size_t outn);
void bytes_hex(const unsigned char* b, size_t n, char* out);

int addr_derive(const unsigned char key[32], secp256k1_context* ctx,
                char a[5][96]) {
    secp256k1_pubkey pk;
    unsigned char pubc[33], h160[20], sh[32], scr[34], rh[20];
    size_t outl;
    if (!secp256k1_ec_pubkey_create(ctx, &pk, key)) return 0;
    outl = 33;
    if (!secp256k1_ec_pubkey_serialize(ctx, pubc, &outl, &pk,
                                       SECP256K1_EC_COMPRESSED) || outl != 33)
        return 0;
    hash160(pubc, 33, h160);
    base58check(0x00, h160, 20, a[0], 96);          /* P2PKH */
    bech32_encode("bc", 0, h160, 20, a[1], 96);     /* P2WPKH */
    scr[0] = 0x00; scr[1] = 0x14;                   /* P2WPKH-in-P2SH */
    memcpy(scr + 2, h160, 20);
    hash160(scr, 22, rh);
    base58check(0x05, rh, 20, a[2], 96);
    SHA256(pubc, 33, sh);                           /* P2WSH */
    bech32_encode("bc", 0, sh, 32, a[3], 96);
    scr[0] = 0x00; scr[1] = 0x20;                   /* P2WSH-in-P2SH */
    memcpy(scr + 2, sh, 32);
    hash160(scr, 34, rh);
    base58check(0x05, rh, 20, a[4], 96);
    return 1;
}

int addr_selftest(void) {
    /* llave = 1: vectores verificados con implementacion Python independiente */
    secp256k1_context* ctx;
    unsigned char key[32] = {0};
    char a[5][96];
    int fails = 0;
    key[31] = 1;
    ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    if (!addr_derive(key, ctx, a)) { printf("FAIL secp\n"); return 1; }
    secp256k1_context_destroy(ctx);
#define CHECK(got, exp) do { \
        if (strcmp((got), (exp)) == 0) printf("  OK %s\n", got); \
        else { printf("  FAIL got=%s exp=%s\n", (got), (exp)); ++fails; } \
    } while (0)
    CHECK(a[0], "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH");
    CHECK(a[1], "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4");
    CHECK(a[2], "3JvL6Ymt8MVWiCNHC7oWU6nLeHNJKLZGLN");
    CHECK(a[3], "bc1qpac4ht6afshdx2tctnhjnetz7u6g3j9zhwwmc4cqkdsa2jumq42qd3drf7");
    printf(fails ? "SELFTEST: FALLA\n" : "SELFTEST: TODO OK\n");
    return fails ? 1 : 0;
}
