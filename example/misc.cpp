// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "jvar.h"
#include <vector>

#ifdef _MSC_VER
#include <Windows.h>
#define getTickCount  GetTickCount
#endif

using namespace std;
using namespace jvar;

/**
 * \example misc.cpp
 * This example shows misc functionality in jvar that jvar is built
 * on top of but can also be used directly.
 */

void showFormat()
{
    int a = 45;
    string s;
    string s2;

    s2 = formatr("%08d", a);
    printf("%s\n", s2.c_str());

    format(s, "[%s] was built before...another string is being built now with hex 0x%x", s2.c_str(), a);

    printf("%s\n", s.c_str());
}


void showArray()
{
    ulongint start;
    const int cnt = 100000000;

    // BArray
    start = getTickCount();
    BArray arr(sizeof(long int), NULL);
    for (int i = 0; i < cnt; i++)
    {
        *(long int*)arr.append(NULL) = i;
    }
    printf("BArray time=%lu\n", getTickCount() - start);

    // ObjArray
    start = getTickCount();
    ObjArray<long int> arrt;
    for (int i = 0; i < cnt; i++)
    {
        *arrt.append() = i;
    }
    printf("ObjArray time=%lu\n", getTickCount() - start);

    // Stl
    start = getTickCount();

    std::vector<long int> v;
    for (int i = 0; i < cnt; i++)
    {
        v.push_back(i);
    }
    printf("STL time=%lu\n", getTickCount() - start);

    // Printed:
    // BArray time=801
    // ObjArray time=783
    // STL time=2283
}


void bugreport1()
{
    // Test that shows an issue when using GCC5's libstd ABI
    jvar::Variant inputData;
    std::string message = "[\"{\\\"msg\\\":\\\"connect\\\",\\\"session\\\":\\\"pjwLzc25gD\\\",\\\"version\\\":\\\"1\\\",\\\"support\\\":[\\\"1\\\",\\\"pre2\\\",\\\"pre1\\\"]}\"]";

    if (inputData.parseJson(message.c_str()))
    {
        for (int i=0; i<inputData.length(); i++)
        {
            jvar::Variant packet;
            packet.parseJson(inputData[i].toString().c_str());
            printf("input = %s\n", inputData.toString().c_str());
            printf("msg = %s\n", packet["msg"].toString().c_str());
        }
    }

    printf("success\n");
}

void testParse(const char* str)
{
    printf("\nParse: %s \n", str);
    for (int i = 0; i < 2; i++)
    {
        Parser p(str);

        if (p.failed())
        {
            printf("parsing failed: %s\n", p.errMsg().c_str());
            return;
        }

        p.setSinglePunc(i == 1);
        printf("Single(%s): ", i == 0 ? "No " : "Yes");

        while (!p.eof())
        {
            printf("<%s> ", p.token().c_str());
            p.advance();
        }
        printf("\n");
    }
}

void bugreport2()
{
    // Test that showed a parsing bug with ,-
    const char *test = "{\"angle\":[171.8,20,2,3,-96.3,20.6]}";

    Variant array;
    array.parseJson(test);
    puts(array.toJsonString().c_str());


    // Parser testing
    testParse("");
    testParse("'This is a string with a \\' followed\"");
    testParse("'each', 'word', 'is', 'single', '\"quotes\"'");
    testParse("_id, name,   count (_id)  , max(_id  ), count(*) ");
    testParse("one,,two");
    testParse("1,-2");
    testParse("1, -2");
    testParse("1,+2");
    testParse("a+=42");
    testParse(":,:,:-/");
    testParse("name:[2],name:2");
    testParse("0, 1.2, -33.04, 20");
    testParse("name(attribute1, attrub2, 'attrib3')");
    testParse("name(,,,)");
    testParse("name(");
}

void bugreport3()
{
    const char *test = ""
"{"
"  \"cmd\":\"sdata.json\","
"  \"arg_s\":1,"
"  \"time\":191451698,"
"  \"ybase\":2010,"
"  \"arg_m\":\"3\","
"  \"irms\":[5.8,0.0,0.0,0.0],"
"  \"vrms\":[121.8,0.1,0.1],"
"  \"watt\":[693.6,0.0,0.0],"
"  \"va\":[700.2,0.0,0.0],"
"  \"var_\":[-95.9,0.0,-0.0],"
"  \"power\":693.6,"
"  \"angle\":[172.1,119.9,140.4],"
"  \"period\":16668.0,"
"  \"freq\":59.0,"
"  \"energy\":42562362,"
"  \"watthr\":[42562362,0,0],"
"  \"vahr\":[46962595,0,0],"
"  \"varhr\":[-6319438,0,0],"
"  \"fwatthr\":[42668693,0,0],"
"  \"fvarhr\":[-6211040,0,0],"
"  \"energy\":9999999,"
"  \"emul\":2.608076793215e-03"
"}";

    Variant array;
    array.parseJson(test);

    puts(array.toString().c_str());
}

int main(int argc, char** argv)
{
    showFormat();
    showArray();
    bugreport3();
}
