// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _VAR_H
#define _VAR_H

#include "util.h"
#include "arr.h"
#include "str.h"
#include <math.h>

#define VNULL Variant::vNull
#define VEMPTY Variant::vEmpty

#define VAR_PATH_DELIM     "."

namespace jvar
{

/** \cond INTERNAL */
class VarSuppImpl : public InterfaceImpl
{
public:
    VarSuppImpl() :
        mSupp(NULL)
    {
    }
    virtual ~VarSuppImpl()
    {
    }
    virtual InterfaceImpl* newImpl()
    {
        return (InterfaceImpl*) (new VarSuppImpl());
    }

public:
    FixedStr<24> mClassName;
    void* mSupp;
};
/** \endcond */

class VarFuncObj;

/**
 * Variant class is inspired by Javascript's Var semantics.  It  maintains data of different kinds such
 * as ints, doubles, strings.  It also stores objects with properties, arrays of variants, and function
 * objects.
 */
class Variant
{
public:
    typedef int (*Compare)(const Variant*, const Variant*);

    enum Type
    {
        V_EMPTY,
        V_NULL,
        V_INT,
        V_BOOL,
        V_DOUBLE,
        V_STRING,
        V_ARRAY,
        V_OBJECT,
        V_FUNCTION,
        V_POINTER
    };

public:

    /**
     * Default constructor-- an Empty object
     */
    inline Variant()
    {
        mData.type = V_EMPTY;
        mData.flags = 0;
    }

    /**
     * Constructs an Integer object with a longint
     */
    inline Variant(longint i)
    {
        mData.type = V_INT;
        mData.intData = i;
    }

    /**
     * Constructs an Integer object with an int
     */
    inline Variant(int i)
    {
        mData.type = V_INT;
        mData.intData = (longint)i;
    }

    /**
     * Constructs a double object
     */
    inline Variant(double d)
    {
        mData.type = V_DOUBLE;
        mData.dblData = d;
    }

    /**
     * Constructs a boolean object
     */
    inline Variant(bool b)
    {
      mData.type = V_BOOL;
      mData.boolData = b;
    }

    /**
     * Constructs a string object using a std::string
     */
    inline Variant(std::string s)
    {
        mData.type = V_STRING;

        // Inplace new a std::string object with the source string passed to
        // the constructor.

        new (&mData.strMemData) std::string(s);
    }

    /**
     * Constructs a string object
     */
    inline Variant(const char* s)
    {
        mData.type = V_STRING;

        // Inplace new a std::string object with the source string passed to
        // the constructor.

        new (&mData.strMemData) std::string(s);
    }

    /**
     * Copy constructor
     */
    inline Variant(Variant const& src)
    {
        mData.type = V_EMPTY;
        *this = src;
    }

    /**
     * Destructor - deletes all data in the object
     */
    inline ~Variant()
    {
        (void)deleteData();
    }

    /**
     * Assigns a variant by copying it
     */
    inline Variant& operator=(const Variant& src)
    {
        copyFrom(&src);
        return *this;
    }

    /**
     * Assigns a variant by copying it
     */
    inline Variant& operator=(const Variant* src)
    {
        copyFrom(src);
        return *this;
    }

    /**
     * Assigns a string (changes type if needed)
     */
    inline Variant& operator=(const char* src)
    {
        assignStr(src);
        return *this;
    }

    /**
     * Assigns a string (changes type if needed)
     */
    inline Variant& operator=(const std::string& src)
    {
        assignStr(src);
        return *this;
    }

    /**
     * Assigns a bool (changes type if needed)
     */
    inline Variant& operator=(bool src)
    {
        assignBool(src);
        return *this;
    }

    /**
     * Assigns a longint (changes type if needed)
     */
    inline Variant& operator=(longint src)
    {
        assignInt(src);
        return *this;
    }

    /**
     * Assigns a int (changes type if needed)
     */
    inline Variant& operator=(int src)
    {
        assignInt((longint)src);
        return *this;
    }

    /**
     * Assigns a double (changes type if needed)
     */
    inline Variant& operator=(double src)
    {
        assignDbl(src);
        return *this;
    }

    /**
     * Assigns a float (changes type if needed)
     */
    inline Variant& operator=(float src)
    {
        assignDbl((double)src);
        return *this;
    }

    /**
     * @page AddingVariants Adding Variants
     * When adding two variants together using the + operator, the result depends on the types. There
     * are two cases: when the types are same, and when the types are different.  If they are same and
     * if they are ints, the result is an int.  If both are double, the result is a double.  If both
     * are strings, the result is a concatenated string.  If the types are different, the variants are
     * first converted into strings and then concatenated.
     */

