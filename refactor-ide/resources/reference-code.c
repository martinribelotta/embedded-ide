#include <stdio.h>
#include "sys/sct.h"

typedef int (*callback_t)(void *ptr, float h);

enum type_t {
    type1, type2, type3=3
};

struct mystruct {
    enum type_t type;
    union {
        int v_int;
        char v_char;
        long v_long:
        short v_short;
        void *v_ptr;
        float v_float;
        double v_double;
    } value;
};

int main(int argc, char *argv[]) {
    if (argc > 0) {
        int i;
        for (i=0; i<argc; i++) {
            switch(argv[i][0]) {
            case 'a': return -1; break;
            case 'c': return -1; break;
            default: return -1; break;
            }
        }
    }
    return 0;
}
