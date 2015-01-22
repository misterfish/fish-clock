/* 
 * This is compiled as 'myextlib' libfish_clock_xs.so.
 */

// for PI
#define _GNU_SOURCE 1

#include <stdio.h>

#include <cairo.h>
#include <math.h>

#include <sys/timeb.h>

#ifndef __FISH_UTIL_H
 #include "fish-util.h"
#endif 

#ifndef __FISH_xsH_H
 #include "xsH.h"
#endif

int is_multiple(int a, int factor);
double to_rad(double deg);
double get_time();
const long double PI = M_PIl;

struct {
    int init;

    /* thickness */
    float th_circle; float th_ticks; float th_hands;
    /* color fill */
    float fr; float fg; float fb;
    /* color ticks */
    float t1r; float t1g; float t1b;
    float t2r; float t2g; float t2b;
    /* color hands */
    float h1r; float h1g; float h1b;
    float h2r; float h2g; float h2b;
    /* circle 2 x shift, delta radius */
    float c2xs; float c2dr;
    /* lame */
    int num_ticks; int major_ticks_every;
    /* tick lengths, int */
    int tl_major; int tl_minor;
    /* hand lengths, as fraction of total */
    float hl_major; float hl_minor;
    /* delta theta, for making second one */
    float dt_ticks; float dt_hands;

    int hour; int min; int sec;
    /* don't currently change, but maybe later */
    int height; int width;
    float radius; int do_fill;
    /* color circles */
    float r1; float g1; float b1;
    float r2; float g2; float b2;
    float transparency; int zooming;
} g;

const char* INFO_HDR;
const char* WARN_HDR;

void error(const char *s);

void _warn(const char *w) { 
    const char* i = WARN_HDR ? WARN_HDR : "";
    warnf("%s %s", WARN_HDR, w); 
}
void _info(const char *w) { 
    const char* i = INFO_HDR ? INFO_HDR : "";
    infof("%s %s", i, w); 
}

void
xs_render_init(
        HV* config
)
{
    INFO_HDR = "  xs_render:";
    WARN_HDR = "  xs_render:";
    
    xsH_hv_iter_init

    //double t1 = get_time();

    while (xsH_hv_iter_while) {
        xsH_hv_iter_getdouble("th_circle", g.th_circle)
        xsH_hv_iter_getdouble("th_ticks", g.th_ticks)
        xsH_hv_iter_getdouble("th_hands", g.th_hands)
        xsH_hv_iter_getdouble("fr", g.fr)
        xsH_hv_iter_getdouble("fg", g.fg)
        xsH_hv_iter_getdouble("fb", g.fb)

        xsH_hv_iter_getdouble("t1r", g.t1r)
        xsH_hv_iter_getdouble("t1g", g.t1g)
        xsH_hv_iter_getdouble("t1b", g.t1b)
        xsH_hv_iter_getdouble("t2r", g.t2r)
        xsH_hv_iter_getdouble("t2g", g.t2g)
        xsH_hv_iter_getdouble("t2b", g.t2b)

        xsH_hv_iter_getdouble("h1r", g.h1r)
        xsH_hv_iter_getdouble("h1g", g.h1g)
        xsH_hv_iter_getdouble("h1b", g.h1b)
        xsH_hv_iter_getdouble("h2r", g.h2r)
        xsH_hv_iter_getdouble("h2g", g.h2g)
        xsH_hv_iter_getdouble("h2b", g.h2b)

        xsH_hv_iter_getdouble("circle_2_x_shift", g.c2xs)
        xsH_hv_iter_getdouble("circle_2_delta_radius", g.c2dr)
        xsH_hv_iter_getint("num_ticks", g.num_ticks)
        xsH_hv_iter_getint("major_ticks_every", g.major_ticks_every)

        xsH_hv_iter_getint("tl_major", g.tl_major)
        xsH_hv_iter_getint("tl_minor", g.tl_minor)
        xsH_hv_iter_getdouble("hl_major", g.hl_major)
        xsH_hv_iter_getdouble("hl_minor", g.hl_minor)
        xsH_hv_iter_getdouble("dt_ticks", g.dt_ticks)
        xsH_hv_iter_getdouble("dt_hands", g.dt_hands)
    }

    g.hour = -1;
    g.min = -1;
    g.sec = -1;

    g.init = 1;
}

void
xs_render_config(
        HV* config
)
{
    if (g.init != 1) {
        _warn("Call init first.");
        return;
    }

    xsH_hv_iter_init

    while (xsH_hv_iter_while) {
        xsH_hv_iter_getint("hour", g.hour)
        xsH_hv_iter_getint("min", g.min)
        xsH_hv_iter_getint("sec", g.sec)

        xsH_hv_iter_getint("height", g.height)
        xsH_hv_iter_getint("width", g.width)
        xsH_hv_iter_getdouble("radius", g.radius)
        xsH_hv_iter_getint("do_fill", g.do_fill)
        xsH_hv_iter_getdouble("r1", g.r1)
        xsH_hv_iter_getdouble("g1", g.g1)
        xsH_hv_iter_getdouble("b1", g.b1)
        xsH_hv_iter_getdouble("r2", g.r2)
        xsH_hv_iter_getdouble("g2", g.g2)
        xsH_hv_iter_getdouble("b2", g.b2)
        xsH_hv_iter_getdouble("transparency", g.transparency)
        xsH_hv_iter_getint("zooming", g.zooming)
    }
}

