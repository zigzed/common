#include <string.h>
#include <string>
#include <iostream>
#include "common/cfg/xml/xml.h"

static const char* xgconsole =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>"
        "<Profile FormatVersion=\"1\">"
            "<Tools>"
                "<Tool Filename=\"jam\" AllowIntercept=\"true\">"
                    "<Description>Jamplus build system</Description>"
                "</Tool>"
                "<Tool Filename=\"mayabatch.exe\" AllowRemote=\"true\" OutputFileMasks=\"*.dae\" DeriveCaptionFrom=\"lastparam\" Timeout=\"40\" />"
                "<Tool Filename=\"meshbuilder_*.exe\" AllowRemote=\"false\" OutputFileMasks=\"*.mesh\" DeriveCaptionFrom=\"lastparam\" Timeout=\"10\" />"
                "<Tool Filename=\"texbuilder_*.exe\" AllowRemote=\"true\" OutputFileMasks=\"*.tex\" DeriveCaptionFrom=\"lastparam\" />"
                "<Tool Filename=\"shaderbuilder_*.exe\" AllowRemote=\"true\" DeriveCaptionFrom=\"lastparam\" />"
            "</Tools>"
        "</Profile>";


static void traverse_iter()
{
    cxx::xml::xml_document doc;
    if(!doc.load_buffer(xgconsole, strlen(xgconsole))) {
        std::cerr << "cxx::xml::load_buffer() failed\n";
        return;
    }

    cxx::xml::xml_node tools = doc.child("Profile").child("Tools");
    for(cxx::xml::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
        std::cout << "Tool:";
        for(cxx::xml::xml_attribute_iterator at = it->attributes_begin(); at != it->attributes_end(); ++at) {
            std::cout << " " << at->name() << "=" << at->value();
        }
        std::cout << std::endl;
    }
}

static void xpath_query()
{
    cxx::xml::xml_document doc;
    if(!doc.load_buffer(xgconsole, strlen(xgconsole))) {
        std::cerr << "cxx::xml::load_buffer() failed\n";
        return;
    }

    cxx::xml::xpath_query query_remote_tools("/Profile/Tools/Tool[@AllowRemote='true']");
    cxx::xml::xpath_node_set tools = query_remote_tools.evaluate_node_set(doc);
    std::cout << "Remote tool: ";
    tools[2].node().print(std::cout);

    cxx::xml::xpath_query query_name("concat(substring-before(@Filename, '_'), ' produces ', @OutputFileMasks)");

    for(cxx::xml::xml_node tool = doc.first_element_by_path("Profile/Tools/Tool"); tool; tool = tool.next_sibling()) {
        std::string s = query_name.evaluate_string(tool);

        std::cout << s << std::endl;
    }
}

int main(int argc, char* argv[])
{
    traverse_iter();

    std::cout << "\n\n\n";

    xpath_query();

    return 0;
}
