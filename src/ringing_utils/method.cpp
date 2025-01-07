#include <string>
#include <cstring>
#include "ringing_utils.h"

using std::string;
namespace ringing_utils {

string MethodClassToString(MethodClass m) {
    switch (m)
    {
    case PLACE:
        return "Place";
    case BOB:
        return "Bob";
    case SLOW_COURSE:
        return "Slow Course";
    case TREBLE_BOB:
        return "Treble Bob";
    case DELIGHT:
        return "Delight";
    case SURPRISE:
        return "Surprise";
    case ALLIANCE:
        return "Alliance";
    case TREBLE_PLACE:
        return "Treble Place";
    case HYBRID:
        return "Hybrid";
    case UNCLASSED:
        return "";
    default:
        return "";
    };
}

MethodClass StringToMethodClass(string classification) {
    if (strcmp(classification.c_str(), "Place") == 0) {
        return PLACE;
    }
    else if (strcmp(classification.c_str(), "Bob") == 0) {
        return BOB;
    }
    else if (strcmp(classification.c_str(), "Slow Course") == 0) {
        return SLOW_COURSE;
    }
    else if (strcmp(classification.c_str(), "Treble Bob") == 0) {
        return TREBLE_BOB;
    }
    else if (strcmp(classification.c_str(), "Delight") == 0) {
        return DELIGHT;
    }
    else if (strcmp(classification.c_str(), "Surprise") == 0) {
        return SURPRISE;
    }
    else if (strcmp(classification.c_str(), "Alliance") == 0) {
        return ALLIANCE;
    }
    else if (strcmp(classification.c_str(), "Treble Place") == 0) {
        return TREBLE_PLACE;
    }
    else if (strcmp(classification.c_str(), "Hybrid") == 0) {
        return HYBRID;
    }
    else {
        return UNCLASSED;
    }
}

}
