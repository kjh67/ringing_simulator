#ifndef RINGUTIL_H
#define RINGUTIL_H

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


enum CompositionType { STANDARD=0, RANDOM }; // add support for random later

struct CompositionComponent { 
    Change change;
    string call; 

    CompositionComponent(Change c): change(c) {};    
};

struct Composition
{
    CompositionType composition_type = STANDARD;
    bool backstroke_start = false;

    // no default constructor
    // how to make call-place comps method agnostic? store a comp type and
    // then allow a method replacement?
    // STORE the compositions in a way which allows the methods to be switched when
    // the composition is properly 'loaded'

    std::vector<CompositionComponent> changes;
    int number_of_changes;
    int stage; // max stage used during composition

    // special constructors for plain course and plain lead of a method
    static Composition PlainLead(Method method);
    static Composition PlainCourse(Method method);
};

}

#endif