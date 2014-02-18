// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "jvar.h"
#include <dirent.h>

using namespace jvar;

/**
 * \example jsonparse.cpp
 * This example shows how to parse json text into a variant.  And generate json text
 * from a variant.  It also runs through the json test suite showing pass/fail.
 */

void showSimple()
{
    const char* jsontxt =
        "{"
        "    \"id\": 9781460700297,"
        "    \"name\": \"manuscript found in accra\","
        "    \"price\": 12.50"
        "}";

    Variant v;

    // Parse json text into a variant
    if (v.parseJson(jsontxt))
    {
        // Print the variant as string
        printf("Parsed...\ntoString=%s\n\n", v.toString().c_str());

        // Print the variant as jsonstring (produces strict json)
        printf("toJsonString=%s\n", v.toJsonString().c_str());
    }
}

void testJsonSuite()
{
    DIR* dir;
    struct dirent* fil;

    dir = opendir("./jsontest/");
    if (dir == NULL)
    {
        printf("Error: failed to open jsontest directory\n");
        return;
    }

    while ((fil = readdir(dir)) != NULL)
    {
        if (!equal(fil->d_name, ".") && !equal(fil->d_name, ".."))
        {
            std::string fn(fil->d_name);

            printf("\nFilename: %s should %s\n", fn.c_str(), fn.find("pass") != std::string::npos ? "pass" : "fail");

            Buffer jsontxt;
            jsontxt.readFile(fn.c_str(), true);

            Variant v;
            if (v.parseJson((const char*)jsontxt.cptr()))
            {
                printf("PASS!!\n");

                // std::string outfn = "out/" + fn;
                // ofstream f;
                // f.open(outfn.c_str());
                // f << v.toJsonString();
                // f.close();
            }
            else
            {
                printf("FAIL\n");
            }
        }
    }
}

int main(int argc, char** argv)
{
    showSimple();
    testJsonSuite();
}
