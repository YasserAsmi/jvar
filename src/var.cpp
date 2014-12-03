// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "var.h"
#include "json.h"

#include <initializer_list>

namespace jvar
{

Variant Variant::vEmpty;
Variant Variant::vNull(Variant::V_NULL);

const KeywordArray::Entry Variant::sTypeNames[] =
{
    {"empty", Variant::V_EMPTY},
    {"null", Variant::V_NULL},
    {"int", Variant::V_INT},
    {"bool", Variant::V_BOOL},
    {"double", Variant::V_DOUBLE},
    {"string", Variant::V_STRING},
    {"array", Variant::V_ARRAY},
    {"object", Variant::V_OBJECT},
    {"function", Variant::V_FUNCTION},
    {"pointer", Variant::V_POINTER}
};


bool Variant::parseJson(const char* jsontxt)
{
    if (mData.type == V_NULL)
    {
        return false;
    }

    JsonParser json(*this, jsontxt);
    setModified();
    return !json.failed();
}


void Variant::format(const char* fmt, ...)
{
    va_list va;
    std::string outstr;

    if (mData.type == V_NULL)
    {
        return;
    }

    va_start(va, fmt);
    bool ret = vformat(outstr, fmt, va);
    va_end(va);

    if (!ret)
    {
        dbgerr("String format failed\n");
        return;
    }

    assignStr(outstr);
}


longint Variant::makeInt() const
{
    switch (mData.type)
    {
        case V_INT:
            return mData.intData;

        case V_BOOL:
            return (longint)mData.boolData;

        case V_DOUBLE:
            return (longint)mData.dblData;

        case V_STRING:
        {
            std::string* strdata = mData.strData();
            char* end;
            errno = 0;
            long int value = strtol(strdata->c_str(), &end, 10);
            if ((errno != 0 && value == 0) || (*end != '\0'))
            {
                value = 0;
            }
            return value;
        }

        case V_POINTER:
        {
            if (mData.vptrData != NULL)
            {
                return mData.vptrData->makeInt();
            }
            else
            {
                return 0;
            }
        }

        default:
            return 0;
    }
}


double Variant::makeDbl() const
{
    switch (mData.type)
    {
        case V_INT:
            return (double)mData.intData;

        case V_BOOL:
            return (double)mData.boolData;

        case V_DOUBLE:
            return mData.dblData;

        case V_STRING:
        {
            std::string* strdata = mData.strData();
            char* end;
            double value = strtod(strdata->c_str(), &end);
            if (*end != '\0')
            {
                value = 0.0;
            }
            return value;
        }

        case V_POINTER:
        {
            if (mData.vptrData != NULL)
            {
                return mData.vptrData->makeDbl();
            }
            else
            {
                return 0;
            }
        }

        default:
            return 0.0;
    }
}


void Variant::makeString(std::string& s, int level, bool json)
{
    switch (mData.type)
    {
        case V_STRING:
        {
            //TODO: use json flag
            s = *(mData.strData());

            if (json)
            {
                jsonifyStr(s);
            }
        }
        break;

        case V_INT:
        {
            jvar::format(s, "%ld", mData.intData);
        }
        break;

        case V_DOUBLE:
        {
            jvar::format(s, "%lg", mData.dblData);
            if (s.find_first_of('.') == std::string::npos)
            {
                s += ".0";
            }
        }
        break;

        case V_BOOL:
        {
            s = mData.boolData ? "true" : "false";
        }
        break;

        case V_EMPTY:
        {
            //TODO: Revisit this value--should work for json
            s = "null";
        }
        break;

        case V_NULL:
        {
            s = "null";
        }
        break;

        case V_ARRAY:
        {
            s += '[';
            level++;
            for (Iter<Variant> i; forEach(i); )
            {
                if (i.pos() != 0)
                {
                    s += ",";
                }

                appendNewline(s, level, json);

                appendQuote(s, i->type());

                std::string tmps;
                i->makeString(tmps, level, json);

                s += tmps;
                appendQuote(s, i->type());

            }
            level--;
            appendNewline(s, level, json);
            s += ']';
        }
        break;

        case V_OBJECT:
        {
            s += '{';

            level++;
            for (Iter<Variant> i; mData.objectData->forEach(i); )
            {

                if (i.pos() != 0)
                {
                    s += ",";
                }

                appendNewline(s, level, json);

                appendQuote(s, V_STRING);

                if (json)
                {
                    std::string keys = i.key();
                    jsonifyStr(keys);
                    s += keys;
                }
                else
                {
                    s += i.key();
                }
                appendQuote(s, V_STRING);

                s += ':';

                appendQuote(s, i->type());

                std::string tmps;
                i->makeString(tmps, level, json);

                s += tmps;
                appendQuote(s, i->type());

            }
            level--;
            appendNewline(s, level, json);
            s += '}';
        }
        break;

        case V_FUNCTION:
        {
            s = "(function)";
        }
        break;

        case V_POINTER:
        {
            if (mData.vptrData != NULL)
            {
                mData.vptrData->makeString(s, level, json);
            }
        }

        default:
        {
            dbgerr("TODO: makeString not handled for type %d\n", mData.type);
        }
    }

    level--;
}


void Variant::jsonifyStr(std::string& s)
{
    std::string ts;

    //dbglog("jsonify: %s\n", s.c_str());

    // First check if the string needs any esc chars

    bool escneeded = false;
    const char* p = s.c_str();
    for (int i = 0; i < (int)s.length(); i++)
    {
        uint c = (unsigned char)p[i];
        if (c >= 0x80 || (c != '/' && strfind(ESCAPE_CHARS, c, NULL)))
        {
            escneeded = true;
            break;
        }
    }

    if (!escneeded)
    {
        return;
    }

    // Build a new string replacing with json style esc chars

    p = s.c_str();
    for (int i = 0; i < (int)s.length(); i++)
    {
        uint c = (unsigned char)p[i];

        if (c >= 0x80)
        {
            int lused = 0;
            c = makeUnicode(&p[i], (int)s.length() - i, &lused);

            std::string hex;
            jvar::format(hex, "\\u%04X", c);
            ts += hex;

            i += lused - 1;
        }
        else
        {
            // If it is an esc char, we want to escape with a proper code.
            // NOTE: We make an exception for / here even though spec calls for it to be escaped

            int pos;
            if (c != '/' && strfind(ESCAPE_CHARS, c, &pos))
            {
                c = ESCAPE_CODES[pos];
                ts += '\\';
                ts += c;
            }
            else
            {
                ts += (char)c;
            }
        }
    }

    // Return the string

    ts.swap(s);
}

void Variant::createArray(const char* initvalue /*= 0*/)
{
    if (deleteData())
    {
        if (initvalue == NULL)
        {
            mData.type = V_ARRAY;
            mData.arrayData = new ObjArray<Variant> ();
        }
        else
        {
            JsonParser json(*this, initvalue, JsonParser::FLAG_FLEXQUOTES | JsonParser::FLAG_ARRAYONLY);
        }
     }
    else
    {
        dbgerr("createArray() failed\n");
    }
}

Variant* Variant::append(const Variant& elem)
{
    if (mData.type != V_ARRAY)
    {
        return NULL;
    }

    Variant* newelem = mData.arrayData->append();
    if (elem.type() != V_EMPTY)
    {
        newelem->copyFrom(&elem);
    }
    setModified();

    return newelem;
}

Variant Variant::pop()
{
    Variant ret;

    if (isArray())
    {
        if (mData.arrayData->length() > 0)
        {
            ret = mData.arrayData->get(mData.arrayData->length() - 1);
            mData.arrayData->remove(mData.arrayData->length() - 1);
        }
    }
    else
    {
        dbgerr("Cannot pop() a non-array\n");
    }

    return (ret.isEmpty() ? VNULL : ret);
}

Variant Variant::shift()
{
    Variant ret;

    if (isArray())
    {
        if (mData.arrayData->length() > 0)
        {
            ret = mData.arrayData->get(0);
            mData.arrayData->remove(0);
        }
    }
    else
    {
        dbgerr("Cannot shift() a non-array\n");
    }

    return (ret.isEmpty() ? VNULL : ret);
}

void Variant::sort(Compare comp)
{
    if (!isArray())
    {
        dbgerr("Cannot sort() a non-array\n");
        return;
    }

    mData.arrayData->sort((BArray::Compare)comp);
}


void Variant::createObject(const char* initvalue /* = NULL*/)
{
    if (deleteData())
    {
        if (initvalue == NULL)
        {
            mData.type = V_OBJECT;
            mData.objectData = new PropArray<Variant> ();
        }
        else
        {
            JsonParser json(*this, initvalue, JsonParser::FLAG_FLEXQUOTES | JsonParser::FLAG_OBJECTONLY);
        }
    }
    else
    {
        dbgerr("createObject() failed\n");
    }
}


Variant& Variant::addProperty(const char* key, const Variant& value /* = vEmpty */)
{
    assert(key);

    if (mData.type != V_OBJECT)
    {
        dbgerr("addProperty(%s) failed -- not an object\n", key);
        return VNULL;
    }

    Variant* newprop = mData.objectData->add(key);
    if (!newprop)
    {
        dbgerr("failed to add property with key '%s'\n", key);
        return VNULL;
    }

    assert(newprop->type() == V_EMPTY);
    if (value.type() != V_EMPTY)
    {
        newprop->copyFrom(&value);
    }

    setModified();

    return *newprop;
}


bool Variant::removeProperty(const char* key)
{
    assert(key);

    if (mData.type == V_OBJECT)
    {
        if (mData.objectData->remove(key))
        {
            setModified();
            return true;
        }
    }
    return false;
}


const char* Variant::getKey(int n)
{
    if (mData.type == V_OBJECT)
    {
        return mData.objectData->getKey(n);
    }
    return NULL;
}

bool Variant::hasProperty(const char* key)
{
    assert(key);

    if (mData.type == V_OBJECT)
    {
        if (mData.objectData->get(key) != NULL)
        {
            return true;
        }
    }
    return false;
}

Variant& Variant::operator[](const char* key)
{
    assert(key);

    if (mData.type == V_OBJECT)
    {
        Variant* v = mData.objectData->get(key);
        if (v)
        {
            return *v;
        }
        else
        {
            dbgerr("[%s] not found, %p\n", key, (void*)this);
        }
    }
    else if (mData.type == V_FUNCTION)
    {
        return mData.funcData->mEnv[key];
    }
    else
    {
        dbgerr("[%s] failed--not an object or func\n", key);
    }
    return vNull;
}


const Variant& Variant::operator[](const char* key) const
{
    assert(key);

    if (mData.type == V_OBJECT)
    {
        Variant* v = mData.objectData->get(key);
        if (v)
        {
            return *v;
        }
        else
        {
            dbgerr("[%s] not found\n", key);
        }
    }
    else if (mData.type == V_FUNCTION)
    {
        return mData.funcData->mEnv[key];
    }
    else
    {
        dbgerr("[%s] failed--not an object or func\n", key);
    }
    return vNull;
}

Variant& Variant::operator[](const std::string key)
{
  return (*this)[key.c_str()];
}

const Variant& Variant::operator[](const std::string key) const
{
  return (*this)[key.c_str()];
}

Variant& Variant::path(const char* pathkey)
{
    assert(pathkey);

    //dbgtrc("path: '%s'\n", pathkey);
    Variant* v = this;

    if (pathkey[0] != '\0')
    {
        Parser p(pathkey);
        while (!p.eof())
        {
            if (!p.tokenEquals(VAR_PATH_DELIM))
            {
                if (v->isArray())
                {
                    bool valid;
                    int n = str2int(p.token(), &valid);
                    if (valid)
                    {
                        v = &((*v)[n]);
                    }
                    else
                    {
                        v = &vNull;
                    }
                }
                else
                {
                    v = &((*v)[p.token().c_str()]);
                }
            }
            p.advance();
        }
    }
    return *v;
}


const char* Variant::typeName() const
{
    KeywordArray km(sTypeNames, countof(sTypeNames));
    return km.toKeyword(mData.type);
}

int Variant::length() const
{
   if (mData.type == V_ARRAY)
    {
        return mData.arrayData->length();
    }
    else if (mData.type == V_OBJECT)
    {
        return mData.objectData->length();
    }
    return 1;
}

Variant& Variant::operator[](int i)
{
    if (mData.type == V_ARRAY)
    {
        Variant* v = mData.arrayData->get(i);
        if (v)
        {
            return *v;
        }
    }
    else if (mData.type == V_OBJECT)
    {
        Variant* v = mData.objectData->get(i);
        if (v)
        {
            return *v;
        }
    }
    else
    {
        dbgerr("[%d] failed--not an object or array\n", i);
    }
    return vNull;
}

const Variant& Variant::operator[](int i) const
{
    if (mData.type == V_ARRAY)
    {
        Variant* v = mData.arrayData->get(i);
        if (v)
        {
            return *v;
        }
    }
    else if (mData.type == V_OBJECT)
    {
        Variant* v = mData.objectData->get(i);
        if (v)
        {
            return *v;
        }
    }
    else
    {
        dbgerr("[%d] failed--not an object or array\n", i);
    }
    return vNull;
}


void Variant::createFunction(Variant (*func)(Variant& env, Variant& arg))
{
    if (deleteData())
    {
        mData.type = V_FUNCTION;
        mData.funcData = new VarFuncObj ();
        mData.funcData->mEnv.createObject();
        mData.funcData->mFunc = func;
    }
    else
    {
        dbgerr("createFunction() failed\n");
    }
}

Variant Variant::operator() (std::initializer_list<const jvar::Variant>&& values)
{
    if (mData.type != V_FUNCTION) return VNULL;

    Variant arg;
    Variant* p;

    arg.createArray();

    for(const auto& value : values)
    {
      p = arg.append(value);
    }

    return mData.funcData->mFunc(mData.funcData->mEnv, arg);
}

Variant Variant::operator() ()
{
    return (*this)({});
}

Variant Variant::operator() (const Variant& value1)
{
    return (*this)({value1});
}

Variant Variant::operator() (const Variant& value1, const Variant& value2)
{
    return (*this)({value1, value2});
}

Variant Variant::operator() (const Variant& value1, const Variant& value2, const Variant& value3)
{
    return (*this)({value1, value2, value3});
}


Variant Variant::operator() (const Variant& value1, const Variant& value2, const Variant& value3,
    const Variant& value4)
{
    return (*this)({value1, value2, value3, value4});
}

bool Variant::addEnv(const char* varname, const Variant& value /* = vEmpty */)
{
    if (mData.type != V_FUNCTION)
    {
        dbgerr("cannot set env value--not a function");
        return false;
    }

    mData.funcData->mEnv.addProperty(varname, value);

    return true;
}


bool Variant::deleteData()
{
    // Note: this check is important.  Nothing can be written to NULL.  Any caller
    // trying to write to variant first will call this function.  If the call fails
    // the caller will not write data.

    if (mData.type == V_NULL)
    {
        return false;
    }

    //assert(this != &VNULL);
    //assert(this != &VEMPTY);

    switch (mData.type)
    {
        case V_STRING:
        {
            std::string* strdata = mData.strData();

            // Call the string destructor on the inplace newed object.
            strdata->std::string::~string();
        }
        break;

        case V_ARRAY:
        {
            delete mData.arrayData;
        }
        break;

        case V_OBJECT:
        {
            delete mData.objectData;
        }
        break;

        case V_FUNCTION:
        {
            delete mData.funcData;
        }
        break;

    }
    mData.type = V_EMPTY;
    return true;
}


void Variant::copyFrom(const Variant* src)
{
    if (this != src)
    {
        if (!deleteData())
        {
            return;
        }

        mData.type = src->mData.type;
        setModified();

        switch (src->mData.type)
        {
            case V_INT:
            {
                mData.intData = src->mData.intData;
            }
            break;

            case V_BOOL:
            {
                mData.boolData = src->mData.boolData;
            }
            break;

            case V_DOUBLE:
            {
                mData.dblData = src->mData.dblData;
            }
            break;

            case V_STRING:
            {
                //TODO: Should not deleteData() above and new here if already string type

                // Inplace new a std::string object with the source string passed to
                // the constructor.

                new (&mData.strMemData) std::string(*(src->mData.strData()));
            }
            break;

            case V_ARRAY:
            {
                // Create the array object using the copy constuctor.

                mData.arrayData = new ObjArray<Variant>(*(src->mData.arrayData));
            }
            break;

            case V_OBJECT:
            {
                // Create the proparray object using the copy constuctor.

                mData.objectData = new PropArray<Variant>(*(src->mData.objectData));
            }
            break;

            case V_FUNCTION:
            {
                // Create the function object using the copy constuctor.

                mData.funcData = new VarFuncObj(*(src->mData.funcData));
            }
            break;

            case V_POINTER:
            {
                // We simply need to copy the pointer value.

                mData.vptrData = src->mData.vptrData;
            }
            break;

            case V_EMPTY:
            case V_NULL:
            {
                // do nothing.
            }
            break;

            default:
            {
                dbgerr("TODO: copyfrom missing %d\n", src->mData.type);

                assert(false);
            }
        }
    }
}

void Variant::assignStr(const char* src)
{
    if (mData.type == V_STRING)
    {
        mData.strData()->assign(src);
        setModified();
    }
    else
    {
        if (deleteData())
        {
            mData.type = V_STRING;

            // Inplace new a std::string object with the source string passed to
            // the constructor.

            new (&mData.strMemData) std::string(src);

            setModified();
        }
    }
}

void Variant::assignStr(const std::string& src)
{
    if (mData.type == V_STRING)
    {
        mData.strData()->assign(src);
        setModified();
    }
    else
    {
        if (deleteData())
        {
            mData.type = V_STRING;

            // Inplace new a std::string object with the source string passed to
            // the constructor.

            new (&mData.strMemData) std::string(src);

            setModified();
        }
    }
}

void Variant::assignInt(longint src)
{
    if (mData.type == V_INT)
    {
        mData.intData = src;
        setModified();
    }
    else
    {
        if (deleteData())
        {
            mData.type = V_INT;
            mData.intData = src;
            setModified();
        }
    }
}

void Variant::assignDbl(double src)
{
    if (mData.type == V_DOUBLE)
    {
        mData.dblData = src;
        setModified();
    }
    else
    {
        if (deleteData())
        {
            mData.type = V_DOUBLE;
            mData.dblData = src;
            setModified();
        }
    }
}

void Variant::assignBool(bool src)
{
    if (mData.type == V_BOOL)
    {
        mData.boolData = src;
        setModified();
    }
    else
    {
        if (deleteData())
        {
            mData.type = V_BOOL;
            mData.boolData = src;
            setModified();
        }
    }
}

std::string& Variant::s()
{
    if (mData.type != V_STRING)
    {
        // Note: This fails if the current var is vNull

        std::string s;
        makeString(s, 0, false);
        *this = s;
        setModified();
    }
    return *(mData.strData());
}

std::string Variant::toString() const
{
    if (mData.type == V_STRING)
    {
        return *(mData.strData());
    }
    else
    {
        std::string s;
        ((Variant*)this)->makeString(s, 0, false);
        return s;
    }
}

std::string Variant::toJsonString() const
{
    std::string s;
    ((Variant*)this)->makeString(s, 0, true);
    return s;
}

std::string Variant::toFixed(int digs /*= 0*/) const
{
    if (mData.type == V_DOUBLE)
    {
        //TODO: optimize
        std::string fmt;
        std::string s;
        jvar::format(fmt, "%%.%df", digs);
        jvar::format(s, fmt.c_str(), mData.dblData);
        return s;
    }
    else
    {
        return toString();
    }
}

void Variant::internalAdd(const Variant& lhs, const Variant& rhs)
{
    if (lhs.mData.type == rhs.mData.type)
    {
        // Types are same.  If they are ints, result is an int.  If they are
        // double, result is double.  If they are strings, concat them using
        // references.  Otherwise, stringify both and concat.

        if (lhs.mData.type == V_INT)
        {
            assignInt(lhs.mData.intData + rhs.mData.intData);
        }
        else if (lhs.mData.type == V_DOUBLE)
        {
            assignDbl(lhs.mData.dblData + rhs.mData.dblData);
        }
        else if (lhs.mData.type == V_STRING)
        {
            assignStr(*(lhs.mData.strData()) + *(rhs.mData.strData()));
        }
        else
        {
            assignStr(lhs.toString() + rhs.toString());
        }
    }
    else
    {
        // Types are different. If they are ints or double,
        // result is double.  Otherwise, stringify both and concat

        if ((lhs.mData.type == V_INT || lhs.mData.type == V_DOUBLE) &&
            (rhs.mData.type == V_INT || rhs.mData.type == V_DOUBLE))
        {
            assignDbl(lhs.makeDbl() + rhs.makeDbl());
        }
        else
        {
            assignStr(lhs.toString() + rhs.toString());
        }
    }
}

void Variant::internalSetPtr(const Variant* v)
{
    if (mData.type == V_POINTER)
    {
        mData.vptrData = (Variant*)v;
        setModified();
    }
    else
    {
        if (deleteData())
        {
            mData.type = V_POINTER;
            mData.vptrData = (Variant*)v;
            setModified();
        }
    }
}

void Variant::setSuppImplData(const char* name, void* supp)
{
    VarSuppImpl* impl = NULL;

    if (mData.type == V_OBJECT)
    {
        impl = (VarSuppImpl*)mData.objectData->getSupportImpl();
        if (impl == NULL)
        {
            impl = (VarSuppImpl*)mData.objectData->setSupportImpl(new VarSuppImpl());
        }
    }
    else if (mData.type == V_ARRAY)
    {
        impl = (VarSuppImpl*)mData.arrayData->getSupportImpl();
        if (impl == NULL)
        {
            impl = (VarSuppImpl*)mData.arrayData->setSupportImpl(new VarSuppImpl());
        }
    }
    if (impl != NULL)
    {
        impl->mClassName.set(name);
        impl->mSupp = supp;
    }
    else
    {
        dbgerr("Cannot set support data\n");
    }
}

void Variant::getSuppImplData(const char** name, void** supp)
{
    VarSuppImpl* impl = NULL;

    if (mData.type == V_OBJECT)
    {
        impl = (VarSuppImpl*)mData.objectData->getSupportImpl();
    }
    else if (mData.type == V_ARRAY)
    {
        impl = (VarSuppImpl*)mData.arrayData->getSupportImpl();
    }
    if (impl != NULL)
    {
        *name = impl->mClassName.get();
        *supp = impl->mSupp;
    }
    else
    {
        dbgerr("Cannot get support data\n");
    }
}


} // jvar
