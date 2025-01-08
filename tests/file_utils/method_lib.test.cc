#include <gtest/gtest.h>
#include "file_utils.h"

#include <stdio.h>

using namespace file_utils;

TEST(FILEUTILS, METHODLIBRARY_LOADTEST) {
    EXPECT_NO_THROW(MethodLibrary lib = MethodLibrary(ROOT_LIBRARY_FILEPATH));
    EXPECT_THROW(MethodLibrary lib = MethodLibrary(ROOT_LIBRARY_FILEPATH+"a"), MethodLibraryLoadError);
}

TEST(FILEUTILS, METHODLIBRARY_DATETEST) {
    MethodLibrary lib = MethodLibrary(ROOT_LIBRARY_FILEPATH);
    string lib_date = lib.RetrieveLibraryDate();
    fprintf(stdout, "Current library is correct as of date: %s\n", lib_date.c_str());
}

TEST(FILEUTILS, METHODLIBRARY_SEARCHTEST) {
    // How to test search effectively? Maybe just testing that it works is good enough
    MethodLibrary lib = MethodLibrary(ROOT_LIBRARY_FILEPATH);
    auto result_list = lib.SearchLibrary("rutland");
    fprintf(stdout, "First result is: %s, %s\n", result_list.front().id, result_list.front().name);
    fprintf(stdout, "Last result is: %s, %s\n", result_list.back().id, result_list.back().name);
}

TEST(FILEUTILS, METHODLIBRARY_UPDATETEST) {
    //TODO: Since this test uses more significant resources than others, find a more elegant way of making it optional.
    // string temp_location = "/home/khawkins/projects/ringing/temp";
    // string file_location = "/home/khawkins/projects/ringing/temp/final.xml";
    // MethodLibrary::UpdateLibrary(file_location, temp_location);
    fprintf(stdout, "Temporarily skipping to avoid frivolous downloads\n");
}

TEST(FILEUTILS, METHODLIBRARY_RETRIEVETEST) {
    MethodLibrary lib = MethodLibrary(ROOT_LIBRARY_FILEPATH);
    string method_id = "m123345";
    EXPECT_THROW(lib.RetrieveMethodInformation(method_id), MethodLibrarySearchError);

    method_id = "m10777";
    Method m = lib.RetrieveMethodInformation(method_id);
    fprintf(stdout,"Name: %s\nStage: %d\nClassification: %s\nPlace Notation: %s\nLead Length: %d\nLead Head: %s\n",
        m.title.c_str(), m.stage, MethodClassToString(m.classification).c_str(), m.place_notation.c_str(), m.lead_length, m.lead_head.c_str());
    ASSERT_EQ(strcmp(m.title.c_str(), "Rutland Branch Bob Doubles"), 0);
}