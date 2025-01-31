#include <gtest/gtest.h>
#include "ringing_utils.h"

using namespace ringing_utils;


void display_rows_from_comp(std::vector<CompositionComponent> changes, int stage) {
    Row row = Row::Rounds(stage);
    fprintf(stderr, "%s\n", row.ToString().c_str());
    for (CompositionComponent comp: changes) {
        row = row.ApplyChange(comp.change);
        fprintf(stderr, "%s\n", row.ToString().c_str());
    }
}

TEST(RINGINGUTILS, COMPOSITION_PLAINLEADTEST) {
    // testing the hack to get a plain course of a method out
    Method c6;
    c6.place_notation = "-36-14-12-36-14-56,12";
    c6.stage = 6;
    c6.lead_length = 24;

    Composition plain_lead = Composition::PlainLead(c6);
    display_rows_from_comp(plain_lead.changes, plain_lead.stage);
}

TEST(RINGINGUTILS, COMPOSITION_PLAINCOURSETEST) {
    // testing the hack to get a plain course of a method out
    Method c6;
    c6.place_notation = "-36-14-12-36-14-56,12";
    c6.stage = 6;
    c6.lead_length = 24;

    Composition plain_course = Composition::PlainCourse(c6);
    display_rows_from_comp(plain_course.changes, plain_course.stage);
}