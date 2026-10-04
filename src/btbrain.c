/* btbrain.c - cazador de brainwallets ES+EN (generativo, sin diccionarios).
   Frase -> SHA256 -> llave -> 5 tipos -> bloom -> confirma saldo.
   5 odometros con estado (brain_estado.txt). Solo habla en WIN. */
#include "btscan.h"
#include "secp256k1.h"
#include <time.h>
#include <bcrypt.h>

/* de crypto.c (se compila junto, sin modificar) */
void wif_encode(const unsigned char key[32], char* out, size_t outn);
void bytes_hex(const unsigned char* b, size_t n, char* out);

static unsigned long long sc[6] = {0, 0, 0, 0, 0, 0};
static const unsigned long long SPS[5] = {0, 0, 0, 0, 0}; /* se llena abajo */

static void state_save(void) {
    FILE* f = fopen("brain_estado.txt", "w");
    if (f) {
        fprintf(f, "%llu %llu %llu %llu %llu %llu\n", sc[0], sc[1], sc[2], sc[3], sc[4], sc[5]);
        fclose(f);
        g_saved_tick = GetTickCount64();
    }
}
static void state_load(void) {
    FILE* f = fopen("brain_estado.txt", "r");
    if (f) {
        if (fscanf(f, "%llu %llu %llu %llu %llu %llu",
                   &sc[0], &sc[1], &sc[2], &sc[3], &sc[4], &sc[5]) >= 5)
            printf("  Estado retomado.\n");
        fclose(f);
    }
}

static void check_key(const unsigned char key[32], secp256k1_context* ctx) {
    char a[5][96], hex[65], wif[80];
    int t;
    if (!addr_derive(key, ctx, a)) return;
    InterlockedIncrement64(&g_checked);
    for (t = 0; t < 5; ++t) {
        long long sat;
        if (!bloom_maybe(a[t])) continue;
        InterlockedIncrement64(&g_m[t]);
        sat = chain_balance(a[t]);
        if (sat > 0) {
            bytes_hex(key, 32, hex);
            wif_encode(key, wif, sizeof(wif));
            InterlockedIncrement64(&g_wins);
            win_box(a, hex, wif, sat);
        } else if (sat == 0) {
            InterlockedIncrement64(&g_empty);
        }
    }
}

