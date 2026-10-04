/* gen5.c - generador procedural ES+EN (sin diccionarios externos).
   5 clases con odometro propio: singles, pares, trios, teclado, numeros.
   Todo determinista por indice -> reanudable sin repetir. ASCII sin acentos. */
#include "btscan.h"
#include <openssl/sha.h>

static const char* W[] = {
"amor","dios","madre","padre","hijo","vida","muerte","cielo","infierno",
"luz","oscuridad","sol","luna","estrella","mar","casa","familia","amigo",
"corazon","alma","fuerza","poder","dinero","oro","plata","sangre","fuego",
"agua","tierra","aire","mujer","hombre","nino","rey","reina","angel",
"demonio","cristo","jesus","maria","jose","pedro","juan","luis","ana",
"carmen","rosa","carlos","miguel","rafael","diego","andres","pablo",
"david","daniel","fernando","jorge","ricardo","alejandro","alberto",
"eduardo","raul","arturo","enrique","oscar","hector","marco","ivan",
"sergio","omar","hugo","mario","amar","querer","vivir","morir","ganar",
"perder","luchar","rezar","creer","sonar","volar","correr","cantar",
"reir","llorar","grande","bueno","malo","fuerte","eterno","santo",
"negro","blanco","rojo","wey","guey","chido","cabron","chingon","neta",
"verga","pedo","orale","naco","fresa","huevo","bitcoin","password",
"clave","wallet","moneda","crypto","minero","mexico","madrid","america",
"libertad","paz","guerra","equipo","futbol","cerveza","tequila","taco",
"love","god","mother","father","son","life","death","heaven","hell",
"light","dark","darkness","sun","moon","star","sea","house","family",
"friend","heart","soul","power","money","gold","silver","blood","fire",
"water","earth","air","woman","man","child","king","queen","angel",
"demon","christ","mary","joseph","peter","john","mike","sarah",
"michael","james","robert","emma","olivia","live","die","win","lose",
"fight","pray","believe","dream","fly","run","sing","laugh","cry",
"big","good","bad","strong","eternal","holy","black","white","red",
"dude","bro","cool","damn","bitch","shit","crazy","money","power",
"dragon","tiger","eagle","wolf","shadow","storm","thunder","angel",
"princess","prince","master","killer","hunter","warrior","ninja",
"pirate","jesus","faith","hope","grace","america","freedom","peace",
"war","team","football","beer","house","car","dog","cat"
};
#define NW (sizeof(W) / sizeof(W[0]))

static const char* KBD[] = {
"qwerty","asdf","zxcv","12345","123456","098765","qaz","wsx","edc",
"qwertyuiop","asdfghjkl","zxcvbnm","1q2w3e","1qaz2wsx","qweasd",
"poiuy","mnbvc","ytrewq","09876","54321","abcde","654321","qwer",
"rewq","asdfg","zxcvb","123456789","987654321","qazxsw","edcvfr",
"qwe","rty","uio","jkl","bnm","123123","abc123","password1"
};
#define NK (sizeof(KBD) / sizeof(KBD[0]))

static const char* LADA[] = {"55", "33", "81", "656", "999"};
#define NLADA (sizeof(LADA) / sizeof(LADA[0]))

static void upper1(char* d, const char* s) {
    size_t i;
    for (i = 0; s[i] && i < 250; ++i)
        d[i] = (char)((i == 0 && s[i] >= 'a' && s[i] <= 'z') ? s[i] - 32 : s[i]);
    d[i] = 0;
}
static void upperall(char* d, const char* s) {
    size_t i;
    for (i = 0; s[i] && i < 250; ++i)
        d[i] = (char)((s[i] >= 'a' && s[i] <= 'z') ? s[i] - 32 : s[i]);
    d[i] = 0;
}
static void lowerall(char* d, const char* s) {
    size_t i;
    for (i = 0; s[i] && i < 250; ++i)
        d[i] = (char)((s[i] >= 'A' && s[i] <= 'Z') ? s[i] + 32 : s[i]);
    d[i] = 0;
}
static void leet(char* d, const char* s) {
    size_t i, o = 0;
    for (i = 0; s[i] && o < 250; ++i) {
        char c = s[i];
        if (c == 'a' || c == 'A') d[o++] = '4';
        else if (c == 'e' || c == 'E') d[o++] = '3';
        else if (c == 'i' || c == 'I') d[o++] = '1';
        else if (c == 'o' || c == 'O') d[o++] = '0';
        else if (c == 's' || c == 'S') d[o++] = '5';
        else d[o++] = c;
    }
    d[o] = 0;
}

