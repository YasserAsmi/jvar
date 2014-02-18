// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "jvar.h"

using namespace jvar;

/**
 * \example func.cpp
 * This example shows how to use function objects inside variants.  Function objects
 * can contain closure/enviroment for the function object and can be called from code
 * passing in additional parameters.
 */

// Actual C++ function--it receives env and arg as Variants.
Variant convert(Variant& env, Variant& arg)
{
    Variant ret;
    ret = ((double)env["offset"] + (double)arg[0]) * (double)env["factor"];
    return ret.toFixed(2) + " " + env["toUnit"].toString();
}

// This fuction creates a function object--to do the conversion based some
// parameters.  Once created function object has all the info it needs
Variant makeConverter(Variant tounit, Variant factor, Variant offset = VNULL)
{
    Variant funcobj;

    funcobj.createFunction(convert);
    funcobj.addEnv("toUnit", tounit);
    funcobj.addEnv("factor", factor);
    funcobj.addEnv("offset", offset);

    return funcobj;
}

int main(int argc, char** argv)
{
    Variant res;

    // Make function objects

    Variant milesToKm = makeConverter("km", 1.60936);
    Variant poundsToKg = makeConverter("kg", 0.45460);
    Variant farenheitToCelsius = makeConverter("degrees-c", 0.5556, -32l);

    // Call function objects and print results

    res = milesToKm(10);
    printf("milestokm(10) = %s\n", res.toString().c_str());

    res = poundsToKg(2.5);
    printf("poundsToKg(2.5) = %s\n", res.toString().c_str());

    res = farenheitToCelsius(98);
    printf("farenheitToCelsius(98l) = %s\n", res.toString().c_str());

    // Printed:
    // milestokm(10) = 16.09 km
    // poundsToKg(2.5) = 1.14 kg
    // farenheitToCelsius(98l) = 36.67 degrees-c
}
