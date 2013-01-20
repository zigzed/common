#include <stdio.h>
#include <string.h>
#include <cassert>
#include "common/cfg/json/json.h"

static const char* test_message = "// Configuration options\n"
        "{\n"
        "// Default encoding for text\n"
        "\"encoding\" : \"UTF-8\",\n"
        "// Plug-ins loaded at start-up\n"
        "\"plug-ins\" : [\n"
        "\"python\",\n"
        "\"c++\",\n"
        "\"ruby\"\n"
        "],\n"
        "// Tab indent size\n"
        "\"indent\" : { \"length\" : 3, \"use_space\": true }\n"
        "}\n";

int main(int argc, char* argv[])
{
    cxx::json::Value    root;
    cxx::json::Reader   reader;
    bool isok = reader.parse(test_message, test_message + strlen(test_message), root);
    if(!isok) {
        std::cerr << "failed to parse the test_mesage: "
                  << reader.getFormatedErrorMessages() << "\n";
        return -1;
    }

    std::string encoding = root.get("encoding", "UTF-8").asString();
    const cxx::json::Value plugins = root["plug-ins"];
    for(int i = 0; i < plugins.size(); ++i) {
        std::cout << "plugins: " << plugins[i].asString() << "\n";
    }

    root["indent"].get("length", 100).asInt();
    root["indent"].get("use_space", false).asBool();

    root["encoding"] = "GB2312";
    root["indent"]["length"] = 4;
    root["indent"]["use_space"] = false;

    cxx::json::StyledWriter writer;
    std::string output = writer.write(root);

    std::cout << output << "\n\n\n";

    std::cout << root << "\n\n\n";


    return 0;
}
