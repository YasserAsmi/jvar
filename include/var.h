/**
 * @file include/var.h
 * Declares the Variant type
 * @copyright Copyright (c) 2014 Yasser Asmi; Released under the MIT
 *            License (http://opensource.org/licenses/MIT)
 */

#ifndef _VAR_H
#define _VAR_H

#include "util.h"
#include "arr.h"
#include "str.h"
#include <math.h>

#define VNULL Variant::sNull
#define VEMPTY Variant::sEmpty

#if __cplusplus > 199711L
#include <initializer_list>
#endif

#define VAR_PATH_DELIM     "."

namespace jvar
{

class Variant;
class VarFuncObj;

/** \cond INTERNAL */
class VarExtInterface : public jvar::BaseInterface
{
public:
    INTF_NAME("VarExtInterface");
    INTF_CAST(
        IID(BaseInterface)
        IID(VarExtInterface)
        );

    virtual void onAppend(Variant& arr, Variant& newelem);
    virtual bool onNewExt(Variant& destobj, Variant& param);
    virtual bool onSaveExt(Variant& sobj);
    virtual bool onLoadExt(Variant& destobj, Variant& param);
    virtual Variant* onAddMissingKey(Variant& destobj, const char* key);

};
/** \endcond */

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
        V_EMPTY,  ///< The Variant is empty.
        V_NULL,   ///< The Variant is null.
        V_INT,    ///< The Variant is an integer.
        V_BOOL,   ///< The Variant is a boolean.
        V_DOUBLE, ///< The Variant is a double-precision floating point value.
        V_STRING, ///< The Variant is a string.
        V_ARRAY,  ///< The Variant is a JSON array.
        V_OBJECT, ///< The Variant is a JSON object.
        V_FUNCTION, ///< The Variant is a function.
        V_POINTER ///< The Variant is a pointer.
    };

public:

    /**
     * Default constructor-- an Empty object
     */
    Variant()
    {
        mData.type = V_EMPTY;
        mData.flags = 0;
    }

    /**
     * Constructs an Integer object with a longint
     */
    Variant(longint i)
    {
        mData.type = V_INT;
        mData.intData = i;
    }

    /**
     * Constructs an Integer object with an int
     */
    Variant(int i)
    {
        mData.type = V_INT;
        mData.intData = (longint)i;
    }

    /**
     * Constructs an double object
     */
    Variant(double d)
    {
        mData.type = V_DOUBLE;
        mData.dblData = d;
    }

    /**
     * Constructs an string object using a std::string
     */
    Variant(std::string s)
    {
        mData.type = V_STRING;

        // Inplace new a std::string object with the source string passed to
        // the constructor.

        new (&mData.strMemData) std::string(s);
    }

    /**
     * Constructs an string object
     */
    Variant(const char* s)
    {
        mData.type = V_STRING;

        // Inplace new a std::string object with the source string passed to
        // the constructor.

        new (&mData.strMemData) std::string(s);
    }

    /**
     * Copy constructor
     */
    Variant(Variant const& src)
    {
        mData.type = V_EMPTY;
        *this = src;
    }

#if __cplusplus > 199711L

    /**
     * Assigns an object literal represented by initializer_list
     */
    Variant(std::initializer_list<const Variant>&& src)
    {
        assignObj(src);
    }

