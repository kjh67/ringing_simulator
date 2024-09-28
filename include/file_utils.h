#include "ringing_utils.h"
#include <libxml2/libxml/xmlversion.h>
#include <libxml2/libxml/xmlreader.h>
#include <string>

using std::string;
using namespace ringing_utils;

namespace file_utils {

class MCFFile {};

class MethodLibrary {

    MethodLibrary(string file_location);

    Method* SearchLibrary(string method_name, Method* search_subset = nullptr);

};

}