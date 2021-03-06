#pragma once

#include <stdbool.h>
#include <math.h>

#include "ra.h"

// A 3x2 matrix, with the translation part separate.
struct gl_transform {
    // row-major, e.g. in mathematical notation:
    //  | m[0][0] m[0][1] |
    //  | m[1][0] m[1][1] |
    float m[2][2];
    float t[2];
};

static const struct gl_transform identity_trans = {
    .m = {{1.0, 0.0}, {0.0, 1.0}},
    .t = {0.0, 0.0},
};

void gl_transform_ortho(struct gl_transform *t, float x0, float x1,
                        float y0, float y1);

// This treats m as an affine transformation, in other words m[2][n] gets
// added to the output.
static inline void gl_transform_vec(struct gl_transform t, float *x, float *y)
{
    float vx = *x, vy = *y;
    *x = vx * t.m[0][0] + vy * t.m[0][1] + t.t[0];
    *y = vx * t.m[1][0] + vy * t.m[1][1] + t.t[1];
}

struct mp_rect_f {
    float x0, y0, x1, y1;
};

// Semantic equality (fuzzy comparison)
static inline bool mp_rect_f_seq(struct mp_rect_f a, struct mp_rect_f b)
{
    return fabs(a.x0 - b.x0) < 1e-6 && fabs(a.x1 - b.x1) < 1e-6 &&
           fabs(a.y0 - b.y0) < 1e-6 && fabs(a.y1 - b.y1) < 1e-6;
}

static inline void gl_transform_rect(struct gl_transform t, struct mp_rect_f *r)
{
    gl_transform_vec(t, &r->x0, &r->y0);
    gl_transform_vec(t, &r->x1, &r->y1);
}

static inline bool gl_transform_eq(struct gl_transform a, struct gl_transform b)
{
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            if (a.m[x][y] != b.m[x][y])
                return false;
        }
    }

    return a.t[0] == b.t[0] && a.t[1] == b.t[1];
}

void gl_transform_trans(struct gl_transform t, struct gl_transform *x);

struct fbodst {
    struct ra_tex *tex;
    bool flip; // mirror vertically
};

void gl_transform_ortho_fbodst(struct gl_transform *t, struct fbodst fbo);

struct fbotex {
    struct ra *ra;
    struct ra_tex *tex;
    int lw, lh; // logical (configured) size, <= than texture size
    struct fbodst fbo;
};

void fbotex_uninit(struct fbotex *fbo);
bool fbotex_change(struct fbotex *fbo, struct ra *ra, struct mp_log *log,
                   int w, int h, const struct ra_format *fmt, int flags);
#define FBOTEX_FUZZY_W 1
#define FBOTEX_FUZZY_H 2
#define FBOTEX_FUZZY (FBOTEX_FUZZY_W | FBOTEX_FUZZY_H)

#define NUM_PBO_BUFFERS 3

// A wrapper around tex_upload that uses PBOs internally if requested or
// required
struct tex_upload {
    size_t buffer_size;
    struct ra_buf *buffers[NUM_PBO_BUFFERS];
    int index;
};

bool tex_upload(struct ra *ra, struct tex_upload *pbo, bool want_pbo,
                const struct ra_tex_upload_params *params);

void tex_upload_uninit(struct ra *ra, struct tex_upload *pbo);

// A wrapper around ra_timer that does result pooling, averaging etc.
struct timer_pool;

struct timer_pool *timer_pool_create(struct ra *ra);
void timer_pool_destroy(struct timer_pool *pool);
void timer_pool_start(struct timer_pool *pool);
void timer_pool_stop(struct timer_pool *pool);
struct mp_pass_perf timer_pool_measure(struct timer_pool *pool);

// print a multi line string with line numbers (e.g. for shader sources)
// log, lev: module and log level, as in mp_msg()
void mp_log_source(struct mp_log *log, int lev, const char *src);
