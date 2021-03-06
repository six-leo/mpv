/*
 * This file is part of mpv.
 *
 * mpv is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * mpv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with mpv.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MP_GL_VIDEO_H
#define MP_GL_VIDEO_H

#include <stdbool.h>

#include "options/m_option.h"
#include "sub/osd.h"
#include "utils.h"
#include "lcms.h"
#include "shader_cache.h"
#include "video/csputils.h"
#include "video/out/filter_kernels.h"

// Assume we have this many texture units for sourcing additional passes.
// The actual texture unit assignment is dynamic.
#define TEXUNIT_VIDEO_NUM 6

struct scaler_fun {
    char *name;
    float params[2];
    float blur;
    float taper;
};

struct scaler_config {
    struct scaler_fun kernel;
    struct scaler_fun window;
    float radius;
    float antiring;
    float cutoff;
    float clamp;
};

struct scaler {
    int index;
    struct scaler_config conf;
    double scale_factor;
    bool initialized;
    struct filter_kernel *kernel;
    struct ra_tex *lut;
    struct fbotex sep_fbo;
    bool insufficient;
    int lut_size;

    // kernel points here
    struct filter_kernel kernel_storage;
};

enum scaler_unit {
    SCALER_SCALE,  // luma/video
    SCALER_DSCALE, // luma-video downscaling
    SCALER_CSCALE, // chroma upscaling
    SCALER_TSCALE, // temporal scaling (interpolation)
    SCALER_COUNT
};

enum dither_algo {
    DITHER_NONE = 0,
    DITHER_FRUIT,
    DITHER_ORDERED,
};

enum alpha_mode {
    ALPHA_NO = 0,
    ALPHA_YES,
    ALPHA_BLEND,
    ALPHA_BLEND_TILES,
};

enum blend_subs_mode {
    BLEND_SUBS_NO = 0,
    BLEND_SUBS_YES,
    BLEND_SUBS_VIDEO,
};

enum tone_mapping {
    TONE_MAPPING_CLIP,
    TONE_MAPPING_MOBIUS,
    TONE_MAPPING_REINHARD,
    TONE_MAPPING_HABLE,
    TONE_MAPPING_GAMMA,
    TONE_MAPPING_LINEAR,
};

// How many frames to average over for HDR peak detection
#define PEAK_DETECT_FRAMES 100

struct gl_video_opts {
    int dumb_mode;
    struct scaler_config scaler[4];
    int scaler_lut_size;
    float gamma;
    int gamma_auto;
    int target_prim;
    int target_trc;
    int target_brightness;
    int tone_mapping;
    int compute_hdr_peak;
    float tone_mapping_param;
    float tone_mapping_desat;
    int linear_scaling;
    int correct_downscaling;
    int sigmoid_upscaling;
    float sigmoid_center;
    float sigmoid_slope;
    int scaler_resizes_only;
    int pbo;
    int dither_depth;
    int dither_algo;
    int dither_size;
    int temporal_dither;
    int temporal_dither_period;
    char *fbo_format;
    int alpha_mode;
    int use_rectangle;
    struct m_color background;
    int interpolation;
    float interpolation_threshold;
    int blend_subs;
    char **user_shaders;
    int deband;
    struct deband_opts *deband_opts;
    float unsharp;
    int tex_pad_x, tex_pad_y;
    struct mp_icc_opts *icc_opts;
    int early_flush;
    char *shader_cache_dir;
};

extern const struct m_sub_options gl_video_conf;

struct gl_video;
struct vo_frame;

struct gl_video *gl_video_init(struct ra *ra, struct mp_log *log,
                               struct mpv_global *g);
void gl_video_uninit(struct gl_video *p);
void gl_video_set_osd_source(struct gl_video *p, struct osd_state *osd);
void gl_video_update_options(struct gl_video *p);
bool gl_video_check_format(struct gl_video *p, int mp_format);
void gl_video_config(struct gl_video *p, struct mp_image_params *params);
void gl_video_set_output_depth(struct gl_video *p, int r, int g, int b);
void gl_video_render_frame(struct gl_video *p, struct vo_frame *frame,
                           struct fbodst target);
void gl_video_resize(struct gl_video *p,
                     struct mp_rect *src, struct mp_rect *dst,
                     struct mp_osd_res *osd);
void gl_video_set_fb_depth(struct gl_video *p, int fb_depth);
struct voctrl_performance_data;
void gl_video_perfdata(struct gl_video *p, struct voctrl_performance_data *out);
struct mp_csp_equalizer;
struct mp_csp_equalizer *gl_video_eq_ptr(struct gl_video *p);
void gl_video_eq_update(struct gl_video *p);
void gl_video_set_clear_color(struct gl_video *p, struct m_color color);
void gl_video_set_osd_pts(struct gl_video *p, double pts);
bool gl_video_check_osd_change(struct gl_video *p, struct mp_osd_res *osd,
                               double pts);

float gl_video_scale_ambient_lux(float lmin, float lmax,
                                 float rmin, float rmax, float lux);
void gl_video_set_ambient_lux(struct gl_video *p, int lux);
void gl_video_set_icc_profile(struct gl_video *p, bstr icc_data);
bool gl_video_icc_auto_enabled(struct gl_video *p);
bool gl_video_gamma_auto_enabled(struct gl_video *p);
struct mp_colorspace gl_video_get_output_colorspace(struct gl_video *p);

void gl_video_reset(struct gl_video *p);
bool gl_video_showing_interpolated_frame(struct gl_video *p);

struct ra_hwdec;
void gl_video_set_hwdec(struct gl_video *p, struct ra_hwdec *hwdec);

struct vo;
void gl_video_configure_queue(struct gl_video *p, struct vo *vo);

struct mp_image *gl_video_get_image(struct gl_video *p, int imgfmt, int w, int h,
                                    int stride_align);


#endif
