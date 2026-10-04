/* tui.c - UNA sola linea viva con \r (como la repo original). Sin cursor. */
#include "btscan.h"

void prog_draw(void) {
    /* UNA linea, recortada al ancho real. Con frase en vivo y * al guardar. */
    static ULONGLONG t0 = 0;
    LONG64 c = g_checked, w = g_wins, e = g_empty;
    char wbuf[32], cbuf[32], ebuf[32], spbuf[32], ph[28];
    int W, full, pad, showph = 0;
    ULONGLONG now = GetTickCount64();
    double el, sp;
    if (!t0) t0 = now;
    el = (now - t0) / 1000.0;
    if (el < 0.5) el = 0.5;
    sp = c / el;
    snprintf(wbuf, sizeof(wbuf), "%lld", (long long)w);
    snprintf(cbuf, sizeof(cbuf), "%lld", (long long)c);
    snprintf(ebuf, sizeof(ebuf), "%lld", (long long)e);
    snprintf(spbuf, sizeof(spbuf), "%.0f", sp);
    strncpy(ph, g_phrase, 24); /* main ya trae el lock; display tolera corte */
    ph[24] = 0;
    showph = ph[0] ? 1 : 0;
    W = con_w();
    full = 85 + (int)strlen(wbuf) + 4 * (int)strlen(cbuf)
         + (int)strlen(ebuf) + (int)strlen(spbuf)
         + (showph ? 10 + (int)strlen(ph) : 0)
         + ((now - g_saved_tick < 2000ULL) ? 2 : 0);
    if (full < W) {
        color(14); printf("\rTotal WIN: (");
        color(12); printf("%s", wbuf);
        color(14); printf(") --- P2PKH(");
        color(12); printf("%s", cbuf);
        color(14); printf(") --- P2WPKH(");
        color(12); printf("%s", cbuf);
        color(14); printf(") --- P2SH(");
        color(12); printf("%s", cbuf);
        color(14); printf(") --- P2WSH(");
        color(12); printf("%s", cbuf);
        color(14); printf(") --- Emptys: ");
        color(12); printf("%s", ebuf);
        color(14); printf(" --- %s/s", spbuf);
        if (showph) { color(14); printf(" --- Frase: "); color(11); printf("%s", ph); }
        if (now - g_saved_tick < 2000ULL) { color(10); printf(" *"); }
        pad = (W - 1) - full;
    } else {
        full = 35 + (int)strlen(wbuf) + 4 * (int)strlen(cbuf)
             + (int)strlen(ebuf) + (int)strlen(spbuf);
        if (full >= W) full = W - 1;
        color(14); printf("\rW:(");
        color(12); printf("%s", wbuf);
        color(14); printf(") PKH(");
        color(12); printf("%s", cbuf);
        color(14); printf(") WPKH(");
        color(12); printf("%s", cbuf);
        color(14); printf(") SH(");
        color(12); printf("%s", cbuf);
        color(14); printf(") WSH(");
        color(12); printf("%s", cbuf);
        color(14); printf(") E:");
        color(12); printf("%s", ebuf);
        color(14); printf(" %s/s", spbuf);
        if (now - g_saved_tick < 2000ULL) { color(10); printf(" *"); full += 2; }
        pad = (W - 1) - full;
    }
    if (pad > 0) printf("%*s", pad, "");
    color(7);
    fflush(stdout);
}
