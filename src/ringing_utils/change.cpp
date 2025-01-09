#include <cstring>
#include "ringing_utils.h"

namespace ringing_utils {

Change::Change(int stage, bool jump) {
    this->stage = stage;
    this->jump = jump;
    this->transposition = (int*) malloc(sizeof(int)*stage);
}

Change::Change(const Change& other) {
    stage = other.stage;
    jump = other.jump;
    transposition = (int*) malloc(sizeof(int)*stage);
    memcpy(transposition, other.transposition, sizeof(int)*stage);
}

Change& Change::operator=(const Change& other) {
    if (this != &other) {
        stage = other.stage;
        jump = other.jump;
        delete transposition;
        transposition = (int*) malloc(sizeof(int)*stage);
        memcpy(transposition, other.transposition, sizeof(int)*stage);
    }
    return *this;
}

Change::~Change() {
    delete(transposition);
}

Change Change::Inverse() {
    if (this->jump) {
        Change to_return(this->stage, true);
        for (int i=0; i<this->stage; i++) {
            for (int j=0; j<this->stage; j++) {
                if (this->transposition[j] == i) {
                    to_return.transposition[i] = j;
                    break;
                }
            }
        }
        return to_return;
    }
    else {
        return Change(*this);
    }
}

}