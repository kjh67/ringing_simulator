#include <stdio.h>
#include <cstring>
#include <stdlib.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <curl/curl.h>
#include <zip.h>

#include "file_utils.h"


class MethodLibraryReadingRoutines {
    public:

    static const int READ_INCREMENT = 256;

    static size_t write_data_to_file(void* buffer, size_t size, size_t nmemb, void* filestream) {
        size_t written = fwrite(buffer, size, nmemb, (FILE *) filestream);
        return written;
    }

    static int download_file(CURL* curl_handle, string url, string destination_address) {
        // set CURL options
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        // verbose and progress bar, helpful for debugging
        // curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
        // curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, MethodLibraryReadingRoutines::write_data_to_file);

        // open file at destination address and write to it
        FILE* filestream = fopen(destination_address.c_str(), "wb");
        if (filestream) {
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, filestream);
            curl_easy_perform(curl_handle);
            fclose(filestream);
        }
        //TODO: add error handling
        return 0;
    }

    /**
     * @brief Extracts a single file (specified using the filename parameter) from a zip archive.
     * 
     * @param archive_source Path to zip archive containing the target file.
     * @param file_dest Full path for extracted file.
     * @param filename Name of target file in the zip archive.
     * @return int 0 if successful, -1 for errors
     */
    static int unzip_file(string archive_source, string file_dest, string filename) {
        int err = 0;

        // Open archive and check for errors
        zip* ziparchive = zip_open(archive_source.c_str(), 0, &err);
        if (err!=0) {
            fprintf(stderr, "Error: zip archive could not be opened\n");
            return -1;
        }
        // Check that the archive contains the expected file
        struct zip_stat st;
        zip_stat_init(&st);
        err = zip_stat(ziparchive, filename.c_str(), 0, &st);
        if (err!=0) {
            fprintf(stderr, "Error: expected file not found in archive\n");
            return -1;
        }

        // Open a file to write extracted contents into
        FILE* destfile = fopen(file_dest.c_str(), "wb");
        if (destfile==NULL) {
            fprintf(stderr, "Error: destination file could not be opened\n");
            return -1;
        }

        // Read the archive into the file
        zip_file* zipfile = zip_fopen(ziparchive, filename.c_str(), 0);

        // right now, this is causing a segfault!
        int progress = 0;
        int bytes_read;
        int bytes_written;
        char read_buffer[READ_INCREMENT];
        do {
            bytes_read = zip_fread(zipfile, read_buffer, READ_INCREMENT);
            bytes_written = fwrite(read_buffer, sizeof(char), bytes_read, destfile);
            // TODO: add a check that bytes read and bytes written are the same
            progress += bytes_written;
        } while (bytes_written>0);

        if (bytes_written<0) {
            fprintf(stderr, "Error: problem reading extracted file\n");
            return -1;
        }

        // Close extracted file
        err = fclose(destfile);
        if (err!=0) {
            fprintf(stderr, "Error: problem closing extracted file\n");
            return -1;
        }
        // Close zip file and archive
        err = zip_fclose(zipfile);
        if (err!=0) {
            fprintf(stderr, "Error: problem closing compressed file\n");
            return -1;
        }
        err = zip_close(ziparchive);
        if (err!=0) {
            fprintf(stderr, "Error: problem closing zip archive\n");
            return -1;
        }
        return 0;
    }
};


