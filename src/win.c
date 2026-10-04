/* win.c - caja verde de HIT + registro en winners.txt */
#include "btscan.h"

void win_box(const char a[5][96], const char* hex, const char* wif,
             long long sat) {
    FILE* f;
    EnterCriticalSection(&g_cs);
    printf("\n");
    color(10);
    printf(" +============================================================+\n");
    printf(" | Win Wallet Information:                                    |\n");
    printf(" | ADDRESS: (%s)\n", a[0]);
    printf(" | BALANCE: ( %.8f BTC )\n", sat / 100000000.0);
    printf(" | PrivateKey: %s\n", hex);
    printf(" | WIF: %s\n", wif);
    printf(" +============================================================+\n");
    color(7);
    fflush(stdout);
    LeaveCriticalSection(&g_cs);
    f = fopen("winners.txt", "a");
    if (f) {
        fprintf(f, "P2PKH=%s P2WPKH=%s P2SH=%s P2WSH=%s P2WSH-P2SH=%s\n",
                a[0], a[1], a[2], a[3], a[4]);
        fprintf(f, "priv=%s wif=%s sat=%lld\n\n", hex, wif, sat);
        fclose(f);
    }
}
