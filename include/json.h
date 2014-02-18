// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _JSON_H
#define _JSON_H

#include "str.h"
#include "var.h"

namespace jvar
{

/**
 * JsonParser class parses a json string into a Variant structure.
 */
class JsonParser : public Parser
{
public:
    /**
     * Constructor
     *
     * @param  outvar  Variant which will contain the parsed data structure
     * @param  jsontxt Json text to parse
     * @param  flags   Flags
     */
    JsonParser(Variant& outvar, const char* jsontxt, uint flags = 0);

    enum
    {
        /**
         * Allow flexible quotes (single quotes or no quotes in some cases)
         */
        FLAG_FLEXQUOTES = 0x1,
        /**
         * Start with parsing an object
         */
        FLAG_OBJECTONLY = 0x2,
        /**
         * Start with parsing an array
         */
        FLAG_ARRAYONLY = 0x4
    };

protected:
    void parseObject(Variant& var);
    void parseMembers(Variant& var);
    void parseArray(Variant& var);
    void parseElements(Variant& var);
    void parseValue(Variant& var);
    void parseNum(Variant& var);
    void parseString(Variant& var);

    bool isString(const std::string& s, bool requirequotes);
    inline bool isArray(const std::string& s)
    {
        return s.compare("[") == 0;
    }
    inline bool isObject(const std::string& s)
    {
        return s.compare("{") == 0;
    }
    inline bool isNum(const std::string& s)
    {
        int l = s.length();
        if (l >= 1)
        {
            if (isdigit(s[0]) || s[0] == '-')
            {
                return true;
            }
        }
        return false;
    }

protected:
    uint mFlags;
};

} // jvar

#endif // _JSON_H
