// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "jvar.h"

using namespace jvar;

/**
 * \example basics.cpp
 * Basic Variants -- You can declare a variable of type "Variant" and then assign
 * any kind of data to it.<p>You can do conversions to different types and call
 * operators.
 */

int main(int argc, char** argv)
{
    Variant v0;
    Variant v1 = 23;
    Variant v2;
    Variant v3;

    // Assign/modifie values
    v0.format("-printf-style %s %p", argv[0], argv);

    ++v1;
    v2 = 553.2f;

    v2 = (double)v2 * 2.0f;

    // Concat
    v3 = v1 + " Hello world ";
    v3 += v2;
    v3 += v0;

    printf("%s\n", v3.toString().c_str());

    // Printed:
    // 24 Hello world 1106.4-printf-style ./ex_basics 0x7fff1f43d618
}