void
xs_render_render(void* cr_void)
{
    if (!g.init) {
        _warn("Call init first.");
        return;
    }

    if (g.hour == -1 || g.min == -1 || g.sec == -1) return;

    cairo_t* cr = (cairo_t*) cr_void;

    cairo_set_line_width(cr, g.th_circle);

    int xc = (int)(g.width / 2);
    int yc = (int)(g.height / 2);

    cairo_set_source_rgba(cr, g.r1, g.g1, g.b1, g.transparency);
    cairo_arc(cr, xc, yc, g.radius, 0, 2*PI);

    if (g.do_fill) cairo_stroke_preserve(cr); 
    else cairo_stroke(cr);

    cairo_set_source_rgba(cr, g.r2, g.g2, g.b2, g.transparency);
    cairo_arc(cr, xc + g.c2xs, yc, g.radius + g.c2dr, 0, 2*PI);

    if (g.do_fill) {
        cairo_stroke_preserve(cr);  // preserve to not delete path.
        cairo_set_source_rgba(cr, g.fr, g.fg, g.fb, g.transparency);
        cairo_fill(cr); // no preserve, not necessary, and color will get affected.
    }
    else cairo_stroke(cr);

    // Ticks
    if (! g.zooming) {

        cairo_set_line_width(cr, g.th_ticks);

        int i = 0;
        float degrees_per_tick = 360.0 / (g.num_ticks+1);
        for (; i <= g.num_ticks; i++) {
            
            float _theta = to_rad(degrees_per_tick) * i;

            int j = 0;
            for (; j < 2; j++) {

                float theta;

                if (j==0) {
                    cairo_set_source_rgba(cr, g.t1r, g.t1g, g.t1b, g.transparency);
                    theta = _theta;
                }
                else {
                    cairo_set_source_rgba(cr, g.t2r, g.t2g, g.t2b, g.transparency);
                    theta = _theta + to_rad(g.dt_ticks);
                }

                int o_clock = i;
                if (o_clock==0) o_clock = (g.num_ticks+1);

                int tick_length = is_multiple(o_clock, g.major_ticks_every) ? 
                    g.tl_major : 
                    g.tl_minor ;

                int end_x = xc + g.radius * cos(theta);
                int end_y = yc - g.radius * sin(theta);

                int start_x = end_x - tick_length * cos(theta);
                int start_y = end_y + tick_length * sin(theta);

                cairo_move_to(cr, start_x, start_y);
                cairo_line_to(cr, end_x, end_y);
                cairo_stroke(cr);
            }
        }
    }

    // Hands.

    cairo_set_line_width(cr, g.th_hands);
    
    float hour_as_fraction = g.hour + g.min / 60.0;
    float min_as_fraction = g.min + g.sec / 60.0;

    float theta_hr = to_rad(90) - to_rad(30) * hour_as_fraction;
    float theta_min = to_rad(90) - to_rad(30) * min_as_fraction / 5; // i.e. 60 / 12 = 5

    float start_x = xc;
    float start_y = yc;

    /* Light source: top (pi/2).
     */
    float delta_theta_hr = -1 * cos(theta_hr) * to_rad(g.dt_hands);
    float delta_theta_min = -1 * cos(theta_min) * to_rad(g.dt_hands);
    int i = 0;
    for (; i < 4; i++) {
        float theta;
        float length;
        if (i==0) {
            // Hour, shadow.
            cairo_set_source_rgba (cr, g.h1r, g.h1g, g.h1b, g.transparency);
            theta = theta_hr + delta_theta_hr;
            length = g.hl_major;
        }
        else if (i==1) {
            // Min, shadow.
            theta = theta_min + delta_theta_min;
            length = g.hl_minor;
        }
        else if (i==2) {
            // Hour, main.
            cairo_set_source_rgba (cr, g.h2r, g.h2g, g.h2b, g.transparency);
            theta = theta_hr;
            length = g.hl_major;
        }
        else if (i==3) {
            // Min, main.
            theta = theta_min;
            length = g.hl_minor;
        }

        int end_x_extended = xc + g.radius * cos(theta);
        int end_y_extended = yc - g.radius * sin(theta);

        int end_x = start_x + (end_x_extended - start_x) * length;
        int end_y = start_y + (end_y_extended - start_y) * length;

        cairo_move_to(cr, xc, yc);
        cairo_line_to(cr, end_x, end_y);
        cairo_stroke(cr);
    }
}

int is_multiple(int a, int factor) {
    return ! (a % factor);
}

double to_rad(double deg) {
    return deg*(PI/180.0);
}

double get_time() {
    struct timeb t;
    memset(&t, 0, sizeof(t));
    ftime(&t);

    time_t time = t.time;
    unsigned short millitm = t.millitm;

    return time + millitm / 1000.0;
}
