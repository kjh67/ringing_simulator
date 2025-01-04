#include "ringing_utils.h"

namespace file_utils {

class MCF_File
{
    int counter;
    int default_method;
    int num_methods;

    public:
        MCF_File(char* path);
        void write_mcf();
};


// constructor, load MCF file from path
MCF_File::MCF_File(char* path) {
    counter = 0;
};

}
