// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "json.h"

namespace jvar
{

JsonParser::JsonParser(Variant& outvar, const char* jsontxt, uint flags /*= 0*/) :
    Parser(jsontxt),
    mFlags(flags)
{
    if (isFlagSet(mFlags, FLAG_ARRAYONLY))
    {
        parseArray(outvar);
    }
    else if (isFlagSet(mFlags, FLAG_OBJECTONLY))
    {
        parseObject(outvar);
    }
    else
    {
        if (tokenEquals('['))
        {
            parseArray(outvar);
        }
        else
        {
            parseObject(outvar);
        }
    }

    advance();
    if (!failed() && !eof() && !token().empty())
    {
        std::string s;
        format(s, "Extra input '%s'", token().c_str());
        setError(s.c_str());
    }

    if (failed())
    {
        dbglog("Json parsing failed: %s\n", errMsg().c_str());
    }
}


void JsonParser::parseObject(Variant& var)
{
    // object
    //    {}
    //    { members }

    advance('{');

    var.createObject();

    parseMembers(var);
    advance('}');
}

void JsonParser::parseMembers(Variant& var)
{
    // members
    //    pair
    //    pair, members
    // pair
    //    string : value

    StrBld key;
    while (!tokenEquals('}') && !failed())
    {
        // A property/key name can be a string.  Quotes can be single or double and
        // are optional in some cases.

        key.clear();

        if (isString(token(), false))
        {
            token().stripQuotes(isFlagSet(mFlags, FLAG_FLEXQUOTES));
            key.moveFrom(token());

            advance();
        }
        advance(':');

        // Issue #24: addOrModifyProperty() allows duplicate properties to be added
        // in the array. While JSON standard don't say what the right behavior is, jvar
        // follows the JavaScript behavior and the value is set to the very last value set.

        Variant& newprop = var.addOrModifyProperty(key.c_str());

        parseValue(newprop);

        if (tokenEquals(','))
        {
            advance();
            if (tokenEquals('}'))
            {
                setError("Found , followed by }");
            }
        }
    }
}

void JsonParser::parseArray(Variant& var)
{
    // array
    //    []
    //    [ elements ]

    advance('[');
    var.createArray();
    parseElements(var);
    advance(']');
}


void JsonParser::parseElements(Variant& var)
{
    // elements
    //    value
    //    value , elements

    while (!tokenEquals(']') && !failed())
    {
        // Variant v;
        // parseValue(v);
        // var.append(v);

        Variant* v = var.append(VEMPTY);
        if (v)
        {
            parseValue(*v);
        }

        if (tokenEquals(','))
        {
            advance();
            if (tokenEquals(']'))
            {
                setError("Found , followed by ]");
            }
        }
    }
}

void JsonParser::parseValue(Variant& var)
{
    // value
    //    string
    //    number
    //    object
    //    array
    //    true
    //    false
    //    null

    if (isNum(token()))
    {
        parseNum(var);
    }
    else if (isArray(token()))
    {
        parseArray(var);
    }
    else if (isObject(token()))
    {
        parseObject(var);
    }
    else if (tokenEquals("true"))
    {
        var = true;

        advance();
    }
    else if (tokenEquals("false"))
    {
        var = false;

        advance();
    }
    else if (tokenEquals("null"))
    {
        var = VNULL;

        advance();
    }
    else if (isString(token(), true))
    {
        parseString(var);
    }
    else
    {
        var = VNULL;
        std::string err;
        format(err, "Invalid value '%s'", token().c_str());
        setError(err.c_str());
    }
}


void JsonParser::parseNum(Variant& var)
{
    // number
    //    int
    //    int frac
    //    int exp
    //    int frac exp

    StrBld num;
    for (;;)
    {
        char c = token()[0];
        if (!isDigit(c) && c != '+' && c != '-' && c != 'e' && c != 'E' && c != '.')
        {
            break;
        }
        num.append(token());
        advance();
    }
    const char* numstr = num.c_str();

    bool isint = num.length() < 20;
    if (isint)
    {
        const char* p = numstr;
        while (*p)
        {
            if (*p == 'e' || *p == 'E' || *p == '.')
            {
                isint = false;
                break;
            }
            p++;
        }
    }

    bool valid = false;
    if (isint)
    {
        if (num.length() >= 2 && *numstr == '0')
        {
            // In json, ints are not allowed to start with zero, invalid
        }
        else
        {
            char* ench;
            longint li = strtol(numstr, &ench, 10);
            if (*ench == '\0')
            {
                valid = true;
                var = li;
            }
        }
    }
    else
    {
        char* ench;
        double dbl = strtod(numstr, &ench);
        if (*ench == '\0')
        {
            valid = true;
            var = dbl;
        }
    }

    if (!valid)
    {
        var = VNULL;
        std::string err;
        format(err, "Invalid number '%s'", numstr);
        setError(err.c_str());
    }
}


void JsonParser::parseString(Variant& var)
{
    token().stripQuotes(isFlagSet(mFlags, FLAG_FLEXQUOTES));
    var = token().toString();
    advance();
}

bool JsonParser::isString(StrBld& s, bool requirequotes)
{
    int l = s.length();
    bool flexquotes = isFlagSet(mFlags, FLAG_FLEXQUOTES);

    if (l >= 2)
    {
        // In Json, we require strings to have double quotes

        if (s[0] == '"' && s[l - 1] == '"')
        {
            return true;
        }
        else if (flexquotes)
        {
            // Allow single quotes

            if (s[0] == '\'' && s[l - 1] == '\'')
            {
                return true;
            }
        }
    }

    // If we get here that means there are no quotes (single or double) or string
    // is shorter than 2 chars.   If we are allowed to have flexible quotes and
    // quotes are not required, try to allow a "word" without quotes.

    if (flexquotes && !requirequotes)
    {
        //TODO: are there other rules for non-quoted string property names

        bool isword = true;
        for (int i = 0; i < l; i++)
        {
            if (!charWord(s[i]))
            {
                isword = false;
                break;
            }
        }
        if (isword)
        {
            return true;
        }
    }

    return false;
}

} // jvar