#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "file_utils.h"


namespace file_utils {

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
        xp_expression.append("[contains(translate(df:name, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz'),\""+partial_method_name+"\")]");
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
        //TODO: xpath injection is possible; abort showing the results if the returned nodes are not methods
        if(result_nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
            cur = result_nodes->nodeTab[i];
            xmlNodePtr cur_children = cur->children;
            xmlNodePtr nextChild = cur_children->next;
            for (int j=0; j<xmlChildElementCount(cur); j++) {
                if (nextChild->type != XML_ELEMENT_NODE) {
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

}