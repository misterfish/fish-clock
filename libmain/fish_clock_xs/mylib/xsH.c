#ifndef __FISH_xsH_H
 #include "xsH.h"
#endif

int hv_iter(HV* hv, char** key_ptr, int* type_ptr, int* val_i_ptr, double* val_d_ptr, char** val_s_ptr) {

    int DEBUG = 0;
    I32 key_len;
    SV* val = hv_iternextsv(hv, key_ptr, &key_len);
    static SV* last_val;
    if (DEBUG) printf("Looking at key %s len %d\n", *key_ptr, key_len);
    if (val == NULL) return 0; // doesn't happen?
    if (val == last_val) return 0;
    last_val = val;
    if (SvIOK(val)) {
        IV ival = SvIV(val);
        if (DEBUG) printf("val is %d\n", ival);
        *val_i_ptr = (int) ival;
        *type_ptr = 1;
    }
    else if (SvNOK(val)) {
        double dval = SvNV(val);
        if (DEBUG) printf("val is %f\n", dval);
        *val_d_ptr = dval;
        *type_ptr = 2;
    }
    else if (SvPOK(val)) {
        int len;
        char* sval = SvPV(val, len); // macro, dus len not &len
        len++;
        if (DEBUG) printf("len is %d\n", len);
        char* new = malloc(len * sizeof(char));
        strncpy(new, sval, len);
        if (DEBUG) printf("orig is %s$\n", sval);
        if (DEBUG) printf("new is %s$\n", new);
        *val_s_ptr = new;
        *type_ptr = 3;
    }
    return 1;
}


