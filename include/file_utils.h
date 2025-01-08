#include "ringing_utils.h"
#include <libxml/parser.h>
#include <string>
#include <list>

using std::string;
using namespace ringing_utils;

namespace file_utils {

class MCFFile {};


// Method Library searching, retrieval, etc
const string ROOT_LIBRARY_FILEPATH = "/home/khawkins/projects/ringing/ringing_simulator/data/CCCBR_methods.xml";
const string CCCBR_LIBRARY_URL = "https://methods.cccbr.org.uk/xml/CCCBR_methods.xml.zip";
const string CCCBR_JUMPLIB_URL = "https://methods.cccbr.org.uk/xml/CCCBR_Jump.xml.zip";

struct MethodSearchResult {
    const char* id;
    const char* name;
};

class MethodLibrary {
    xmlDocPtr library_ptr;

    public:
        MethodLibrary(string library_path);
        ~MethodLibrary();

        string RetrieveLibraryDate();

        /**
         * @brief Returns basic identifying information about methods from the library matching the search parameters.
         * 
         * @param partial_method_name A lower-case, escaped string to search for in method names, default "" (no filtering).
         * @param stage Integer method stage, default 0 (no filtering).
         * @param method_class Enum type indicating the method class, default NONE (no filtering).
         * @return MethodSearchResult*
         */
        std::list<MethodSearchResult> SearchLibrary(string partial_method_name="", int stage=0, MethodClass method_class=NONE);
        Method RetrieveMethodInformation(string method_id);

        static void UpdateLibrary(string file_location, string temp_location="");
};

class MethodLibraryLoadError {};
class MethodLibrarySearchError {};
class MethodLibraryUpdateError {};

}