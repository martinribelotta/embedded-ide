#include "makefileinfo.h"

class MakefileInfo_metatypeHelper_t {
public:
    MakefileInfo_metatypeHelper_t() {
        qRegisterMetaType<MakefileInfo>();
    }
};

static MakefileInfo_metatypeHelper_t MakefileInfo_metatypeHelper;
