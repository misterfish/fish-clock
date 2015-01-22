#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

// also in util XX
#include <assert.h>

#include "mylib/xs_render.h"

MODULE = fish_clock_xs             PACKAGE = fish_clock_xs

void
render_init( config )
    HV* config
    CODE:
        xs_render_init(config);

void
render_config(config)
    HV * config
    CODE:
        xs_render_config(config);

void
render_render(cr_sv)
    SV * cr_sv
    CODE:
        int DEBUG = 0;

        // begin work
        ENTER;
        SAVETMPS;

        // cr_sv = SvREFCNT_inc(cr_sv);
        if (DEBUG) printf("is SvROK: %d\n", SvROK(cr_sv));

        // $cr: contains reference to iv, which is the pointer to the Cairo
        // object.
        SV* rv = SvRV(cr_sv);
        IV inner = SvIV(rv);

        if (DEBUG) {
            UV uvrv = PTR2UV(rv);
            printf("rv is %"UVuf", %"UVxf"\n", uvrv, uvrv);
            svtype typ = SvTYPE(rv);
            printf("type is %d\n", typ);  // pvmg type
            printf("IV inner is %"IVdf"\n", inner);
        }
        
        void* cr = (void*) INT2PTR(void*, inner);

        // done
        FREETMPS;
        LEAVE;

        xs_render_render(cr);





