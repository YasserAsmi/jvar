// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "jvar.h"
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <dirent.h>
#endif

#include <fstream>

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

#ifdef _MSC_VER
void testJsonSuite()
{
    const char* datadir = "..\\..\\example\\jsontest\\";
    printf("\nRunning test on json files in %s....\n", datadir);

    WIN32_FIND_DATA ffd;
    std::string dir(datadir);
    dir += "*";
    HANDLE hfind = FindFirstFile(dir.c_str(), &ffd);
    if (hfind == INVALID_HANDLE_VALUE)
    {
        printf("Error: failed to open jsontest directory, err=%d\n", GetLastError());
        return;
    }

    do
    {
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            std::string fn(datadir);
            fn += ffd.cFileName;

            printf("\nFilename: '%s' should %s\n", fn.c_str(), fn.find("pass") != std::string::npos ? "pass" : "fail");

            Buffer jsontxt;
            jsontxt.readFile(fn.c_str(), true);

            Variant v;
            if (v.parseJson((const char*)jsontxt.cptr()))
            {
                printf("PASS!!\n");
            }
            else
            {
                printf("FAIL\n");
            }
        }
   }
   while (FindNextFile(hfind, &ffd) != 0);
   FindClose(hfind);
}

#else
void testJsonSuite()
{
    DIR* dir;
    struct dirent* fil;

    printf("\nRunning test on json files in ../example/jsontest directory....\n");

    dir = opendir("../example/jsontest/");
    if (dir == NULL)
    {
        printf("Error: failed to open jsontest directory\n");
        closedir(dir);
        return;
    }

    while ((fil = readdir(dir)) != NULL)
    {
        if (!equal(fil->d_name, ".") && !equal(fil->d_name, ".."))
        {
            std::string fn("../example/jsontest/");

            fn += fil->d_name;

            printf("\nFilename: '%s' should %s\n", fn.c_str(), fn.find("pass") != std::string::npos ? "pass" : "fail");

            Buffer jsontxt;
            jsontxt.readFile(fn.c_str(), true);

            Variant v;
            if (v.parseJson((const char*)jsontxt.cptr()))
            {
                printf("PASS!!\n");
            }
            else
            {
                printf("FAIL\n");
            }
        }
    }

    closedir(dir);
}

#endif


void bench(const char* fn)
{
    ulongint start;
    Buffer jsontxt;

    jsontxt.readFile(fn, true);

    const int count = 1;
    Variant v;
    bool success;

    start = getTickCount();
    for (int i = 0; i < count; i++)
    {
        success = v.parseJson((const char*)jsontxt.cptr());
    }
    if (success)
    {
        printf("PASS, total parse time for %d passes=%lu\n", count, getTickCount() - start);
    }
    else
    {
        printf("FAIL, total parse time for %d passes=%lu\n", count, getTickCount() - start);
    }

    start = getTickCount();
    StrBld sb;
    for (int i = 0; i < count; i++)
    {
        v.makeJson(sb);
    }
    printf("Pretty str time for %d passes=%lu\n", count, getTickCount() - start);

    std::string outfn = "/tmp/out.json";
    std::ofstream f;
    f.open(outfn.c_str());
    f << sb.toString();;
    f.close();
}

int main(int argc, char** argv)
{
    showSimple();
    testJsonSuite();
    if (argc > 1)
    {
        bench(argv[1]);
    }
}
