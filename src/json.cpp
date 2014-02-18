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
        if (tokenEquals("["))
        {
            parseArray(outvar);
        }
        else
        {
            parseObject(outvar);
        }
    }

    advance();
    if (!eof() && !tokenEquals(""))
    {
        std::string s;
        format(s, "Extra input '%s'", token().c_str());
        setError(s.c_str());
    }

    if (failed())
    {
        dbgerr("Json parsing failed: %s\n", errMsg().c_str());
    }
}


void JsonParser::parseObject(Variant& var)
{
    // object
    //    {}
    //    { members }

    advance("{");

    var.createObject();

    parseMembers(var);
    advance("}");
}

void JsonParser::parseMembers(Variant& var)
{
    // members
    //    pair
    //    pair, members
    // pair
    //    string : value

    while (!tokenEquals("}") && !failed())
    {
        std::string key;

        // A property/key name can be a string.  Quotes can be single or double and
        // are optional in some cases.

        if (isString(token(), false))
        {
            stripQuotes(isFlagSet(mFlags, FLAG_FLEXQUOTES));
            key = token();

            advance();
        }
        advance(":");

        Variant& newprop = var.addProperty(key.c_str());

        parseValue(newprop);

        if (tokenEquals(","))
        {
            advance();
            if (tokenEquals("}"))
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

    advance("[");
    var.createArray();
    parseElements(var);
    advance("]");
}


void JsonParser::parseElements(Variant& var)
{
    // elements
    //    value
    //    value , elements

    while (!tokenEquals("]") && !failed())
    {
        // Variant v;
        // parseValue(v);
        // var.append(v);

        Variant* v = var.append(Variant::vEmpty);
        if (v)
        {
            parseValue(*v);
        }

        if (tokenEquals(","))
        {
            advance();
            if (tokenEquals("]"))
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

    if (isString(token(), true))
    {
        parseString(var);
    }
    else if (isArray(token()))
    {
        parseArray(var);
    }
    else if (isObject(token()))
    {
        parseObject(var);
    }
    else if (isNum(token()))
    {
        parseNum(var);
    }
    else if (token().compare("true") == 0)
    {
        var = true;

        advance();
    }
    else if (token().compare("false") == 0)
    {
        var = false;

        advance();
    }
    else if (token().compare("null") == 0)
    {
        var = Variant::vNull;

        advance();
    }
    else
    {
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

    std::string num;
    while (isNum(token()) || tokenEquals("+") || tokenEquals("e") ||
        tokenEquals("E") || tokenEquals("."))
    {
        num += token();

        advance();
    }

    // Determine if the number is a double
    bool isdbl = (num.find('E') != std::string::npos) || (num.find('e') != std::string::npos) ||
        (num.find('.') != std::string::npos);

    // Validate the number string (handle leading zero case)
    bool valid = true;

    if (num.length() >= 2)
    {
        if (num[0] == '0')
        {
            if (num[1] != '.')
            {
                valid = false;
            }
        }
    }

    // Convert the number string into a number
    if (valid)
    {
        if (isdbl)
        {
            double dbl = str2dbl(num.c_str(), &valid);
            var = dbl;
        }
        else
        {
            longint li = str2int(num.c_str(), &valid);
            var = li;
        }
    }

    if (!valid)
    {
        std::string err;
        format(err, "Invalid number '%s'", num.c_str());
        setError(err.c_str());
    }
}


void JsonParser::parseString(Variant& var)
{
    stripQuotes(isFlagSet(mFlags, FLAG_FLEXQUOTES));
    var = token();
    advance();
}

bool JsonParser::isString(const std::string& s, bool requirequotes)
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