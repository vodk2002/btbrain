/* bloom.c - filtro + carga de DB con barra de bloques */
#include "btscan.h"

#define BLOOM_BITS 2500000000ULL
#define BLOOM_K 12

static uint64_t* bits = NULL;

static uint64_t fnv1a(const char* s) {
    uint64_t h = 14695981039346656037ULL;
    while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211ULL; }
    return h;
}
static uint64_t splitmix(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

void bloom_init(void) {
    bits = (uint64_t*)calloc((size_t)(BLOOM_BITS / 64), 8);
    if (!bits) { fprintf(stderr, "[E] sin memoria para bloom\n"); exit(1); }
}
void bloom_add(const char* s) {
    uint64_t h1 = fnv1a(s), h2 = splitmix(h1) | 1ULL;
    int i;
    for (i = 0; i < BLOOM_K; ++i) {
        uint64_t b = (h1 + (uint64_t)i * h2) % BLOOM_BITS;
        bits[b >> 6] |= (1ULL << (b & 63));
    }
}
int bloom_maybe(const char* s) {
    uint64_t h1 = fnv1a(s), h2 = splitmix(h1) | 1ULL;
    int i;
    for (i = 0; i < BLOOM_K; ++i) {
        uint64_t b = (h1 + (uint64_t)i * h2) % BLOOM_BITS;
        if (!(bits[b >> 6] & (1ULL << (b & 63)))) return 0;
    }
    return 1;
}

unsigned long long bloom_load(const char* path) {
    FILE* f = fopen(path, "rb");
    static char buf[1 << 20];
    char line[128];
    size_t lpos = 0, read, i;
    unsigned long long count = 0, done = 0, fsize = 0;
    int first = 1, lastfill = -1;
    if (!f) { fprintf(stderr, "[E] no DB: %s\n", path); exit(1); }
    _fseeki64(f, 0, SEEK_END);
    fsize = (unsigned long long)_ftelli64(f);
    rewind(f);
    while ((read = fread(buf, 1, sizeof(buf), f)) > 0) {
        int fill;
        done += (unsigned long long)read;
        if (fsize) {
            fill = (int)(done * 30ULL / fsize);
            if (fill != lastfill) {
                int k;
                lastfill = fill;
                printf("\r  [");
                for (k = 0; k < 30; ++k) printf("%s", k < fill ? "#" : "-");
                printf("]");
                fflush(stdout);
            }
        }
        for (i = 0; i < read; ++i) {
            char ch = buf[i];
            if (ch == '\n') {
                char* sep;
                line[lpos] = 0;
                if (lpos > 0 && line[lpos - 1] == '\r') line[lpos - 1] = 0;
                if (first) {
                    first = 0;
                    if (strncmp(line, "address", 7) == 0) { lpos = 0; continue; }
                }
                sep = strchr(line, '\t');
                if (sep) *sep = 0;
                if (line[0]) { bloom_add(line); ++count; }
                lpos = 0;
            } else if (lpos + 1 < sizeof(line)) {
                line[lpos++] = ch;
            }
        }
    }
    fclose(f);
    printf("\r  [");
    {
        int k;
        for (k = 0; k < 30; ++k) printf("#");
    }
    printf("]\n");
    fflush(stdout);
    return count;
}
