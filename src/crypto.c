/* crypto.c - base58/bech32/keccak/hash + helpers de llaves */
#include "scan.h"

static const char* B58A = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

void base58_raw(const unsigned char* data, size_t len, char* out, size_t outn) {
    size_t zeros = 0;
    size_t bsz, size = 0, p = 0, o = 0, i, j, k;
    unsigned char b58[64];
    memset(b58, 0, sizeof(b58));
    while (zeros < len && data[zeros] == 0) ++zeros;
    bsz = (len - zeros) * 138 / 100 + 1;
    if (bsz > sizeof(b58)) bsz = sizeof(b58);
    for (i = zeros; i < len; ++i) {
        int carry = data[i];
        j = 0;
        for (k = 0; k < bsz; ++k) {
            size_t idx = bsz - 1 - k;
            if (carry == 0 && j >= size) break;
            carry += 256 * b58[idx];
            b58[idx] = (unsigned char)(carry % 58);
            carry /= 58;
            ++j;
        }
        size = j;
    }
    while (p < bsz && b58[p] == 0) ++p;
    for (i = 0; i < zeros && o + 1 < outn; ++i) out[o++] = '1';
    for (; p < bsz && o + 1 < outn; ++p) out[o++] = B58A[b58[p]];
    out[o] = 0;
}

void base58check(unsigned char ver, const unsigned char* payload, size_t plen,
                 char* out, size_t outn) {
    unsigned char v[64];
    unsigned char h1[32], h2[32];
    SHA256_CTX c;
    if (plen + 5 > sizeof(v)) { out[0] = 0; return; }
    v[0] = ver;
    memcpy(v + 1, payload, plen);
    SHA256_Init(&c); SHA256_Update(&c, v, 1 + plen); SHA256_Final(h1, &c);
    SHA256_Init(&c); SHA256_Update(&c, h1, 32); SHA256_Final(h2, &c);
    memcpy(v + 1 + plen, h2, 4);
    base58_raw(v, 1 + plen + 4, out, outn);
}

static const char* B32C = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
static const uint32_t B32G[5] = { 0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3 };

static uint32_t b32_polymod(const int* v, size_t n) {
    uint32_t chk = 1;
    size_t i;
    int j;
    for (i = 0; i < n; ++i) {
        uint32_t b = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) ^ (uint32_t)v[i];
        for (j = 0; j < 5; ++j)
            if ((b >> j) & 1) chk ^= B32G[j];
    }
    return chk;
}

static void bech32_impl(const char* hrp, int ver, const unsigned char* prog, size_t plen,
                        char* out, size_t outn, uint32_t spec) {
    int data[128], values[256];
    size_t nd = 0, nv = 0, i, h;
    uint32_t acc = 0;
    int bits = 0;
    uint32_t pm;
    size_t o = 0;
    out[0] = 0;
    if (ver < 0 || ver > 16 || plen < 2 || plen > 40) return;
    if (ver == 0 && plen != 20 && plen != 32) return;
    data[nd++] = ver;
    for (i = 0; i < plen; ++i) {
        acc = (acc << 8) | prog[i];
        bits += 8;
        while (bits >= 5) { bits -= 5; data[nd++] = (acc >> bits) & 31; }
    }
    if (bits > 0) data[nd++] = (int)((acc << (5 - bits)) & 31);
    for (h = 0; hrp[h]; ++h) values[nv++] = hrp[h] >> 5;
    values[nv++] = 0;
    for (h = 0; hrp[h]; ++h) values[nv++] = hrp[h] & 31;
    for (i = 0; i < nd; ++i) values[nv++] = data[i];
    for (i = 0; i < 6; ++i) values[nv++] = 0;
    pm = b32_polymod(values, nv) ^ spec;
    for (i = 0; i < 6; ++i) data[nd++] = (int)((pm >> (5 * (5 - i))) & 31);
    for (h = 0; hrp[h] && o + 1 < outn; ++h) out[o++] = hrp[h];
    if (o + 1 < outn) out[o++] = '1';
    for (i = 0; i < nd && o + 1 < outn; ++i) out[o++] = B32C[data[i]];
    out[o] = 0;
}

void bech32_encode(const char* hrp, int ver, const unsigned char* prog, size_t plen,
                   char* out, size_t outn) {
    bech32_impl(hrp, ver, prog, plen, out, outn, 1u);
}

void bech32m_encode(const char* hrp, int ver, const unsigned char* prog, size_t plen,
                    char* out, size_t outn) {
    if (ver != 1 || plen != 32) { out[0] = 0; return; }
    bech32_impl(hrp, ver, prog, plen, out, outn, 0x2bc830a3u);
}

/* CashAddr P2PKH (Bitcoin Cash). Sin prefijo = coincide con DB bch_funded.tsv.
   Con prefijo (with_prefix=1) = "bitcoincash:..." para APIs. */
static const char* CASHSET = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
static const uint64_t CASHG[5] = {
    0x98f2bc8e61ULL, 0x79b76d99e2ULL, 0xf33e5fb3c4ULL,
    0xae2eabe2a8ULL, 0x1e4f43e470ULL
};

