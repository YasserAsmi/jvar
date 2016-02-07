jvar
====
jvar tries to capture some of the expressiveness of JavaScript and bring it into C++

In particular, it implements a Variant type which is similar to 'var' in JS.  Like JS, you can store data of any type into it.  You can have objects, arrays, arrays of objects, objects with arrays, etc and nest them to any level.   You can even have function objects with closure.   You can write out Variant as JSON and you can parse JSON directly into a Variant.

The Variant, combined with JSON and simple automatic memory management, makes for a very powerful data structure even in C++.
jvar is designed to be very fast.  Because the backing data structures are optimized, JSON parsing speed is very fast.   Special attention has been paid to how often things are copied and how often memory is allocated.

jvar doesn't use have any dependencies.  It doesn't use Boost and hardly uses STL.  It only uses std::string--but even with that jvar tries to minimize memory allocations.  Some C++11 features are optionally (via build option) used to provide more ease of use.

Please take a look at the examples to see if you agree with the added expressiveness.

Build! Go!


Contributers:
Yasser Asmi     https://github.com/YasserAsmi
Alex Elzenaar   https://github.com/aelzenaar
Andrei Markeev  https://github.com/andrei-markeev
