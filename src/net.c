/* net.c - confirmacion de saldo on-chain (solo en match, raro) */
#include "btscan.h"
#include <wininet.h>

long long chain_balance(const char* addr) {
    HINTERNET net = NULL, url = NULL;
    char link[128], body[4096], *p;
    DWORD got = 0, total = 0;
    unsigned long long funded = 0, spent = 0;
    long long bal;
    DWORD to = 10000;
    snprintf(link, sizeof(link), "https://mempool.space/api/address/%s", addr);
    net = InternetOpenA("btscan/2.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!net) return -1;
    InternetSetOptionA(net, INTERNET_OPTION_CONNECT_TIMEOUT, &to, sizeof(to));
    InternetSetOptionA(net, INTERNET_OPTION_RECEIVE_TIMEOUT, &to, sizeof(to));
    url = InternetOpenUrlA(net, link, NULL, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!url) { InternetCloseHandle(net); return -1; }
    body[0] = 0;
    while (total < sizeof(body) - 1 &&
           InternetReadFile(url, body + total, (DWORD)(sizeof(body) - 1 - total), &got) && got) {
        total += got;
    }
    body[total] = 0;
    InternetCloseHandle(url);
    InternetCloseHandle(net);
    if (!strstr(body, "\"chain_stats\"")) return -1;
    p = strstr(body, "\"funded_txo_sum\":");
    if (p) funded = strtoull(p + 17, NULL, 10);
    p = strstr(body, "\"spent_txo_sum\":");
    if (p) spent = strtoull(p + 15, NULL, 10);
    bal = (long long)(funded - spent);
    return bal < 0 ? 0 : bal;
}