static uint64_t cash_polymod(const int* v, size_t n) {
    uint64_t c = 1;
    size_t i;
    int j;
    for (i = 0; i < n; ++i) {
        uint64_t b = c >> 35;
        c = ((c & 0x07ffffffffULL) << 5) ^ (uint64_t)v[i];
        for (j = 0; j < 5; ++j)
            if ((b >> j) & 1) c ^= CASHG[j];
    }
    return c;
}

void cashaddr_p2pkh(const unsigned char h160[20], char* out, size_t outn,
                    int with_prefix) {
    static const char* prefix = "bitcoincash";
    unsigned char payload[21];
    int data[64], vals[128];
    size_t nd = 0, nv = 0, i, o = 0;
    uint32_t acc = 0;
    int bits = 0;
    uint64_t pm;
    out[0] = 0;
    payload[0] = 0x00; /* version P2PKH 160-bit */
    memcpy(payload + 1, h160, 20);
    for (i = 0; i < 21; ++i) {
        acc = (acc << 8) | payload[i];
        bits += 8;
        while (bits >= 5) { bits -= 5; data[nd++] = (acc >> bits) & 31; }
    }
    if (bits > 0) data[nd++] = (int)((acc << (5 - bits)) & 31);
    for (i = 0; prefix[i]; ++i) vals[nv++] = prefix[i] & 31;
    vals[nv++] = 0;
    for (i = 0; i < nd; ++i) vals[nv++] = data[i];
    for (i = 0; i < 8; ++i) vals[nv++] = 0;
    pm = cash_polymod(vals, nv) ^ 1;
    for (i = 0; i < 8; ++i) data[nd++] = (int)((pm >> (5 * (7 - i))) & 31);
    if (with_prefix) {
        for (i = 0; prefix[i] && o + 1 < outn; ++i) out[o++] = prefix[i];
        if (o + 1 < outn) out[o++] = ':';
    }
    for (i = 0; i < nd && o + 1 < outn; ++i) out[o++] = CASHSET[data[i]];
    out[o] = 0;
}

/* BIP86 taproot: x-only de salida. Devuelve 1 ok. */
int taproot_xonly(const unsigned char seckey[32],
                  EC_GROUP* g, BN_CTX* ctx, unsigned char xout[32]) {
    BIGNUM* n = BN_new();
    BIGNUM* s = BN_bin2bn(seckey, 32, NULL);
    BIGNUM* t = BN_new();
    BIGNUM* k = BN_new();
    EC_POINT* P = NULL;
    EC_POINT* Q = NULL;
    unsigned char tag[32], pre[96], th[32];
    unsigned char c[33];
    int ok = 0;
    SHA256_CTX sc;
    if (!n || !s || !t || !k) goto done;
    EC_GROUP_get_order(g, n, ctx);
    if (BN_is_zero(s) || BN_cmp(s, n) >= 0) goto done;
    P = EC_POINT_new(g);
    if (!P || !EC_POINT_mul(g, P, s, NULL, NULL, ctx)) goto done;
    if (EC_POINT_point2oct(g, P, POINT_CONVERSION_COMPRESSED, c, 33, ctx) != 33) goto done;
    if (c[0] == 0x03) { /* Y impar: negar seckey */
        BIGNUM* neg = BN_new();
        if (!neg) goto done;
        BN_sub(neg, n, s);
        BN_free(s);
        s = neg;
    }
    /* tagged_hash("TapTweak", x) */
    SHA256_Init(&sc); SHA256_Update(&sc, "TapTweak", 8); SHA256_Final(tag, &sc);
    memcpy(pre, tag, 32);
    memcpy(pre + 32, tag, 32);
    memcpy(pre + 64, c + 1, 32);
    SHA256_Init(&sc); SHA256_Update(&sc, pre, 96); SHA256_Final(th, &sc);
    BN_bin2bn(th, 32, t);
    if (BN_cmp(t, n) >= 0) BN_sub(t, t, n);
    BN_mod_add(k, s, t, n, ctx);
    if (BN_is_zero(k)) goto done;
    Q = EC_POINT_new(g);
    if (!Q || !EC_POINT_mul(g, Q, k, NULL, NULL, ctx)) goto done;
    if (EC_POINT_point2oct(g, Q, POINT_CONVERSION_COMPRESSED, c, 33, ctx) != 33) goto done;
    memcpy(xout, c + 1, 32);
    ok = 1;
done:
    if (P) EC_POINT_free(P);
    if (Q) EC_POINT_free(Q);
    BN_free(n); BN_free(s); BN_free(t); BN_free(k);
    return ok;
}