/* mutadores single: 0..11 fijos + 12..88 anio 1950..2026 */
static void single_mut(const char* w, unsigned long long m, char* out, size_t outn) {
    char t1[256];
    (void)outn;
    if (m == 0) { strncpy(out, w, 255); out[255] = 0; }
    else if (m == 1) upper1(out, w);
    else if (m == 2) upperall(out, w);
    else if (m == 3) lowerall(out, w);
    else if (m == 4) snprintf(out, 256, "%s123", w);
    else if (m == 5) snprintf(out, 256, "%s1234", w);
    else if (m == 6) snprintf(out, 256, "%s12345", w);
    else if (m == 7) snprintf(out, 256, "%s!", w);
    else if (m == 8) snprintf(out, 256, "%s01", w);
    else if (m == 9) leet(out, w);
    else if (m == 10) { leet(t1, w); snprintf(out, 256, "%s123", t1); }
    else if (m == 11) { upper1(t1, w); snprintf(out, 256, "%s123", t1); }
    else snprintf(out, 256, "%s%llu", w, 1950ULL + (m - 12ULL));
}

static const char* NAMES[] = {
"juan","pedro","maria","jose","luis","ana","carmen","rosa","carlos",
"miguel","diego","raul","hugo","jesus","cristo","dios","mama","papa",
"bebe","amor","vida","luz","sol","luna","rey","angel","john","mike",
"david","sarah","michael","james","robert","emma","olivia","god",
"mary","love","son","king","queen","faith","hope","grace","jesus",
"mama","papa","baby","amor","vida","luz","sol","rey","juan","pedro",
"maria","luis","ana","david","love","god","son","king"
};
#define NNAMES (sizeof(NAMES) / sizeof(NAMES[0]))

static const char* TPLP[][2] = {
{"teamo",""},{"dioses",""},{"jesuste",""},{"mama","tequiero"},
{"te","amo"},{"dios","esamor"},{"cristo","vive"},{"maria","madre"},
{"spanish",""},{"mexicano",""},{"tequiero",""},{"amordemivida",""},
{"iloveyou",""},{"godis",""},{"jesussaves",""},{"mama","iloveyou"},
{"i","love"},{"god","bless"},{"christ","isrisen"},{"mary","mother"},
{"dios","mio"},{"jesus","vive"},{"teamomama",""},{"teamopapa",""},
{"ilovegod",""},{"godislove",""},{"faith","hope"},{"amordedios",""},
{"santamaria",""},{"ave","maria"},{"padrenuestro",""},{"amen",""},
{"diosito",""},{"chiquito","pero"},{"teamo","mucho"},{"loveyou",""},
{"kiss","me"},{"hug","me"},{"my","love"},{"sweet","heart"},
{"corazon","mio"},{"alma","mia"},{"luz","divina"},{"sangre","joven"},
{"holy","ghost"},{"sonof","god"},{"daughter","ofgod"},{"heaven","awaits"},
};
#define NTPL (sizeof(TPLP) / sizeof(TPLP[0]))

static char kb_next(char c) {
    static const char* rows[4] = {"1234567890", "qwertyuiop", "asdfghjkl", "zxcvbnm"};
    int r, i;
    for (r = 0; r < 4; ++r)
        for (i = 0; rows[r][i]; ++i)
            if (rows[r][i] == c) {
                int j = (i > 0) ? i - 1 : i + 1;
                if (!rows[r][j]) j = i;
                return rows[r][j];
            }
    return c;
}

static void typo(const char* w, unsigned kind, unsigned pos, char* out) {
    size_t n = strlen(w), i, o = 0;
    char t[256];
    if (n > 200) n = 200;
    if (n < 2) { strncpy(out, w, 255); out[255] = 0; return; }
    memcpy(t, w, n); t[n] = 0;
    {
        unsigned p = pos % (unsigned)n;
        if (kind == 0) {
            unsigned q = (p + 1) % (unsigned)n;
            char tmp = t[p]; t[p] = t[q]; t[q] = tmp;
            strncpy(out, t, 255); out[255] = 0;
        } else if (kind == 1) {
            for (i = 0; i < n && o < 250; ++i) { out[o++] = t[i]; if (i == p) out[o++] = t[i]; }
            out[o] = 0;
        } else if (kind == 2) {
            for (i = 0; i < n && o < 250; ++i) if (i != p) out[o++] = t[i];
            out[o] = 0;
        } else {
            t[p] = kb_next(t[p]);
            strncpy(out, t, 255); out[255] = 0;
        }
    }
}

/* mutadores single: 0..11 fijos, 12..88 anio, 89..92 typos, 93..99 sufijos */
static void single_mut2(const char* w, unsigned m, unsigned pos, char* out) {
    static const char* sufs[7] = {"007", "69", "666", "777", "2012", "2000", "1212"};
    char t1[256];
    if (m < 12) { single_mut(w, m, out, 256); return; }
    if (m < 89) { snprintf(out, 256, "%s%llu", w, 1950ULL + (m - 12ULL)); return; }
    if (m < 93) { typo(w, m - 89U, pos, out); return; }
    snprintf(out, 256, "%s%s", w, sufs[m - 93U]);
}

