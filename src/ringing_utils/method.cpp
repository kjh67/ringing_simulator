#include <cstring>
#include "ringing_utils.h"


int get_place(const char* notation, int* current_pointer) {
    char next_char = notation[*current_pointer];
    if (next_char == '{') {
        string num = "";
        (*current_pointer)++; next_char = notation[*current_pointer];
        // accumulate chars until the closing brackets
        while (next_char != '}') {
            num += next_char;
            (*current_pointer)++; next_char = notation[*current_pointer];
        }
        return std::stoi(num.c_str()) - 1;
    }
    else {
        return ringing_utils::BellSymbolToInt(next_char) - 1;
    }
}


namespace ringing_utils {

/**
 * @brief Returns a std::list of changes corresponding to the provided place notation.
 * 
 * @param place_notation Assumed to specify ALL places (including external), and to be otherwise valid place notation.
 * @param stage Int representing the number of bells affected by the notation.
 * @return std::list<Change> 
 */
std::vector<Change> Method::PlaceNotationToChanges(string place_notation, int stage) {
    const char* notation = place_notation.c_str();
    int length = place_notation.length();

    // step one: parse the place notation out into per-change pieces
    // step two: get the int representations of the bell symbols
    // step three: create corresponding Change
    //      (potentially cache these to avoid needless recalculating, as a future optimisation)
    bool palindrome = false;
    int palindrome_start;

    int num_changes = 0;
    std::vector<Change> changes = {};
    int i = 0;
    while (i < length) {
        if (notation[i]=='-'||notation[i]=='x'||notation[i]=='X') {
            // cross change; swap all pairs; assume even stage
            Change* change = new Change(stage);
            int j = 0;
            while (j < stage) {
                change->transposition[j] = j+1;
                change->transposition[j+1] = j;
                j += 2;
            }
            i++; num_changes++;
            changes.push_back(*change);
        }
        else if (notation[i]==',') {
            // palindrome: reflect all the notation so far, and set a flag to do the same at the
            // end, starting from the next change (save in palindrome_start)
            palindrome = true;
            for (int j=num_changes-2; j>=0; j--) {
                changes.push_back(changes[j].Inverse());
                num_changes++;
            }
            palindrome_start = num_changes;
            i++;
        }
        else if (notation[i]=='.') {
            // end marker for previous change, just move on
            i++;
        }
        else {
            // generate a Change from the next sequence of bell symbols
            // assume that ALL places are specified (external as well as internal)
            Change* change = new Change(stage);
            int last_place = -1;
            int current_place;
            char next_char = notation[i];
            while (i < length && next_char!='.' && next_char!='-' && next_char!='x' && next_char!='X' && next_char!=',') {
                if (next_char == '(' || next_char == '[') {
                    // handle jump changes
                    change->jump = true;
                    int lowest_affected = __INT_MAX__;
                    int highest_affected = -1;

                    if (next_char == '(') {
                        // single jump notation
                        i++;
                        int swap[2];
                        swap[0] = get_place(notation, &i);
                        i++;
                        swap[1] = get_place(notation, &i);
                        i++;
                        next_char = notation[i];
                        if (next_char != ')') {
                            fprintf(stderr, "incorrect use of () notation\n");
                            throw PlaceNotationError();
                        }
                        if (swap[0] < lowest_affected) { lowest_affected = swap[0]; }
                        if (swap[0] > highest_affected) { highest_affected = swap[0]; }
                        if (swap[1] < lowest_affected) {lowest_affected = swap[1]; }
                        if (swap[1] > highest_affected) { highest_affected = swap[1]; }

                        // bell in p1 will jump to p2; prepopulate transp with -1 in affected range
                        for (int j=lowest_affected; j<=highest_affected; j++) {
                            change->transposition[j] = -1;
                        }
                        change->transposition[swap[1]] = swap[0];
                        int shifted = (swap[0] < swap[1]) ? lowest_affected + 1 : lowest_affected;
                        for (int j=lowest_affected; j<=highest_affected; j++) {
                            if (change->transposition[j] == -1) {
                                change->transposition[j] = shifted;
                                shifted++;
                            }
                        }

                    }
                    else {
                        // full specification notation
                        std::vector<int> positions = {};
                        i++; next_char = notation[i];
                        while (next_char != ']') {
                            int pos = get_place(notation, &i);
                            if (pos < lowest_affected) { lowest_affected = pos; }
                            if (pos > highest_affected) { highest_affected = pos; }
                            positions.push_back(pos);
                            i++; next_char = notation[i];
                        }
                        // starting from the lowest affected pos, read in the new positions
                        for (int j=lowest_affected; j<=highest_affected; j++) {
                            change->transposition[j] = positions[j-lowest_affected];
                        }
                    }
                    // transpositions for the affected range have been sorted
                    // need to account for any swaps below, then set last_place as appropriate (top of range)
                    if (lowest_affected - last_place > 1) {
                        for (int j=last_place+1; j<lowest_affected; j+=2) {
                            change->transposition[j] = j+1;
                            change->transposition[j+1] = j;
                        }
                    }
                    last_place = highest_affected;
                }
                else {
                    current_place = get_place(notation, &i);
                    // if current place is separated by more than one from the last place, fill with swaps
                    if (current_place - last_place > 1) {
                        // from the last to the current place, do swaps
                        for (int j=last_place+1; j<current_place; j+=2) {
                            change->transposition[j] = j+1;
                            change->transposition[j+1] = j;
                        }
                    }
                    // make the place
                    change->transposition[current_place] = current_place;
                    last_place = current_place;
                }

                // move to the next piece of place notation
                i++; next_char = notation[i];
            }

            // check if swaps need to be made above the last place
            if (last_place < stage-1) {
                for (int j=last_place+1; j<stage; j+=2) {
                    change->transposition[j] = j+1;
                    change->transposition[j+1] = j;
                }
            }

            changes.push_back(*change);
            num_changes++;
        }
    }

    if (palindrome) {
        // palindrom-ise all changes since the palindrome indicator
        for (int j=num_changes-2; j>=palindrome_start; j--) {
            changes.push_back(changes[j].Inverse());
            num_changes++;
        }
    }

    return changes;
}

std::vector<Change> Method::getChanges() {
    if (this->changes.empty()) {
        // generate changes
        this->changes = PlaceNotationToChanges(this->place_notation, this->stage);
        // check that number of changes and lead length agrees
        if (this->changes.size() != this->lead_length) {
            fprintf(stderr, "Mismatch in number of changes generated: %d  vs %d expected.\n", this->changes.size(), this->lead_length);
            throw PlaceNotationError();
        }
    }
    return this->changes;
}


int BellSymbolToInt(char bell_symbol) {
    if (bell_symbol == '1') {
        return 1;
    }
    else if (bell_symbol == '2') {
        return 2;
    }
    else if (bell_symbol == '3') {
        return 3;
    }
    else if (bell_symbol == '4') {
        return 4;
    }
    else if (bell_symbol == '5') {
        return 5;
    }
    else if (bell_symbol == '6') {
        return 6;
    }
    else if (bell_symbol == '7') {
        return 7;
    }
    else if (bell_symbol == '8') {
        return 8;
    }
    else if (bell_symbol == '9') {
        return 9;
    }
    else if (bell_symbol == '0') {
        return 10;
    }
    else if (bell_symbol == 'E' || bell_symbol == 'e') {
        return 11;
    }
    else if (bell_symbol == 'T' || bell_symbol == 't') {
        return 12;
    }
    else if (bell_symbol == 'A' || bell_symbol == 'a') {
        return 13;
    }
    else if (bell_symbol == 'B' || bell_symbol == 'b') {
        return 14;
    }
    else if (bell_symbol == 'C' || bell_symbol == 'c') {
        return 15;
    } 
    else if (bell_symbol == 'D' || bell_symbol == 'd') {
        return 16;
    }
    else if (bell_symbol == 'F' || bell_symbol == 'f') {
        return 17;
    }
    else if (bell_symbol == 'G' || bell_symbol == 'g') {
        return 18;
    }
    else if (bell_symbol == 'H' || bell_symbol == 'h') {
        return 19;
    }
    else if (bell_symbol == 'J' || bell_symbol == 'j') {
        return 20;
    }
    else if (bell_symbol == 'K' || bell_symbol == 'k') {
        return 21;
    }
    else if (bell_symbol == 'L' || bell_symbol == 'l') {
        return 22;
    }
    else if (bell_symbol == 'M' || bell_symbol == 'm') {
        return 23;
    }
    else if (bell_symbol == 'N' || bell_symbol == 'n') {
        return 24;
    }
    else if (bell_symbol == 'P' || bell_symbol == 'p') {
        return 25;
    }
    else if (bell_symbol == 'Q' || bell_symbol == 'q') {
        return 26;
    }
    else if (bell_symbol == 'R' || bell_symbol == 'r') {
        return 27;
    }
    else if (bell_symbol == 'S' || bell_symbol == 's') {
        return 28;
    }
    else if (bell_symbol == 'U' || bell_symbol == 'u') {
        return 29;
    }
    else if (bell_symbol == 'V' || bell_symbol == 'v') {
        return 30;
    }
    else if (bell_symbol == 'W' || bell_symbol == 'w') {
        return 31;
    }
    else if (bell_symbol == 'Y' || bell_symbol == 'y') {
        return 32;
    }
    else if (bell_symbol == 'Z' || bell_symbol == 'z') {
        return 33;
    }
    else {
        fprintf(stderr, "Not a valid bell symbol\n");
        throw PlaceNotationError();
    }
}
char IntToBellSymbol(int bell_number) {
    switch (bell_number)
    {
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    case 10:
        return '0';
    case 11:
        return 'E';
    case 12:
        return 'T';
    case 13:
        return 'A';
    case 14:
        return 'B';
    case 15:
        return 'C';
    case 16:
        return 'D';
    case 17:
        return 'F';
    case 18:
        return 'G';
    case 19:
        return 'H';
    case 20:
        return 'J';
    case 21:
        return 'K';
    case 22:
        return 'L';
    case 23:
        return 'M';
    case 24:
        return 'N';
    case 25:
        return 'P';
    case 26:
        return 'Q';
    case 27:
        return 'R';
    case 28:
        return 'S';
    case 29:
        return 'U';
    case 30:
        return 'V';
    case 31:
        return 'W';
    case 32:
        return 'Y';
    case 33:
        return 'Z';
    default:
        fprintf(stderr, "No representation exists for bell %d\n", bell_number);
        throw;
    }
}

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