namespace file_utils {

MethodLibrary::MethodLibrary(string library_path) {
    // Initialise the libxml parser and parse the method library
    xmlInitParser();
    library_ptr = xmlParseFile(library_path.c_str());
    if (library_ptr == NULL) {
        fprintf(stderr, "Library not found\n");
        throw MethodLibraryLoadError();
    }
}

MethodLibrary::~MethodLibrary() {
    xmlFreeDoc(library_ptr);
    xmlCleanupParser();
}


string MethodLibrary::RetrieveLibraryDate() {
    // open the xml file for searching
    xmlXPathContextPtr xp_context = xmlXPathNewContext(library_ptr);
    if (xp_context == NULL) {
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }
    if (xmlXPathRegisterNs(xp_context, BAD_CAST "df", BAD_CAST "http://www.cccbr.org.uk/methods/schemas/2007/05/methods") != 0) {
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }

    // retrieve the method node using method id
    string xpath_expression = "/df:collection";
    xmlXPathObjectPtr xpath_result = xmlXPathEvalExpression(BAD_CAST xpath_expression.c_str(), xp_context);
    if (xpath_result == NULL) {
        fprintf(stderr, "XPath error, NULL returned.\n");
        xmlXPathFreeObject(xpath_result);
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }
    xmlNodePtr result_node = xpath_result->nodesetval->nodeTab[0];
    string date = (const char*) xmlGetProp(result_node, BAD_CAST "date");

    xmlXPathFreeObject(xpath_result);
    xmlXPathFreeContext(xp_context);

    return date;
}


std::list<MethodSearchResult> MethodLibrary::SearchLibrary(string partial_method_name, int stage, MethodClass method_class) {
    // Create xpath evaluation context (maybe save this context each time instead? even if )
    xmlXPathContextPtr xp_context = xmlXPathNewContext(library_ptr);
    if (xp_context == NULL) {
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }

    // Register default namespace for the file: all terms in xpath query must be prefixed by "df"
    // Custom prefix kludge needed because libxml2 does NOT support XPath 2.0 default namespaces!
    if (xmlXPathRegisterNs(xp_context, BAD_CAST "df", BAD_CAST "http://www.cccbr.org.uk/methods/schemas/2007/05/methods") != 0) {
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }

    // Build xpath expression
    string xp_expression = "/df:collection/df:methodSet";
    // filter methodSets by stage and classification
    if (stage!=0 & method_class!=NONE) {
        xp_expression.append("[df:properties/df:stage="+std::to_string(stage)+" and df:properties/df:classification=\""+MethodClassToString(method_class)+"\"]");
    }
    else if (stage!=0) {
        xp_expression.append("[df:properties/df:stage="+std::to_string(stage)+"]");
    }
    else if (method_class!=NONE) {
        xp_expression.append("[df:properties/df:classification=\""+MethodClassToString(method_class)+"\"]");
    }

    xp_expression.append("/df:method");
    // filter methods by (partial) name using contains(), and translate() to make results lower-case
    if (partial_method_name != "") {
        xp_expression.append("[contains(translate(df:title, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz'),\""+partial_method_name+"\")]");
    }

    // Execute xpath expression
    xmlXPathObjectPtr search_result = xmlXPathEvalExpression(BAD_CAST xp_expression.c_str(), xp_context);
    if (search_result == NULL) {
        fprintf(stderr, "Unable to execute expression!\n");
        xmlXPathFreeObject(search_result);
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }

    // Process results
    xmlNodePtr cur;
    xmlNodeSetPtr result_nodes = search_result->nodesetval;
    int size = (result_nodes) ? result_nodes->nodeNr : 0;
    std::list<MethodSearchResult> result_list = std::list<MethodSearchResult>();
    for(int i = 0; i < size; i++) {
        //TODO: xpath injection is possible; maybe abort showing the results if the returned nodes are not methods
        if(result_nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
            cur = result_nodes->nodeTab[i];
            int num_children = xmlChildElementCount(cur);
            xmlNodePtr cur_children = cur->children;
            xmlNodePtr nextChild = cur_children->next;
            for (int j=0; j<num_children; j++) {
                while (nextChild->type != XML_ELEMENT_NODE) {
                    nextChild = nextChild->next;
                }
                if (strcmp((const char*) nextChild->name,"title") == 0) {
                    // Construct MethodSearchResult for this item and push to the return list
                    result_list.push_back(MethodSearchResult{(const char*) xmlGetProp(cur, BAD_CAST "id"), (const char*) xmlNodeGetContent(nextChild)});
                    break;
                }
                nextChild = nextChild->next;
            }
        }
    }

    // XPath cleanup and return
    xmlXPathFreeObject(search_result);
    xmlXPathFreeContext(xp_context);
    return result_list;
}


Method MethodLibrary::RetrieveMethodInformation(string method_id) {
    // open the xml file for searching
    xmlXPathContextPtr xp_context = xmlXPathNewContext(library_ptr);
    if (xp_context == NULL) {
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }
    if (xmlXPathRegisterNs(xp_context, BAD_CAST "df", BAD_CAST "http://www.cccbr.org.uk/methods/schemas/2007/05/methods") != 0) {
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }

    // retrieve the method node using method id
    string xpath_expression = "/df:collection/df:methodSet/df:method[@id=\""+method_id+"\"]";
    xmlXPathObjectPtr xpath_result = xmlXPathEvalExpression(BAD_CAST xpath_expression.c_str(), xp_context);
    if (xpath_result == NULL) {
        fprintf(stderr, "XPath error, NULL returned.\n");
        xmlXPathFreeObject(xpath_result);
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }
    if (xpath_result->nodesetval->nodeNr != 1) {
        fprintf(stderr, "No matching methods found.\n");
        xmlXPathFreeObject(xpath_result);
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }

    // find the MethodSet first, and check the properties from there: then only set below if overridden by the method
    // since we known the method exists, must be in a methodSet: get this one
    xpath_expression = "/df:collection/df:methodSet[df:method/@id=\""+method_id+"\"]/df:properties";
    xmlXPathObjectPtr methodSet_properties = xmlXPathEvalExpression(BAD_CAST xpath_expression.c_str(), xp_context);
    if (methodSet_properties == NULL) {
        fprintf(stderr, "XPath error, NULL returned from second expression.\n");
        xmlXPathFreeObject(xpath_result);
        xmlXPathFreeObject(methodSet_properties);
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }
    if (methodSet_properties->nodesetval->nodeNr != 1) {
        fprintf(stderr, "No matching methodSets found.\n");
        xmlXPathFreeObject(xpath_result);
        xmlXPathFreeObject(methodSet_properties);
        xmlXPathFreeContext(xp_context);
        throw MethodLibrarySearchError();
    }

    Method to_return;
    to_return.id = method_id;
    // first, set properties using the methodSet information
    xmlNodePtr result_node = methodSet_properties->nodesetval->nodeTab[0];
    int num_children = xmlChildElementCount(result_node);
    xmlNodePtr result_children = result_node->children;
    xmlNodePtr current_child = result_children->next;
    for (int i=0; i<num_children; i++) {
        while (current_child->type != XML_ELEMENT_NODE) {
            current_child = current_child->next;
        }
        // set properties of the return struct
        const char* current_name = (const char*) current_child->name;
        if (strcmp(current_name, "stage") == 0) {
            // need to get this from the methodSet
            to_return.stage = std::stoi((const char*) xmlNodeGetContent(current_child));
        }
        else if (strcmp(current_name, "classification") == 0) {
            // if a property is there, it's only because it's true, I hope!
            to_return.little = (xmlGetProp(current_child, BAD_CAST "little") != NULL);
            to_return.differential = (xmlGetProp(current_child, BAD_CAST "differential") != NULL);
            to_return.plain = (xmlGetProp(current_child, BAD_CAST "plain") != NULL);
            to_return.treble_dodging = (xmlGetProp(current_child, BAD_CAST "trebleDodging") != NULL);
            to_return.classification = StringToMethodClass((const char*) xmlNodeGetContent(current_child));
        }
        else if (strcmp(current_name, "lengthOfLead") == 0) {
            to_return.lead_length = std::stoi((const char*) xmlNodeGetContent(current_child));
        }
        else if (strcmp(current_name, "numberOfHunts") == 0) {
            to_return.num_hunt_bells = std::stoi((const char*) xmlNodeGetContent(current_child));
        }
        //TODO: change this to instantiating an actual row, rather than just a string
        else if (strcmp(current_name, "leadHead") == 0) {
            to_return.lead_head = (const char*) xmlNodeGetContent(current_child);
        }
        else if (strcmp(current_name, "leadHeadCode") == 0) {
            to_return.lead_head_code = (const char*) xmlNodeGetContent(current_child);
        }
        current_child = current_child->next;
    }

    result_node = xpath_result->nodesetval->nodeTab[0];
    num_children = xmlChildElementCount(result_node);
    result_children = result_node->children;
    current_child = result_children->next;
    // set method-specific properties and perform necessary overrides using method information
    for (int i=0; i<num_children; i++) {
        while (current_child->type != XML_ELEMENT_NODE) {
            current_child = current_child->next;
        }
        // set properties of the return struct
        const char* current_name = (const char*) current_child->name;
        if (strcmp(current_name, "title") == 0) {
            to_return.title = (const char*) xmlNodeGetContent(current_child);
        }
        else if (strcmp(current_name, "stage") == 0) {
            // need to get this from the methodSet
            to_return.stage = std::stoi((const char*) xmlNodeGetContent(current_child));
        }
        else if (strcmp(current_name, "classification") == 0) {
            // if a property is there, it's only because it's true, I hope!
            to_return.little = (xmlGetProp(current_child, BAD_CAST "little") != NULL);
            to_return.differential = (xmlGetProp(current_child, BAD_CAST "differential") != NULL);
            to_return.plain = (xmlGetProp(current_child, BAD_CAST "plain") != NULL);
            to_return.treble_dodging = (xmlGetProp(current_child, BAD_CAST "trebleDodging") != NULL);
            to_return.classification = StringToMethodClass((const char*) xmlNodeGetContent(current_child));
        }
        else if (strcmp(current_name, "notation") == 0) {
            to_return.place_notation = (const char*) xmlNodeGetContent(current_child);
        }
        else if (strcmp(current_name, "lengthOfLead") == 0) {
            // this is not guaranteed
            to_return.lead_length = std::stoi((const char*) xmlNodeGetContent(current_child));
        }
        else if (strcmp(current_name, "numberOfHunts") == 0) {
            to_return.num_hunt_bells = std::stoi((const char*) xmlNodeGetContent(current_child));
        }
        //TODO: change this to instantiating an actual row, rather than just a string
        else if (strcmp(current_name, "leadHead") == 0) {
            to_return.lead_head = (const char*) xmlNodeGetContent(current_child);
        }
        else if (strcmp(current_name, "leadHeadCode") == 0) {
            to_return.lead_head_code = (const char*) xmlNodeGetContent(current_child);
        }
        current_child = current_child->next;
    }

    // Clean up and return
    xmlXPathFreeObject(xpath_result);
    xmlXPathFreeObject(methodSet_properties);
    xmlXPathFreeContext(xp_context);
    return to_return;
}


void MethodLibrary::UpdateLibrary(string file_location, string temp_location) {
    if (temp_location.empty()) {
        temp_location = file_location;
    }

    string lib_rel_location = "/all.xml";
    string jump_rel_location = "/jumps.xml";
    
    // First: download two new files, one of the 'all classes' xml
    // and another of the 'jump methods' xml, to the temp location
    auto curl_handle = curl_easy_init();
    int success;
    success = MethodLibraryReadingRoutines::download_file(curl_handle, CCCBR_LIBRARY_URL, temp_location+lib_rel_location+".zip");
    if (success!=0) {
        fprintf(stderr, "Method library could not be downloaded\n");
        throw MethodLibraryUpdateError();
    }
    success = MethodLibraryReadingRoutines::download_file(curl_handle, CCCBR_JUMPLIB_URL, temp_location+jump_rel_location+".zip");
    if (success!=0) {
        fprintf(stderr, "Jump library could not be downloaded\n");
        throw MethodLibraryUpdateError();
    }
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    // first, part two: unzip the files
    success = MethodLibraryReadingRoutines::unzip_file(temp_location+lib_rel_location+".zip", temp_location+lib_rel_location, "CCCBR_methods.xml");
    if (success<0) {
        fprintf(stderr, "Method library could not be unzipped\n");
        throw MethodLibraryUpdateError();
    }
    success = MethodLibraryReadingRoutines::unzip_file(temp_location+jump_rel_location+".zip", temp_location+jump_rel_location, "CCCBR_Jump.xml");
    if (success<0) {
        fprintf(stderr, "Method library could not be unzipped\n");
        throw MethodLibraryUpdateError();
    }

    fprintf(stdout, "Files downloaded and unzipped\n");


    // Third: overwrite old library with the method library; then, keep open and add the jump library to the end of the file
    // open the target file
    FILE* target_file = fopen(file_location.c_str(), "wb");
    if (target_file==NULL) {
        fprintf(stderr, "Error: Failed to open target library save location.\n");
        throw MethodLibraryUpdateError();
    }

    // open the temp files to read
    FILE* lib_file = fopen((temp_location+lib_rel_location).c_str(), "rb");
    if (lib_file==NULL) {
        fprintf(stderr, "Error: failed to open temp method library.\n");
        throw MethodLibraryUpdateError();
    }
    FILE* jump_file = fopen((temp_location+jump_rel_location).c_str(), "rb");
    if (jump_file==NULL) {
        fprintf(stderr, "Error: failed to open temp jump library.\n");
        throw MethodLibraryUpdateError();
    }

    // read contents of the lib files into the destination location in small pieces
    int bytes_read;
    int bytes_written;
    char read_buffer[MethodLibraryReadingRoutines::READ_INCREMENT];
    do {
        bytes_read = fread(read_buffer, sizeof(char), MethodLibraryReadingRoutines::READ_INCREMENT, lib_file);
        bytes_written = fwrite(read_buffer, sizeof(char), bytes_read, target_file);
        //TODO: add check that bytes read and written are the same
    } while (bytes_written>0);
    // skip the second xml declaration
    fseek(target_file, -13, SEEK_CUR);
    //TODO: improve this sh*t. maybe seek for the end of the first <notes> tag, or the first methodSet
    // a different date format might actually break everything - we'll see next week!
    fseek(jump_file, 707, SEEK_CUR);
    do {
        bytes_read = fread(read_buffer, sizeof(char), MethodLibraryReadingRoutines::READ_INCREMENT, jump_file);
        bytes_written = fwrite(read_buffer, sizeof(char), bytes_read, target_file);
    } while (bytes_written>0);
    //TODO: add error handling

    fclose(lib_file);
    fclose(jump_file);
    fclose(target_file);

    // Fourth: clean up
    // delete temp files creates; zip archives and intermediate .xml files
    std::remove((temp_location+lib_rel_location+".zip").c_str());
    std::remove((temp_location+lib_rel_location).c_str());
    std::remove((temp_location+jump_rel_location+".zip").c_str());
    std::remove((temp_location+jump_rel_location).c_str());

    fprintf(stdout, "finished and cleaned up\n");
}

}