#endif


    /**
     * Destructor deletes all data in the object
     */
    ~Variant()
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
    inline operator const longint()
    {
        return toInt();
    }

    /**
     * Typecast the value as an double
     */
    inline operator double() const
    {
        return makeDbl();
    }
    inline operator const double() const
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
     * Returns a pointer to the string.
     *
     * NOTE: Returns NULL if not string type.
     */
    inline operator const char*()
    {
        return c_str();
    }

    /**
     * Type cast to a copy of a string
     *
     * @return [description]
     */
    operator const std::string() const
    {
        return toString();
    }

    /**
     * Returns a pointer to the string.
     *
     * NOTE:Returns NULL if not string type.
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
     * Returns a *copy* of the string performing any conversions --similar to String().
     * However, an empty string ("") is returned for NULL and EMPTY.
     *
     * @return Copy of the string
     */
    std::string toStrE() const;

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
    void makeJson(StrBld& sb) const;

    /**
     * Parses json text and loads the data structure into the variant
     *
     * @param  jsontxt Json text string
     *
     * @return         Success
     */
    bool parseJson(const char* jsontxt);

    bool eq(const char* str);

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

    /**
     * Returns true if the object is empty or null.  It is prefered way
     * to check that the variant doesn't have data.
     */
    bool empty() const
    {
        return (mData.type == V_NULL) || (mData.type == V_EMPTY);
    }

    /**
     * (Deprecated) Returns true if the type is V_NULL. Please use empty() instead
     * of this function as there are cases where a Variant may be V_EMPTY even
     * though it is being set to V_NULL.
     */
    inline bool isNull() const
    {
        return (mData.type == V_NULL);
    }
    /**
     * (Deprecated) Returns true if the type is V_EMPTY. Please use empty() instead
     * of this function.
     */
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
    inline bool isString() const
    {
        return (mData.type == V_STRING);
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
     * Returns a reference to the variant in an array
     */
    Variant& operator[](int i);

    /**
     * Returns a const reference to the variant in an array
     */
    const Variant& operator[](int i) const;

    /**
     * Returns a reference to the variant inside an object or a function using key
     */
    Variant& operator[](const char* key);

    Variant& operator[](const std::string& key)
    {
        return this->operator[](key.c_str());
    }

    /**
     * Returns a const reference to the variant inside an object or a function using key
     */
    const Variant& operator[](const char* key) const;

    const Variant& operator[](const std::string& key) const
    {
        return this->operator[](key.c_str());
    }

    /**
     * Returns a reference to a variant by parsing properties and indexes using a path
     * syntax with '.' separator (ex: "obj.propA.2.name")
     *
     * @param  pathkey property names separated by '.'
     *
     * @return         Reference to the element
     */
    Variant& path(const char* pathkey);

    inline Variant& path(const std::string& pathkey)
    {
        return path(pathkey.c_str());
    }

    Variant& p(const char* pathkey)
    {
        return path(pathkey);
    }
    /**
     * Returns a string value for the variant by parsing properties and indexes using a path
     * syntax with '.' separator (ex: "obj.propA.2.name")
     *
     * @param  pathkey property names separated by '.'
     *
     * @return         String value of the element
     */
    std::string ps(const char* pathkey)
    {
        return path(pathkey).toStrE();
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

    Variant& appendr()
    {
        Variant* v = append();
        return (v != NULL) ? *v : Variant::sNull;
    }

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

    int indexOf(const char* str);
    int indexOf(const std::string s)
    {
        return indexOf(s.c_str());
    }
    int lastIndexOf(const char* str);
    int lastIndexOf(const std::string s)
    {
        return lastIndexOf(s.c_str());
    }

    /**
     * Splits a string separated a separators and returns the substrings in the array
     * Whitespace is ignored unless inside quotes
     * Separator cannot be whitespace
     * Separator is ignored if inside quotes
     *
     * @param str String to split
     * @param sep Separator
     */
    void split(const char* str, const char* sep);

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
    Variant& addProperty(const char* key, const Variant& value = VEMPTY);
    Variant& addProperty(const std::string key, const Variant& value = VEMPTY)
    {
        return addProperty(key.c_str(), value);
    }

    /**
     * Adds or modifies (if already exists) property to the object
     *
     * @param  key   Key name for the property
     *
     * @return       Reference to the value in object
     */
    Variant& addOrModifyProperty(const char* key);

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
     * Adds a variable to the environment of the function object
     *
     * @param  varname Key name for the variable
     * @param  value   Optional value for the variable
     *
     * @return         Success
     */
    bool addEnv(const char* varname, const Variant& value = VEMPTY);

#if __cplusplus > 199711L
    /**
     * Executes the function object with any number of parameters.
     *
     * This method is called with a brace-enclosed parameter list - see
     * \ref func.cpp for an example.
     *
     * The overloads of operator() taking zero to four parameters call this
     * method to do real work and return the result. They can therefore
     * be used as shorthand if only four arguments are needed.
     */
    Variant operator() (std::initializer_list<const jvar::Variant>&& values);
#endif

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

    /**
     * Make property lookup for object, case insensitive.
     */
    inline void makeCI()
    {
        if (mData.type == V_OBJECT)
        {
            mData.objectData->makeCI();
        }
    }

    /**
     * Instruct the object to automatically add a property if missing (like JS)
     */
    inline void enableAutoAdd()
    {
        if (mData.type == V_OBJECT)
        {
            setFlag(mData.flags, VF_AUTOADDPROP);
        }
    }

    /**
     * Don't report an error if a property is missing
     */
    inline void disableMissingErr()
    {
        setFlag(mData.flags, VF_NOMISSINGKEYERR);
    }

    bool readJsonFile(const char* filename);
    inline bool readJsonFile(const std::string& filename)
    {
        return readJsonFile(filename.c_str());
    }

    void newFrom(Variant param);
    void save();
    void load(Variant param);

/** \cond INTERNAL */
    jvar::RcLife<jvar::BaseInterface>& extInterface();
/** \endcond */

private:

    enum Flags
    {
        VF_MODIFIED = 0x1,
        VF_NOMISSINGKEYERR = 0x2,
        VF_AUTOADDPROP = 0x4
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
            assert(type == V_STRING);
            return (std::string*)&strMemData;
        }
    };
    #pragma pack(pop)

    VarData mData;

private:

    /**
     * Frees all memory related to the data inside the Variant (probably).
     */
    bool deleteData();

    /**
     * Copy the data from \p src into this Variant.
     */
    void copyFrom(const Variant* src);

    /**
     * Assign a \ref longint to this Variant.
     */
    void assignInt(longint src);

    /**
     * Assign a double to this Variant.
     */
    void assignDbl(double src);

    /**
     * Assign a bool to this Variant.
     */
    void assignBool(bool src);

    /**
     * Assign a C-style string to this Variant.
     */
    void assignStr(const char* src);

    /**
     * Assign a C++ string to this Variant.
     */
    void assignStr(const std::string& src);

#if __cplusplus > 199711L

    /**
     * Assign an object defined by initializer list to this Variant.
     */
    void assignObj(std::initializer_list<const Variant>& src);

#endif

    /**
     * Coerce the data in this Variant to a \ref longint.
     */
    longint makeInt() const;

    /**
     * Coerce the data in this Variant to a double.
     */
    double makeDbl() const;

    /**
     * Coerce the data in this Variant to a boolean by first converting it
     * to a \ref longint.
     */
    inline bool makeBool() const
    {
        return makeInt() == 0 ? false : true;
    }
	/**
     * Coerce the data in this Variant to a C++ string.
     * @param[out] s the string to fill.
     */
    void makeString(StrBld& s, int level, bool json);
	/**
     * Escape special characters in \p s.
     */
    void jsonifyStr(StrBld& s);

    inline void appendQuote(StrBld& s, Type type)
    {
        if (type == V_STRING)
        {
            s.append('"');
        }
    }
	 /**
     * JSON formatting helper (adds newlines and indentation).
     */
    inline void appendNewline(StrBld& s, int level, bool json)
    {
        if (json)
        {
            s.append('\n');
            //s.append(std::string(level, '\t'));
            for (int i = 0; i < level; i++)
            {
                s.append('\t');
            }
        }
    }

    Variant* handleMissingKey(const char* key);

    static const KeywordArray::Entry sTypeNames[];

public:
/** \cond INTERNAL */
    static Variant sEmpty;
    static Variant sNull;
    static jvar::RcLife<jvar::BaseInterface> sNullExtIntf;

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


/**
 * Global operator to compare a variant and string
 */
inline bool operator==(const Variant& lhs, const std::string& rhs)
{
    return lhs.toString() == rhs;
}
inline bool operator!=(const Variant& lhs, const std::string& rhs) { return !(lhs == rhs); }

/**
 * Global operator to compare a variant and const char *
 */
inline bool operator==(const Variant& lhs, const char *rhs)
{
    return strcmp(lhs.toString().c_str(), rhs) == 0;
}
inline bool operator!=(const Variant& lhs, const char *rhs) { return !(lhs == rhs); }



} // jvar



#endif // _VAR_H
