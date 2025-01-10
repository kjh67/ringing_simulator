#include <gtest/gtest.h>
#include "ringing_utils.h"

using namespace ringing_utils;

TEST(RINGINGUTILS, ROW_INITTEST) {
    // check that rounds on various numbers are instantiated as expected
    Row minimus = Row::Rounds(4);
    fprintf(stderr, "Rounds on 4 is: %s\n", minimus.ToString().c_str());
    EXPECT_EQ(strcmp("1234", minimus.ToString().c_str()), 0);

    Row twenty = Row::Rounds(20);
    fprintf(stderr, "Rounds on 20 is: %s\n", twenty.ToString().c_str());
    EXPECT_EQ(strcmp("1234567890ETABCDFGHJ", twenty.ToString().c_str()), 0);

    Row queens = Row(6, "135246");
    fprintf(stderr, "Queens is: %s\n", queens.ToString().c_str());
    EXPECT_EQ(strcmp("135246", queens.ToString().c_str()), 0);
}

TEST(RINGINGUTILS, ROW_CHANGETEST) {
    // check that changes permute rows as expected
    std::vector<Change> changes = Method::PlaceNotationToChanges("x16x16x14", 6);
    Row row = Row::Rounds(6);
    fprintf(stderr, "%s\n", row.ToString().c_str());
    for (Change change: changes) {
        row = row.ApplyChange(change);
        fprintf(stderr, "%s\n", row.ToString().c_str());
    }
}
