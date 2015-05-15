// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "jvar.h"

using namespace jvar;

/**
 * \example arrays.cpp
 */

void showSimple()
{
    Variant arr;

    // Create an array in this variant

    arr.createArray();

    // Push some items.  You can also add using append() or insert()

    arr.push(10);
    arr.push(21);
    arr.push(50);
    arr.push(3022);
    arr.push(44);

    arr[1] = 234.00f;
    arr[2] = "Hello world";

    // Iterate over all of the array items (note: unusual syntax).  One can also
    // use length() and [] to iterate over the array.

    for (Iter<Variant> i; arr.forEach(i); )
    {
        printf("%d %s\n", i.pos(), i->toString().c_str());
    }

    // pop() all elements from the array

    Variant p;
    while (!(p = arr.pop()).empty())
    {
        printf("Pop %s\n", p.toString().c_str());
    }
}

void showAltInit()
{
    Variant arr;

    // Init an array using js style initializer syntax (string)

    arr.createArray("[123, 23, 'can be string', 233.2, false, -20, -120]");

    for (int i = 0; i < arr.length(); i++)
    {
        printf("%d %s\n", i, arr[i].toString().c_str());
    }
}

void showArrOfArr()
{
    Variant arr;

    // Arrays can contain any number of any types.  Unlike JS, they cannot have holes, though
    // the element can be of empty type.  Arrays can be nested as well.

    arr.createArray("[0, 'one', '2.0', 3, 'four']");

    arr[3].createArray();
    for (int i = 0; i < 4; i++)
    {
        arr[3].push(i);
    }

    arr[3][2].createArray("[999, 9999, 99999]");

    // Print the entire array of array as string

    printf("%s\n", arr.toString().c_str());

    // Printed:
    // [0,"one","2.0",[0,1,[999,9999,99999],3],"four"]

}

int main(int argc, char** argv)
{
    showSimple();
    showAltInit();
    showArrOfArr();
}