    /**
     * @ref AddingVariants
     */
    inline Variant& operator+=(const Variant& rhs)
    {
        internalAdd(*this, rhs);
        return *this;
    }

    /**
     * @ref AddingVariants
     */
    inline Variant& operator+=(longint rhs)
    {
        Variant r(rhs);
        internalAdd(*this, r);
        return *this;
    }

    /**
     * @ref AddingVariants
     */
    inline Variant& operator+=(int rhs)
    {
        Variant r((longint)rhs);
        internalAdd(*this, r);
        return *this;
    }

    /**
     * @ref AddingVariants
     */
    inline Variant& operator+=(const char* rhs)
    {
        Variant r(rhs);
        internalAdd(*this, r);
        return *this;
    }

    /**
     * @ref AddingVariants
     */
    inline Variant& operator+=(const std::string& rhs)
    {
        Variant r(rhs);
        internalAdd(*this, r);
        return *this;
    }

    /**
     * @ref AddingVariants
     */
    inline Variant& operator+=(double rhs)
    {
        Variant r(rhs);
        internalAdd(*this, r);
        return *this;
    }

    /**
     * Prefix (++v)
     * @ref AddingVariants
     */
    inline Variant& operator++()
    {
        Variant r(1);
        internalAdd(*this, r);
        return *this;
    }

    /**
     * Postfix (v++)
     * @ref AddingVariants
     */
    inline Variant operator++(int)
    {
        Variant r(1);
        internalAdd(*this, r);
        return *this;
    }

    /**
     * Return the value as an int
     */
    inline longint toInt() const
    {
        return makeInt();
    }

    /**
     * Return the value as an double
     */
    inline double toDouble() const
    {
        return makeDbl();
    }

    /**
     * Return the value as an bool
     */
    inline bool toBool() const
    {
        return makeBool();
    }

    /**
     * Typecast the value as an int
     */
    inline operator longint() const
    {
        return makeInt();
    }

    /**
     * Typecast the value as an double
     */
    inline operator double() const
    {
        return makeDbl();
    }

    /**
     * Typecast the value as an bool
     */
    inline operator bool() const
    {
        return makeBool();
    }

    /**
     * Returns a pointer to the string.  Returns NULL if not string type.
     */
    inline operator const char*()
    {
        return c_str();
    }

    /**
     * Returns a pointer to the string.  Returns NULL if not string type.
     */
    inline const char* c_str() const
    {
        if (mData.type == V_STRING)
        {
            return mData.strData()->c_str();
        }
        return NULL;
    }

    /**
     * Returns a *reference* to the string object.  If not string type, the type is automatically
     * *changed* to string by this function.
     *
     * NOTE 1: Modifies the type of the object to string
     * NOTE 2: Should not be called with type==V_NULL, or VNULL or VEMPTY.
     *
     * @return Reference to the string object.
     */
    std::string& s();

    /**
     * Returns a *copy* of the string performing any conversions.  Format is similar to
     * json but does not do any escaping (for proper Json strings, use toJsonString()
     *
     * @return Copy of the string
     */
    std::string toString() const;

    /**
     * Returns as a formatted string for a double, otherwise returns a copy of string
     * performing any conversions
     *
     * @param  digs Number of digits to show in double
     *
     * @return      Copy of string
     */
    std::string toFixed(int digs = 0) const;

    /**
     * Returns a json text representing the object.
     *
     * @return String with Json
     */
    std::string toJsonString() const;

    /**
     * Parses json text and loads the data structure into the variant
     *
     * @param  jsontxt Json text string
     *
     * @return         Success
     */
    bool parseJson(const char* jsontxt);

    /**
     * Formats a new string using printf style formatting and assigns it to the variant
     */
    void format(const char* fmt, ...);

    /**
     * Returns the variant type V_xxx for the object
     *
     * @return Variant type
     */
    inline Type type() const
    {
        return (Type)(mData.type);
    }

    /**
     * Returns the variant type of the object as a string
     *
     * @return String containing type name
     */
    const char* typeName() const;

    inline bool isNull() const
    {
        return (mData.type == V_NULL);
    }
    inline bool isEmpty() const
    {
        return (mData.type == V_EMPTY);
    }
    inline bool isObject() const
    {
        return (mData.type == V_OBJECT);
    }
    inline bool isArray() const
    {
        return (mData.type == V_ARRAY);
    }
    inline bool isPointer() const
    {
        return (mData.type == V_POINTER);
    }
    inline bool isNaN()
    {
        if (mData.type == V_DOUBLE)
        {
            return isnan(mData.dblData);
        }
        else if (mData.type == V_INT)
        {
            return false;
        }
        return true;
    }