static DWORD WINAPI worker(LPVOID arg) {
    secp256k1_context* ctx;
    unsigned char key[32];
    char phrase[256];
    int c = (int)(intptr_t)arg;
    (void)arg;
    ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    if (g_mode == 1) {
        /* azar: llave CSPRNG directa, sin frases ni estado */
        for (;;) {
            if (g_stop) break;
            if (BCryptGenRandom(NULL, key, 32, BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0) continue;
            if ((key[0] | key[1] | key[2] | key[3]) == 0) continue;
            check_key(key, ctx);
        }
    } else
    for (;;) {
        int cls;
        unsigned long long n;
        if (g_stop) break;
        /* round-robin de clases con odometro propio */
        for (cls = 0; cls < 6; ++cls) {
            unsigned long long sps;
            if (g_stop) break;
            sps = gen5_space((c + cls) % 6);
            if (sps && sc[(c + cls) % 6] >= sps) continue; /* jubilada: ni la toca */
            n = (unsigned long long)InterlockedIncrement64((volatile LONG64*)&sc[(c + cls) % 6]) - 1ULL;
            {
                int cc = (int)((c + cls) % 6);
                unsigned long long sps = gen5_space(cc);
                if (sps && n >= sps) continue; /* clase agotada: se jubila */
                gen5_phrase(n, cc, phrase, sizeof(phrase));
            }
            phrase_key(phrase, key);
            if ((n & 8191ULL) == 0ULL) {
                EnterCriticalSection(&g_cs);
                strncpy(g_phrase, phrase, sizeof(g_phrase) - 1);
                LeaveCriticalSection(&g_cs);
            }
            check_key(key, ctx);
        }
    }
    secp256k1_context_destroy(ctx);
    return 0;
}

static BOOL WINAPI ctrl(DWORD t) {
    (void)t;
    g_stop = 1;
    printf("\nDeteniendo...\n");
    return TRUE;
}

static void start_run(int mode);

int main(int argc, char** argv) {
    int i, have_threads = 0;
    int dbok = 0;
    int force_azar = 0;
    (void)SPS;
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--threads") && i + 1 < argc) {
            have_threads = 1;
            g_threads = atoi(argv[++i]);
            if (g_threads < 1) g_threads = 1;
            if (g_threads > 32) g_threads = 32;
        } else if (!strcmp(argv[i], "--db") && i + 1 < argc) {
            strncpy(g_db, argv[++i], sizeof(g_db) - 1);
        } else if (!strcmp(argv[i], "--azar")) {
            force_azar = 1;
        } else if (!strcmp(argv[i], "--selftest")) {
            /* frase "" y "password": imprime direcciones para cotejar */
            secp256k1_context* ctx;
            char a[5][96];
            unsigned char key[32];
            int t;
            const char* vec[2] = {"", "password"};
            ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
            for (t = 0; t < 2; ++t) {
                int k;
                phrase_key(vec[t], key);
                printf("frase=\"%s\"\n", vec[t]);
                if (addr_derive(key, ctx, a))
                    for (k = 0; k < 5; ++k) printf("  [%d] %s\n", k, a[k]);
            }
            secp256k1_context_destroy(ctx);
            return 0;
        } else {
            printf("uso: btbrain.exe [--threads N] [--db archivo.tsv] [--selftest]\n");
            return 1;
        }
    }
    if (!g_db[0]) {
        strncpy(g_db, "db.txt", sizeof(g_db) - 1);
    }
    srand((unsigned)time(NULL) ^ GetTickCount());
    state_load();
    bloom_init();
    if (force_azar) {
        unsigned long long n = bloom_load(g_db);
        printf("  DB: %llu direcciones. Modo azar directo.\n", n);
        start_run(1);
        return 0;
    }
    for (;;) {
        char line[32] = {0};
        int op;
        system("cls");
        menu_draw(dbok);
        if (!fgets(line, sizeof(line), stdin)) return 0;
        op = atoi(line);
        if (op == 0) return 0;
        if (op == 1) {
            if (dbok) {
                printf("  DB ya cargada. Enter para volver...");
            } else {
                unsigned long long n = bloom_load(g_db);
                printf("  DB: %llu direcciones en bloom (%.0f MB)\n",
                       n, 2500000000.0 / 8000000.0);
                dbok = 1;
                printf("  Enter para volver...");
            }
            fflush(stdout);
            fgets(line, sizeof(line), stdin);
        } else if (op == 2 || op == 3) {
            if (!dbok) {
                printf("  Carga la DB primero (opcion 1).\n");
                Sleep(1500);
                continue;
            }
            start_run(op == 3 ? 1 : 0);
        } else if (op == 4) {
            printf("\n Cuantos hilos quieres usar? [actual %d, max 32]: ", g_threads);
            fflush(stdout);
            if (fgets(line, sizeof(line), stdin)) {
                int v = atoi(line);
                if (v >= 1 && v <= 32) g_threads = v;
            }
            printf(" Usando %d hilos.\n", g_threads);
            Sleep(1200);
        }
    }
}

static void start_run(int mode) {
    HANDLE* th;
    ULONGLONG lastsave;
    int i;
    char line[32];
    g_stop = 0;
    g_mode = mode;
    g_phrase[0] = 0;
    system("cls");
    banner_brain();
    SetConsoleCtrlHandler(ctrl, TRUE);
    InitializeCriticalSection(&g_cs);
    th = (HANDLE*)malloc(sizeof(HANDLE) * (size_t)g_threads);
    for (i = 0; i < g_threads; ++i)
        th[i] = CreateThread(NULL, 0, worker, (LPVOID)(intptr_t)(i % 6), 0, NULL);
    lastsave = GetTickCount64();
    for (;;) {
        Sleep(250);
        if (g_stop) break;
        EnterCriticalSection(&g_cs);
        prog_draw();
        LeaveCriticalSection(&g_cs);
        if (GetTickCount64() - lastsave > 60000ULL) {
            lastsave = GetTickCount64();
            state_save();
        }
    }
    state_save();
    WaitForMultipleObjects(g_threads, th, TRUE, INFINITE);
    free(th);
    printf("\nParado. Revisadas: %lld  WIN: %lld  Emptys: %lld\n",
           (long long)g_checked, (long long)g_wins, (long long)g_empty);
    printf("Enter para volver al menu...");
    fflush(stdout);
    fgets(line, sizeof(line), stdin);
}