static uint64_t krol(uint64_t x, int n) { return n == 0 ? x : ((x << n) | (x >> (64 - n))); }
static const uint64_t KRC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};
static const int KRO[5][5] = {
    { 0, 36, 3, 41, 18 }, { 1, 44, 10, 45, 2 }, { 62, 6, 43, 15, 61 },
    { 28, 55, 25, 21, 56 }, { 27, 20, 39, 8, 14 }
};
static void keccakf(uint64_t s[25]) {
    int r, x, y;
    uint64_t c[5], d[5], b[25];
    for (r = 0; r < 24; ++r) {
        for (x = 0; x < 5; ++x) c[x] = s[x] ^ s[x+5] ^ s[x+10] ^ s[x+15] ^ s[x+20];
        for (x = 0; x < 5; ++x) d[x] = c[(x+4)%5] ^ krol(c[(x+1)%5], 1);
        for (x = 0; x < 5; ++x) for (y = 0; y < 5; ++y) s[x+5*y] ^= d[x];
        for (x = 0; x < 5; ++x) for (y = 0; y < 5; ++y) b[y+5*((2*x+3*y)%5)] = krol(s[x+5*y], KRO[x][y]);
        for (x = 0; x < 5; ++x) for (y = 0; y < 5; ++y) s[x+5*y] = b[x+5*y] ^ ((~b[(x+1)%5+5*y]) & b[(x+2)%5+5*y]);
        s[0] ^= KRC[r];
    }
}

void keccak256(const unsigned char* in, size_t len, unsigned char out[32]) {
    uint64_t s[25];
    unsigned char blk[136];
    size_t i, off = 0;
    memset(s, 0, sizeof(s));
    while (len - off >= 136) {
        for (i = 0; i < 17; ++i) {
            uint64_t w = 0;
            int b;
            for (b = 0; b < 8; ++b) w |= (uint64_t)in[off + i*8 + b] << (8*b);
            s[i] ^= w;
        }
        keccakf(s);
        off += 136;
    }
    memset(blk, 0, sizeof(blk));
    memcpy(blk, in + off, len - off);
    blk[len - off] = 0x01;
    blk[135] |= 0x80;
    for (i = 0; i < 17; ++i) {
        uint64_t w = 0;
        int b;
        for (b = 0; b < 8; ++b) w |= (uint64_t)blk[i*8 + b] << (8*b);
        s[i] ^= w;
    }
    keccakf(s);
    for (i = 0; i < 4; ++i) {
        int b;
        for (b = 0; b < 8; ++b) out[i*8 + b] = (unsigned char)((s[i] >> (8*b)) & 0xff);
    }
}

void hash160(const unsigned char* data, size_t len, unsigned char out[20]) {
    unsigned char h[32];
    SHA256_CTX c;
    RIPEMD160_CTX r;
    SHA256_Init(&c);
    if (len) SHA256_Update(&c, data, len);
    SHA256_Final(h, &c);
    RIPEMD160_Init(&r);
    RIPEMD160_Update(&r, h, 32);
    RIPEMD160_Final(out, &r);
}

void p2sh_addr(const unsigned char h160[20], unsigned char ver, char* out, size_t outn) {
    unsigned char redeem[22], rh[20];
    redeem[0] = 0x00;
    redeem[1] = 0x14;
    memcpy(redeem + 2, h160, 20);
    hash160(redeem, 22, rh);
    base58check(ver, rh, 20, out, outn);
}

void wif_encode(const unsigned char key[32], char* out, size_t outn) {
    unsigned char vbuf[34], h[32], h2[32], full[38];
    SHA256_CTX c;
    vbuf[0] = 0x80;
    memcpy(vbuf + 1, key, 32);
    vbuf[33] = 0x01;
    memcpy(full, vbuf, 34);
    SHA256_Init(&c); SHA256_Update(&c, full, 34); SHA256_Final(h, &c);
    SHA256_Init(&c); SHA256_Update(&c, h, 32); SHA256_Final(h2, &c);
    memcpy(full + 34, h2, 4);
    base58_raw(full, 38, out, outn);
}

void eth_addr(const unsigned char pubu[65], char* out) {
    unsigned char kh[32];
    keccak256(pubu + 1, 64, kh);
    out[0] = '0';
    out[1] = 'x';
    bytes_hex(kh + 12, 20, out + 2);
}

void bytes_hex(const unsigned char* b, size_t n, char* out) {
    static const char* H = "0123456789abcdef";
    size_t i;
    for (i = 0; i < n; ++i) { out[2*i] = H[b[i] >> 4]; out[2*i+1] = H[b[i] & 15]; }
    out[2*n] = 0;
}

void hex_bytes(const char* h, unsigned char* out, size_t n) {
    size_t i, j;
    memset(out, 0, n);
    for (i = 0; i < n; ++i) {
        unsigned b = 0;
        for (j = 0; j < 2; ++j) {
            char c = h[2*i + j];
            b <<= 4;
            if (c >= '0' && c <= '9') b |= (unsigned)(c - '0');
            else if (c >= 'a' && c <= 'f') b |= (unsigned)(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') b |= (unsigned)(c - 'A' + 10);
        }
        out[i] = (unsigned char)b;
    }
}

void key_add(const unsigned char base[32], unsigned long long n, unsigned char out[32]) {
    unsigned long long carry = n;
    int i;
    memcpy(out, base, 32);
    for (i = 31; i >= 0 && carry; --i) {
        unsigned long long v = (unsigned long long)out[i] + (carry & 0xff);
        out[i] = (unsigned char)(v & 0xff);
        carry = (carry >> 8) + (v >> 8);
    }
}
