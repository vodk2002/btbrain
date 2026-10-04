/* threads.h SHIM (btcscan): este toolchain WinLibs-micro no trae C11 threads.h.
   Implementa el subconjunto C11 usado por el motor sobre winpthreads.
   Se usa via -I src (los #include <threads.h> lo recogen al no existir el del sistema). */
#ifndef BTSCAN_THREADS_SHIM_H
#define BTSCAN_THREADS_SHIM_H
#include <pthread.h>

typedef pthread_t thrd_t;
typedef int (*thrd_start_t)(void*);
enum { thrd_success = 0, thrd_error = 1, thrd_nomem = 2, thrd_timedout = 3, thrd_busy = 4 };
static inline int thrd_create(thrd_t* t, thrd_start_t f, void* a) {
    return pthread_create(t, NULL, (void*(*)(void*))f, a) == 0 ? thrd_success : thrd_error;
}
static inline int thrd_join(thrd_t t, int* r) {
    void* v = NULL;
    int rc = pthread_join(t, &v);
    if (r) *r = (int)(intptr_t)v;
    return rc == 0 ? thrd_success : thrd_error;
}

enum { mtx_plain = 0, mtx_recursive = 1, mtx_timed = 2 };
typedef pthread_mutex_t mtx_t;
static inline int mtx_init(mtx_t* m, int t) {
    (void)t;
    return pthread_mutex_init(m, NULL) == 0 ? thrd_success : thrd_error;
}
static inline int mtx_lock(mtx_t* m) { return pthread_mutex_lock(m) == 0 ? thrd_success : thrd_error; }
static inline int mtx_unlock(mtx_t* m) { return pthread_mutex_unlock(m) == 0 ? thrd_success : thrd_error; }
static inline void mtx_destroy(mtx_t* m) { pthread_mutex_destroy(m); }

typedef pthread_cond_t cnd_t;
static inline int cnd_init(cnd_t* c) { return pthread_cond_init(c, NULL) == 0 ? thrd_success : thrd_error; }
static inline int cnd_wait(cnd_t* c, mtx_t* m) { return pthread_cond_wait(c, m) == 0 ? thrd_success : thrd_error; }
static inline int cnd_signal(cnd_t* c) { return pthread_cond_signal(c) == 0 ? thrd_success : thrd_error; }
static inline int cnd_broadcast(cnd_t* c) { return pthread_cond_broadcast(c) == 0 ? thrd_success : thrd_error; }
static inline void cnd_destroy(cnd_t* c) { pthread_cond_destroy(c); }

#endif
