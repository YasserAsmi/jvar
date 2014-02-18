// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "jvar.h"

using namespace jvar;

/**
 * \example printv.cpp
 * Example program that implements a printf like function but one that takes
 * variants.  For formatting, it uses {0}, {1} etc for parameters.  It also
 * allows taking string parameters by appending a s after the param number ie {1s}
 */

void printv(const char* fmt, ...)
{
    // "text {0} {1s} {2}"

    va_list vl;

    std::string outstr;
    outstr.reserve(strlen(fmt));

    const char* p = fmt;

    while (*p)
    {
        if (*p == '{')
        {
            p++;

            if (*p == '{')
            {
                outstr += '{';
                p++;
                continue;
            }

            char* endnum;
            long int param = strtol(p, &endnum, 10);

            p = endnum;

            va_start(vl, fmt);
            for (int i = 0; i < param; i++)
            {
                va_arg(vl, void*);
            }

            if (*p == 's')
            {
                p++;
                const char* s = va_arg(vl, const char*);
                outstr += s;
            }
            else
            {
                Variant* v = va_arg(vl, Variant*);
                outstr += v->toString();
            }
            va_end(vl);
        }
        else
        {
            outstr += *p;
        }

        p++;
    }

    printf("%s", outstr.c_str());
}

int main(int argc, char** argv)
{
    Variant v1(100);
    Variant v2("two");
    Variant v3 = 3.0f;
    Variant v4;

    v4.createObject("{id:4000, name:'four thousand'}");

    printv("Printing v1={0} v2={1}  v3={2}  v3={3}\n", &v1, &v2, &v3, &v4);
}
