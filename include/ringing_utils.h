#include <string>
#include <vector>

using std::string;
namespace ringing_utils {

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


struct Row
{
    int num_bells;
    char* bell_positions;

    // constructors and destructors
    Row(); // do not use, gives empty row of length 0
    Row(int stage, string bells="");
    Row(const Row& other);
    Row& operator=(const Row& other);
    ~Row();

    void SetBells(string bells);

    // special constructor for rounds
    static Row Rounds(int num_bells);

    // apply change to get new row
    Row ApplyChange(Change change);

    // for convenience of printing
    string ToString();
};


enum MethodClass {
    NONE=0, PLACE, BOB, SLOW_COURSE, TREBLE_BOB, DELIGHT, SURPRISE, ALLIANCE, TREBLE_PLACE, HYBRID, UNCLASSED
};
string MethodClassToString(MethodClass m);
MethodClass StringToMethodClass(string classification);

int BellSymbolToInt(char bell_symbol);
char IntToBellSymbol(int num_bells);
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
    Row lead_head;
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