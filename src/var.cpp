// Copyright (c) 2014-2015 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "var.h"
#include "json.h"

#if __cplusplus > 199711L
#include <initializer_list>
#endif

namespace jvar
{

// Static variables

Variant Variant::sEmpty;
Variant Variant::sNull(Variant::V_NULL);
RcLife<BaseInterface> Variant::sNullExtIntf;

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
    bool err = json.failed();
    if (err)
    {
        clear();
    }
    setModified();
    return !err;
}

bool Variant::eq(const char* s)
{
    return equal(toString().c_str(), s);
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


void Variant::makeString(StrBld& s, int level, bool json)
{
    switch (mData.type)
    {
        case V_STRING:
        {
            s.clear();
            s.append( *(mData.strData()) );

            if (json)
            {
                jsonifyStr(s);
            }
        }
        break;

        case V_INT:
        {
            s.clear();
            s.appendFmt("%ld", mData.intData);
        }
        break;

        case V_DOUBLE:
        {
            //TODO: Verify it is ok not to do the following
            // if (tmp.find_first_of('.') == std::string::npos)
            // {
            //     tmp += ".0";
            // }
            s.clear();
            s.appendFmt("%lg", mData.dblData);
        }
        break;

        case V_BOOL:
        {
            s.clear();
            s.append(mData.boolData ? "true" : "false");
        }
        break;

        case V_EMPTY:
        {
            //TODO: Revisit this value--should work for json
            s.clear();
            s.append("null");
        }
        break;

        case V_NULL:
        {
            s.clear();
            s.append("null");
        }
        break;

        case V_ARRAY:
        {
            s.append('[');
            level++;
            for (Iter<Variant> i; forEach(i); )
            {
                if (i.pos() != 0)
                {
                    s.append(',');
                }

                appendNewline(s, level, json);

                appendQuote(s, i->type());

                StrBld tmps;
                i->makeString(tmps, level, json);

                s.append(tmps);
                appendQuote(s, i->type());

            }
            level--;
            appendNewline(s, level, json);
            s.append(']');
        }
        break;

        case V_OBJECT:
        {
            s.append('{');

            level++;
            for (Iter<Variant> i; mData.objectData->forEach(i); )
            {

                if (i.pos() != 0)
                {
                    s.append(",");
                }

                appendNewline(s, level, json);

                appendQuote(s, V_STRING);

                if (json)
                {
                    StrBld keys(i.key());
                    jsonifyStr(keys);
                    s.append(keys);
                }
                else
                {
                    s.append(i.key());
                }
                appendQuote(s, V_STRING);

                s.append(':');

                appendQuote(s, i->type());

                StrBld tmps;
                i->makeString(tmps, level, json);

                s.append(tmps);
                appendQuote(s, i->type());

            }
            level--;
            appendNewline(s, level, json);
            s.append('}');
        }
        break;

        case V_FUNCTION:
        {
            s.clear();
            s.append("(function)");
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


void Variant::jsonifyStr(StrBld& s)
{
    StrBld ts(s.length());

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

            ts.appendFmt("\\u%04X", c);

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
                ts.append('\\');
                ts.append(c);
            }
            else
            {
                ts.append((char)c);
            }
        }
    }

    // Return the string

    s.moveFrom(ts);
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
    else
    {
        VarExtInterface* intf = cast<VarExtInterface>(extInterface().ptr());
        if (intf != NULL)
        {
            intf->onAppend(*this, *newelem);
        }
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


int Variant::indexOf(const char* str)
{
    if (mData.type == V_STRING)
    {
        size_t pos = s().find(str);
        return (pos != std::string::npos ? (int)pos : -1);
    }
    else if (mData.type == V_ARRAY)
    {
        for (Iter<Variant> i; forEach(i); )
        {
            if (i->eq(str))
            {
                return i.pos();
            }
        }
    }
    return -1;
}

int Variant::lastIndexOf(const char* str)
{
   if (mData.type != V_STRING)
   {
       return -1;
   }
   size_t pos = s().rfind(str);
   return (pos != std::string::npos ? (int)pos : -1);
}

void Variant::split(const char* str, const char* sep)
{
    // Splitter doesn't work if sep is just whitespace--whitespace is ignored
    Splitter splt(str, sep);

    createArray();
    while (!splt.eof())
    {
        append(splt.get());
    }
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

#ifdef AUTOADDPROP
        mData.flags |= Variant::VF_AUTOADDPROP;
#endif

    }
    else
    {
        dbgerr("createObject() failed\n");
    }
}


Variant& Variant::addProperty(const char* key, const Variant& value /* = VEMPTY */)
{
    assert(key);

    if (mData.type != V_OBJECT)
    {
        dbglog("addProperty(%s) failed -- not an object\n", key);
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


Variant& Variant::addOrModifyProperty(const char* key)
{
    assert(key);

    if (mData.type != V_OBJECT)
    {
        dbglog("addOrModifyProperty(%s) failed -- not an object\n", key);
        return VNULL;
    }

    Variant* newprop = mData.objectData->addOrModify(key);
    if (!newprop)
    {
        dbglog("addOrModifyProperty(%s) failed\n", key);
        return VNULL;
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

Variant* Variant::handleMissingKey(const char* key)
{
    Variant* v = NULL;

    // Call the ext interface if found to see if it has the required value.

    VarExtInterface* intf = cast<VarExtInterface>(extInterface().ptr());
    if (intf != NULL)
    {
        v = intf->onAddMissingKey(*this, key);
    }

    // If still missing key and we are allowed to add automatically, add the key

    if (v == NULL && isFlagSet(mData.flags, Variant::VF_AUTOADDPROP))
    {
        v = &(addProperty(key));
    }

    // If still missing key and error reporting is not disabled, log the error

    if (v == NULL && isFlagClear(mData.flags, VF_NOMISSINGKEYERR))
    {
        dbglog("[%s] not found\n", key);
    }

    // Return the pointer to the added or null

    return v;
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
            v = handleMissingKey(key);
            if (v)
            {
                return *v;
            }
        }
    }
    else if (mData.type == V_FUNCTION)
    {
        return mData.funcData->mEnv[key];
    }
    else if (mData.type != V_NULL && mData.type != V_EMPTY)
    {
        dbglog("[%s] failed--not an object or func\n", key);
    }
    return VNULL;
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
            v = const_cast<Variant*>(this)->handleMissingKey(key);
            if (v)
            {
                return *v;
            }
        }
    }
    else if (mData.type == V_FUNCTION)
    {
        return mData.funcData->mEnv[key];
    }
    else if (mData.type != V_NULL && mData.type != V_EMPTY)
    {
        dbglog("[%s] failed--not an object or func\n", key);
    }
    return VNULL;
}

Variant& Variant::path(const char* pathkey)
{
    assert(pathkey);

    //dbglog("path: %p ['%s']\n", this, pathkey);
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
                    int n = str2int(p.token().toString(), &valid);
                    if (valid)
                    {
                        v = &((*v)[n]);
                    }
                    else
                    {
                        v = &sNull;
                    }
                }
                else
                {
                    v = &((*v)[p.token().c_str()]);
                }
                //dbglog("  %s = %p\n", p.token().c_str(), v);
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
    else if (mData.type != V_NULL && mData.type != V_EMPTY)
    {
        dbgerr("[%d] failed--not an object or array\n", i);
    }
    return VNULL;
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
    else if (mData.type != V_NULL && mData.type != V_EMPTY)
    {
        dbgerr("[%d] failed--not an object or array\n", i);
    }
    return VNULL;
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

/*
 * Code requires C++11 to work - see below for backwards-compatible code.
 */
#if __cplusplus > 199711L

Variant Variant::operator() (std::initializer_list<const jvar::Variant>&& values)
{
    if (mData.type != V_FUNCTION) return VNULL;

    Variant arg;

    arg.createArray();

    for (const auto& value : values)
    {
        arg.append(value);
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

/*
 * Pre-C++11 code (does not require init lists)
 */
#else

Variant Variant::operator() ()
{
    if (mData.type == V_FUNCTION)
    {
        Variant arg;

        arg.createArray();

        return mData.funcData->mFunc(mData.funcData->mEnv, arg);
    }
    return VNULL;
}

Variant Variant::operator() (const Variant& value1)
{
    if (mData.type == V_FUNCTION)
    {
        Variant arg;
        Variant* p;

        arg.createArray();

        p = arg.append();
        p->internalSetPtr(&value1);

        return mData.funcData->mFunc(mData.funcData->mEnv, arg);
    }
    return VNULL;
}

Variant Variant::operator() (const Variant& value1, const Variant& value2)
{
    if (mData.type == V_FUNCTION)
    {
        Variant arg;
        Variant* p;

        arg.createArray();

        p = arg.append();
        p->internalSetPtr(&value1);
        p = arg.append();
        p->internalSetPtr(&value2);

        return mData.funcData->mFunc(mData.funcData->mEnv, arg);
    }
    return VNULL;
}

Variant Variant::operator() (const Variant& value1, const Variant& value2, const Variant& value3)
{
    if (mData.type == V_FUNCTION)
    {
        Variant arg;
        Variant* p;

        arg.createArray();

        p = arg.append();
        p->internalSetPtr(&value1);
        p = arg.append();
        p->internalSetPtr(&value2);
        p = arg.append();
        p->internalSetPtr(&value3);

        return mData.funcData->mFunc(mData.funcData->mEnv, arg);
    }
    return VNULL;
}


Variant Variant::operator() (const Variant& value1, const Variant& value2, const Variant& value3,
    const Variant& value4)
{
    if (mData.type == V_FUNCTION)
    {
        Variant arg;
        Variant* p;

        arg.createArray();

        p = arg.append();
        p->internalSetPtr(&value1);
        p = arg.append();
        p->internalSetPtr(&value2);
        p = arg.append();
        p->internalSetPtr(&value3);
        p = arg.append();
        p->internalSetPtr(&value4);

        return mData.funcData->mFunc(mData.funcData->mEnv, arg);
    }
    return VNULL;
}
#endif

bool Variant::addEnv(const char* varname, const Variant& value /* = VEMPTY */)
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
            using namespace std;
	    strdata->std::string::~string();
            //strdata->std::string::~basic_string();
        }
        break;

        case V_ARRAY:
        {
            delete mData.arrayData;
            mData.arrayData = NULL;
        }
        break;

        case V_OBJECT:
        {
            delete mData.objectData;
            mData.objectData = NULL;
        }
        break;

        case V_FUNCTION:
        {
            delete mData.funcData;
            mData.funcData = NULL;
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
                // Create the array object using the copy constructor.

                mData.arrayData = new ObjArray<Variant>(*(src->mData.arrayData));
            }
            break;

            case V_OBJECT:
            {
                // Create the proparray object using the copy constructor.

                mData.objectData = new PropArray<Variant>(*(src->mData.objectData));
            }
            break;

            case V_FUNCTION:
            {
                // Create the function object using the copy constructor.

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
                // If a NULL or EMPTY is being copied, we set the data type of src to EMPTY.
                // Setting it to NULL is not good because further assignments will fail.

                mData.type = V_EMPTY;
            }
            break;

            default:
            {
                dbgerr("TODO: copyfrom missing %d\n", src->mData.type);

                //assert(false);
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

#if __cplusplus > 199711L

void Variant::assignObj(std::initializer_list<const Variant>& src)
{
    bool is_object = true;

    if (src.size() == 0)
    {
        createObject();
        return;
    }

    for(const auto& v : src)
    {
        if (!v.isArray() || v.length() != 2 || !v[0].isString())
            is_object = false;
    }

    if (is_object)
    {
        createObject();
        for(const auto& v : src)
        {
            addProperty(v[0].toString(), v[1]);
        }
    }
    else
    {
        createArray();
        for (const auto& v: src)
        {
            push(v);
        }
    }
}

#endif

std::string& Variant::s()
{
    if (mData.type != V_STRING)
    {
        // Note: This fails if the current var is VNULL

        std::string s;
        StrBld sb;
        makeString(sb, 0, false);
        *this = sb.toString();
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
        StrBld sb;
        ((Variant*)this)->makeString(sb, 0, false);
        return sb.toString();
    }
}


std::string Variant::toStrE() const
{
    if (mData.type == V_STRING)
    {
        return *(mData.strData());
    }
    else if (mData.type != V_NULL && mData.type != V_EMPTY)
    {
        StrBld sb;
        ((Variant*)this)->makeString(sb, 0, false);
        return sb.toString();
    }
    return std::string();
}


std::string Variant::toJsonString() const
{
    StrBld sb;
    ((Variant*)this)->makeString(sb, 0, true);
    return sb.toString();
}

void Variant::makeJson(StrBld& sb) const
{
    sb.clear();
    ((Variant*)this)->makeString(sb, 0, true);
}


std::string Variant::toFixed(int digs /*= 0*/) const
{
    if (mData.type == V_DOUBLE)
    {
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
        // result is double.  If the lhs type is NULL or empty, use lhs type.
        // Otherwise, stringify both and concat

        if ((lhs.mData.type == V_INT || lhs.mData.type == V_DOUBLE) &&
            (rhs.mData.type == V_INT || rhs.mData.type == V_DOUBLE))
        {
            assignDbl(lhs.makeDbl() + rhs.makeDbl());
        }
        else if (lhs.empty() && (rhs.mData.type == V_INT || rhs.mData.type == V_DOUBLE))
        {
            if (rhs.mData.type == V_INT)
            {
                assignInt(lhs.makeInt() + rhs.mData.intData);
            }
            else
            {
                assignDbl(lhs.makeDbl() + rhs.mData.dblData);
            }
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


bool Variant::readJsonFile(const char* filename)
{
    Buffer jsontxt;
    if (!jsontxt.readFile(filename, true))
    {
        dbgerr("Failed to json read file: %s\n", filename);
        return false;
    }

    JsonParser json(*this, (const char*)jsontxt.cptr());
    if (json.failed())
    {
        dbgerr("Failed to parse json file\n");
        clear();
        return false;
    }
    return true;
}

RcLife<BaseInterface>& Variant::extInterface()
{
    if (mData.type == V_OBJECT)
    {
        return mData.objectData->extInterface();
    }
    else if (mData.type == V_ARRAY)
    {
        return mData.arrayData->extInterface();
    }
    dbgerr("Failed to get interface type=%s\n", typeName());
    return sNullExtIntf;
}


void Variant::newFrom(Variant param)
{
    VarExtInterface* ext = cast<VarExtInterface>(param.extInterface().ptr());
    if (ext)
    {
        ext->onNewExt(*this, param);
    }
    else
    {
        dbgerr("Null interface\n");
    }
}


void Variant::save()
{
    VarExtInterface* ext = cast<VarExtInterface>(extInterface().ptr());

    if (ext)
    {
        ext->onSaveExt(*this);
    }
    else
    {
        dbgerr("Null interface\n");
    }
}


void Variant::load(Variant param)
{
    VarExtInterface* ext = cast<VarExtInterface>(param.extInterface().ptr());
    if (ext)
    {
        ext->onLoadExt(*this, param);
    }
    else
    {
        dbgerr("Null interface\n");
    }
}

// VarExtInterface::

void VarExtInterface::onAppend(Variant& arr, Variant& newelem)
{
}

bool VarExtInterface::onNewExt(Variant& destobj, Variant& param)
{
    return false;
}

bool VarExtInterface::onSaveExt(Variant& sobj)
{
    return false;
}

bool VarExtInterface::onLoadExt(Variant& destobj, Variant& param)
{
    return false;
}

Variant* VarExtInterface::onAddMissingKey(Variant& destobj, const char* key)
{
    return NULL;
}


} // jvar
