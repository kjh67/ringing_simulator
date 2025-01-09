#include <gtest/gtest.h>
#include "ringing_utils.h"

using namespace ringing_utils;

void display_changes(std::vector<Change> changes) {
    for (Change change: changes) {
        for (int i=0; i<change.stage; i++) {
            fprintf(stderr, "%d ", change.transposition[i]);
        }
        fprintf(stderr, "\n");
    }
}

TEST(RINGINGUTILS, METHOD_PLACENOTATIONTEST) {
    // test failure mode: num changes does not match expected
    Method m1;
    m1.place_notation = "3.145.125.123.5.3.5.145.3.125";
    m1.stage = 5;
    m1.lead_length = 11;
    EXPECT_THROW(m1.getChanges(), PlaceNotationError);
    fprintf(stderr,"Threw correctly\n");

    // test normal operation
    Method m;
    m.place_notation = "3.145.125.123.5.3.5.145.3.125";
    m.stage = 5;
    m.lead_length = 10;
    std::vector<Change> changes = m.getChanges();
    display_changes(changes);

    // test extended notation; check that resulting changes are the same as above
    Method m2;
    m2.place_notation = "{3}.{1}{4}{5}.{1}{2}{5}.{1}{2}{3}.{5}.{3}.{5}.{1}{4}{5}.{3}.{1}{2}{5}";
    m2.stage = 5;
    m2.lead_length = 10;
    std::vector<Change> changes_extendednotation = m2.getChanges();
    fprintf(stderr, "\nTransps for extended notation:\n");
    display_changes(changes_extendednotation);

    // test palindrome handling
    string palindrome_notation_1 = "3,1"; // only two changes
    std::vector<Change> changes_pal1 = Method::PlaceNotationToChanges(palindrome_notation_1, 3);
    fprintf(stderr, "Changes for pal test 1\n");
    display_changes(changes_pal1);

    string palindrome_notation_2 = "3.1,1.3";
    std::vector<Change> changes_pal2 = Method::PlaceNotationToChanges(palindrome_notation_2, 3);
    fprintf(stderr, "Changes for pal test 2: should be 3.1.3.1.3.1\n");
    display_changes(changes_pal2);

    // test jump change notation handling
    fprintf(stderr, "Jump notation tests\n");
    // () notation
    string jump_notation_1 = "(14)";
    std::vector<Change> changes_j1 = Method::PlaceNotationToChanges(jump_notation_1, 4);
    display_changes(changes_j1);
    string jump_notation_2 = "(41)";
    std::vector<Change> changes_j2 = Method::PlaceNotationToChanges(jump_notation_2, 4);
    display_changes(changes_j2);
    // [] notation
    string jump_notation_3 = "[3412]";
    std::vector<Change> changes_j3 = Method::PlaceNotationToChanges(jump_notation_3, 4);
    display_changes(changes_j3);
    // mixing with standard notation
    string jump_notation_4 = "(35)8";
    std::vector<Change> changes_j4 = Method::PlaceNotationToChanges(jump_notation_4, 8);
    display_changes(changes_j4);
    string jump_notation_5 = "[543]8";
    std::vector<Change> changes_j5 = Method::PlaceNotationToChanges(jump_notation_5, 8);
    display_changes(changes_j5);
}
