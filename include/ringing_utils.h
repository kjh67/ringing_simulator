#include <string>
#include <vector>

using std::string;
namespace ringing_utils {

class Row
{
    private:
        /* data */
    public:
        /* methods */
};


struct Change
{
    int stage;
    bool jump;
    int* transposition;

    // constructors and destructors
    Change(int stage, bool jump=false);
    Change(const Change& other);
    Change& operator=(const Change& other);
    ~Change();

    Change Inverse();
};


enum MethodClass {
    NONE=0, PLACE, BOB, SLOW_COURSE, TREBLE_BOB, DELIGHT, SURPRISE, ALLIANCE, TREBLE_PLACE, HYBRID, UNCLASSED
};
string MethodClassToString(MethodClass m);
MethodClass StringToMethodClass(string classification);

int BellSymbolToInt(char bell_symbol);
class PlaceNotationError {};

struct Method
{
    string title;
    string id;
    int stage;

    MethodClass classification;
    bool little;
    bool differential;
    bool plain;
    bool treble_dodging;
    
    string place_notation;

    int lead_length;
    int num_hunt_bells;
    //TODO: change from string to Row once Row has been implemented
    string lead_head;
    string lead_head_code;

    std::vector<Change> getChanges();

    static std::vector<Change> PlaceNotationToChanges(string notation, int stage);

    private: std::vector<Change> changes = {};
};


class Composition
{
    public:
        //methods
};


class Touch
{
    public:
        //methods
        Row next_row();
};

}