static void rev(const char* s, char* out) {
    size_t n = strlen(s), i;
    if (n > 250) n = 250;
    for (i = 0; i < n; ++i) out[i] = s[n - 1 - i];
    out[n] = 0;
}

/* clase 0..4, n = indice local. out siempre termina en 0. */
void gen5_phrase(unsigned long long n, int cls, char* out, size_t outn) {
    (void)outn;
    if (cls == 0) {
        const char* w = W[n % NW];
        unsigned long long j = n / NW;
        single_mut2(w, (unsigned)(j % 100ULL), (unsigned)((j / 100ULL) % 8ULL), out);
    } else if (cls == 1) {
        unsigned long long k = n;
        char a[256], b[256];
        const char* seps[5] = {"", " ", "_", "-", "."};
        unsigned st;
        strncpy(a, W[k % NW], 255); a[255] = 0; k /= NW;
        strncpy(b, W[k % NW], 255); b[255] = 0; k /= NW;
        st = (unsigned)(k % 10ULL);
        {
            char aa[256], bb[256];
            if (st / 5) { upper1(aa, a); upper1(bb, b); }
            else { strncpy(aa, a, 255); aa[255] = 0; strncpy(bb, b, 255); bb[255] = 0; }
            snprintf(out, 256, "%s%s%s", aa, seps[st % 5], bb);
        }
    } else if (cls == 2) {
        unsigned long long k = n;
        char a[256], b[256], c[256];
        const char* seps[6] = {"", " ", "_", "-", ".", "/"};
        unsigned st;
        strncpy(a, W[k % NW], 255); a[255] = 0; k /= NW;
        strncpy(b, W[k % NW], 255); b[255] = 0; k /= NW;
        strncpy(c, W[k % NW], 255); c[255] = 0; k /= NW;
        st = (unsigned)(k % 6ULL);
        snprintf(out, 256, "%s%s%s%s%s", a, seps[st], b, seps[st], c);
    } else if (cls == 3) {
        unsigned long long k = n;
        char t[256], r[256];
        unsigned v;
        strncpy(t, KBD[k % NK], 255); t[255] = 0; k /= NK;
        v = (unsigned)(k % 5ULL);
        if (v == 0) snprintf(out, 256, "%s", t);
        else if (v == 1) snprintf(out, 256, "%s123", t);
        else if (v == 2) snprintf(out, 256, "%s!", t);
        else if (v == 3) { rev(t, r); snprintf(out, 256, "%s", r); }
        else { rev(t, r); snprintf(out, 256, "%s123", r); }
    } else if (cls == 5) {
        /* plantillas humanas ES+EN con nombre + sufijo de gente real */
        unsigned long long k = n;
        const char* nm;
        unsigned sfx, tp;
        char suf[16];
        nm = NAMES[k % NNAMES]; k /= NNAMES;
        tp = (unsigned)(k % NTPL); k /= NTPL;
        sfx = (unsigned)(k % 80ULL);
        if (sfx == 0) suf[0] = 0;
        else if (sfx == 1) strcpy(suf, "123");
        else if (sfx == 2) strcpy(suf, "!");
        else snprintf(suf, sizeof(suf), "%u", 1950U + (sfx - 3U));
        snprintf(out, 256, "%s%s%s%s", TPLP[tp][0], nm, TPLP[tp][1], suf);
    } else {
        unsigned sub = (unsigned)(n % 3ULL);
        unsigned long long k = n / 3ULL;
        if (sub == 0 && k < 25984ULL) {
            unsigned y = 1950 + (unsigned)(k % 77ULL);
            unsigned m = 1 + (unsigned)((k / 77ULL) % 12ULL);
            unsigned d = 1 + (unsigned)((k / (77ULL * 12ULL)) % 28ULL);
            snprintf(out, 256, "%04u%02u%02u", y, m, d);
        } else if (sub == 1) {
            snprintf(out, 256, "%s%08llu", LADA[k % NLADA], (k / NLADA) % 100000000ULL);
        } else {
            snprintf(out, 256, "%llu", k);
        }
    }
    out[255] = 0;
}

unsigned long long gen5_space(int cls) {
    if (cls == 0) return NW * 800ULL;
    if (cls == 1) return NW * NW * 10ULL;
    if (cls == 2) return NW * NW * NW * 6ULL;
    if (cls == 3) return NK * 5ULL;
    if (cls == 5) return NTPL * NNAMES * 80ULL;
    return 0ULL; /* clase 4: infinita */
}

void phrase_key(const char* phrase, unsigned char key[32]) {
    SHA256((const unsigned char*)phrase, strlen(phrase), key);
}