    /**
     * Returns the length.  If an array, returns array length.  If an object, returns
     * the number of properties.  Otherwise, returns 1.
     */
    int length() const;

    /**
     * Returns a referene to the variant in an array
     */
    Variant& operator[](int i);

    /**
     * Returns a const referene to the variant in an array
     */
    const Variant& operator[](int i) const;

    /**
     * Returns a referene to the variant inside an object or a fuction using key
     */
    Variant& operator[](const char* key);

    /**
     * Returns a const referene to the variant inside an object or a fuction using key
     */
    const Variant& operator[](const char* key) const;

    /**
     * Returns a refernece to a variant by parsing properties and indexes using a path
     * syntax (ex: "obj.propA.2.name")
     *
     * @param  pathkey [description]
     *
     * @return         [description]
     */
    Variant& path(const char* pathkey);

    /**
     * Returns a refernece to a variant by parsing properties and indexes using a path
     * syntax (ex: "obj.propA.2.name")
     *
     * @param  pathkey [description]
     *
     * @return         [description]
     */
    inline Variant& path(const std::string& pathkey)
    {
        return path(pathkey.c_str());
    }

    /**
     * Creates an array
     *
     * @param initvalue Optional JSON to initialize the array
     */
    void createArray(const char* initvalue = NULL);

    /**
     * Adds a variant at the end of an array
     *
     * @param  elem Variant to add
     *
     * @return      Pointer to the newly added variant
     */
    Variant* append(const Variant& elem = VEMPTY);

    /**
     * Adds an variant at the end of an array
     */
    inline void push(const Variant& elem)
    {
        (void)append(elem);
    }

    /**
     * Removes the last item from the array and returns it
     *
     * @return Copy of the last element
     */
    Variant pop();

    /**
     * Removes the first item from the array and returns it
     *
     * @return Copy of first item
     */
    Variant shift();

    /**
     * Sorts an array
     *
     * @param comp Optional compare function
     */
    void sort(Compare comp);

    /**
     * Creates an objects with optional initial value
     *
     * @param initvalue Json style initial value
     */
    void createObject(const char* initvalue = NULL);

    /**
     * Adds a new property to the object
     *
     * @param  key   Key name for the property
     * @param  value Optional value
     *
     * @return       Reference to the value in object
     */
    Variant& addProperty(const char* key, const Variant& value = vEmpty);

    /**
     * Determines if the object has a property
     *
     * @param  key Key name for the property
     *
     * @return     True if found, false otherwise
     */
    bool hasProperty(const char* key);

    /**
     * Remove a property
     *
     * @param  key Key name for the property
     *
     * @return     True if removed, false otherwise
     */
    bool removeProperty(const char* key);

    /**
     * Returns the key name at a given index
     *
     * @param  n Index position
     *
     * @return   A pointer to property key name
     */
    const char* getKey(int n);

    /**
     * Creates a function object]
     *
     * @param func Function pointer
     */
    void createFunction(Variant (*func)(Variant& env, Variant& arg));

    /**
     * Adds a variable to the enviroment of the function object
     *
     * @param  varname Key name for the variable
     * @param  value   Optional value for the variable
     *
     * @return         Success
     */
    bool addEnv(const char* varname, const Variant& value = vEmpty);
    Variant operator() ();
    Variant operator() (const Variant& value1);
    Variant operator() (const Variant& value1, const Variant& value2);
    Variant operator() (const Variant& value1, const Variant& value2, const Variant& value3);
    Variant operator() (const Variant& value1, const Variant& value2, const Variant& value3, const Variant& value4);

    /**
     * Returns an iterator to go over all elements in an array or object
     *
     * @param  iter Iterator
     *
     * @return      Success
     */
    inline bool forEach(Iter<Variant>& iter)
    {
        if (mData.type == V_ARRAY)
        {
            return mData.arrayData->forEach(iter);
        }
        else if (mData.type == V_OBJECT)
        {
            return mData.objectData->forEach(iter);
        }
        return false;
    }

    /**
     * Clears the object by deleting all data
     */
    inline void clear()
    {
        (void)deleteData();
    }

    /**
     * Sets the modified flag
     */
    inline void setModified()
    {
        setFlag(mData.flags, VF_MODIFIED);
    }

    /**
     * Clears the modified flag
     */
    inline void clearModified()
    {
        clearFlag(mData.flags, VF_MODIFIED);
    }

