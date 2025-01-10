#include <cstring>
#include "ringing_utils.h"

namespace ringing_utils {

// default constructor gives an empty row
Row::Row() {
    num_bells = 0;
    bell_positions = nullptr;
}

// maximum number of bells is 33 due to representation restrictions
Row::Row(int num_bells, string bells) {
    this->num_bells = num_bells;
    this->bell_positions = (char*) malloc(sizeof(char) * num_bells);
    if (!bells.empty()) {
        SetBells(bells);
    }
}

Row::Row(const Row& other) {
    num_bells = other.num_bells;
    bell_positions = (char*) malloc(sizeof(char) * num_bells);
    memcpy(bell_positions, other.bell_positions, sizeof(char)*num_bells);
}

Row& Row::operator=(const Row& other) {
    if (this != &other) {
        num_bells = other.num_bells;
        free(bell_positions);
        bell_positions = (char*) malloc(sizeof(char) * num_bells);
        memcpy(bell_positions, other.bell_positions, sizeof(char)*num_bells);
    }
    return *this;
}

Row::~Row() {
    free(this->bell_positions);
}

void Row::SetBells(string bells) {
    //TODO: actually check that the bell sequence is valid
    if (num_bells == bells.length()) {
        for (int i=0; i<num_bells; i++) {
            bell_positions[i] = bells[i];
        }
    }
}

Row Row::Rounds(int num_bells) {
    Row to_return = Row(num_bells);
    // populate row with rounds
    for (int i=0; i<num_bells; i++) {
        to_return.bell_positions[i] = IntToBellSymbol(i+1);
    }
    return to_return;
}

Row Row::ApplyChange(Change change) {
    if (change.stage <= num_bells) {
        // copy the current row
        Row to_return = Row(*this);
        // permute the row according to the change
        for (int i=0; i<change.stage; i++) {
            to_return.bell_positions[i] = this->bell_positions[change.transposition[i]];
        }
        return to_return;
    }
    else {
        fprintf(stderr, "Stage of change too high for this row.\n");
        throw;
    }
}

string Row::ToString() {
    string r = "";
    for (int i=0; i<num_bells; i++) {
        r += bell_positions[i];
    }
    return r;
}

}