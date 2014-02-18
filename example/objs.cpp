// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "jvar.h"

using namespace jvar;

/**
 * \example objs.cpp
 */

void showIter(Variant& v)
{
    for (Iter<Variant> i; v.forEach(i); )
    {
        printf("%d. Key: %s Value: %s\n", i.pos(), i.key(), i->toString().c_str());
    }
    printf("\n");
}

void showSimple()
{
    Variant obj;

    // Create an object in the variant

    obj.createObject();

    // Add properties to the object.  Unlike JS, properties must be explicitly added

    obj.addProperty("PropA", 65);
    obj.addProperty("PropB", 66);
    obj.addProperty("PropC", 67);

    // Update propertie values

    obj["PropA"] = 6500;
    obj["PropB"]++;
    obj["PropC"] = (double)obj["PropC"] * 2.0f;

    // Print the entire object as string

    printf("%s\n", obj.toString().c_str());

    // {"PropA":6500,"PropB":67,"PropC":134.0}

    showIter(obj);
}

void showAltInit()
{
    Variant obj;

    obj.createObject("{firstname:'Yasser', lastname:'Asmi', email:'yasserasmi@live.com', dogname:'Jake'}");

    showIter(obj);
}

void showObjOfArr()
{
    Variant obj;

    obj.createObject();

    obj.addProperty("PropA");
    obj.addProperty("PropB");
    obj.addProperty("PropC");

    obj["PropA"].createArray("[110, 120, 130]");
    obj["PropB"].createArray("[210, 220]");
    obj["PropC"].createArray("[310, 320, 330, 340, 350]");

    showIter(obj);
}

int main(int argc, char** argv)
{
    showSimple();
    showAltInit();
    showObjOfArr();
}