    /**
     * Checks modified flag
     */
    inline bool isModified()
    {
        return isFlagSet(mData.flags, VF_MODIFIED);
    }

/** \cond INTERNAL */
    void setSuppImplData(const char* name, void* supp);
    void getSuppImplData(const char** name, void** supp);
/** \endcond */

private:

    enum Flags
    {
        VF_MODIFIED = 0x1
    };

    #pragma pack(push, 4)
    struct VarData
    {
        ushortint type;
        shortint flags;
        union
        {
            longint intData;
            double dblData;
            bool boolData;
            char strMemData[sizeof(std::string)];
            ObjArray<Variant>* arrayData;
            PropArray<Variant>* objectData;
            VarFuncObj* funcData;
            Variant* vptrData;
        };

        std::string* strData() const
        {
            return (std::string*)&strMemData;
        }
    };
    #pragma pack(pop)

    VarData mData;

private:

    bool deleteData();
    void copyFrom(const Variant* src);

    void assignInt(longint src);
    void assignDbl(double src);
    void assignBool(bool src);
    void assignStr(const char* src);
    void assignStr(const std::string& src);

    longint makeInt() const;
    double makeDbl() const;
    inline bool makeBool() const
    {
        return makeInt() == 0 ? false : true;
    }
    void makeString(std::string& s, int level, bool json);

    void jsonifyStr(std::string& s);
    inline void appendQuote(std::string& s, Type type)
    {
        if (type == V_STRING)
        {
            s += '"';
        }
    }
    inline void appendNewline(std::string& s, int level, bool json)
    {
        if (json)
        {
            s += '\n';
            s += std::string(level, '\t');
        }
    }

    static const KeywordArray::Entry sTypeNames[];

public:
    static Variant vEmpty;
    static Variant vNull;

public:
/** \cond INTERNAL */

    Variant(Type type)
    {
        mData.type = type;
    }

    void internalAdd(const Variant& lhs, const Variant& rhs);
    void internalSetPtr(const Variant* v);

/** \endcond */
};

/** \cond INTERNAL */
class VarFuncObj
{
public:
    VarFuncObj() :
        mFunc(NULL)
    {
    }

    Variant mEnv;
    Variant (*mFunc)(Variant& env, Variant& arg);
};
/** \endcond */

/**
 * Global operator to add two variants
 *
 * @ref AddingVariants
 */
inline Variant operator+(const Variant& lhs, const Variant& rhs)
{
    Variant res;
    res.internalAdd(lhs, rhs);
    return res;
}

/**
 * Global operator to add a variant and string
 *
 * @ref AddingVariants
 */
inline Variant operator+(const Variant& lhs, const char* rhs)
{
    Variant res;
    Variant r(rhs);
    res.internalAdd(lhs, r);
    return res;
}

/**
 * Global operator to add a string and a variant
 *
 * @ref AddingVariants
 */
inline Variant operator+(const char* lhs, const Variant& rhs)
{
    Variant res;
    Variant l(lhs);
    res.internalAdd(l, rhs);
    return res;
}

/**
 * Global operator to add a string and a variant
 *
 * @ref AddingVariants
 */
inline Variant operator+(const std::string& lhs, const Variant& rhs)
{
    Variant res;
    Variant l(lhs);
    res.internalAdd(l, rhs);
    return res;
}

/**
 * Global operator to add a variant and string
 *
 * @ref AddingVariants
 */
inline Variant operator+(const Variant& lhs, const std::string& rhs)
{
    Variant res;
    Variant r(rhs);
    res.internalAdd(lhs, r);
    return res;
}

/**
 * Global operator to add a variant and long int
 *
 * @ref AddingVariants
 */
inline Variant operator+(const Variant& lhs, longint rhs)
{
    Variant res;
    Variant r(rhs);
    res.internalAdd(lhs, r);
    return res;
}

/**
 * Global operator to add a variant and an int
 *
 * @ref AddingVariants
 */
inline Variant operator+(const Variant& lhs, int rhs)
{
    return operator+(lhs, (longint)rhs);
}

/**
 * Global operator to add a variant and a double
 *
 * @ref AddingVariants
 */
inline Variant operator+(const Variant& lhs, double rhs)
{
    Variant res;
    Variant r(rhs);
    res.internalAdd(lhs, r);
    return res;
}

/**
 * Global operator to add a double and a variant
 *
 * @ref AddingVariants
 */
inline Variant operator+(double lhs, const Variant& rhs)
{
    Variant res;
    Variant l(lhs);
    res.internalAdd(l, rhs);
    return res;
}

} // jvar

#endif // _VAR_H