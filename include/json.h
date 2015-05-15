/**
 * @file include/json.h
 * Declares the JsonParser class.
 * @copyright Copyright (c) 2014 Yasser Asmi; Released under the MIT
 *            License (http://opensource.org/licenses/MIT)
 */

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

    /**
     * Flags that can be set in \ref mFlags.
     */
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
    /**
     * Parse an object into \p var.
     */
    void parseObject(Variant& var);

    /**
     * Parse the members of an object into \p var.
     */
    void parseMembers(Variant& var);

    /**
     * Parse an array into \p var.
     */
    void parseArray(Variant& var);

    /**
     * Parse the elements of an array into \p var.
     */
    void parseElements(Variant& var);

    /**
     * Parse a JSON value into \p var.
     */
    void parseValue(Variant& var);

    /**
     * Parse a number into \p var.
     */
    void parseNum(Variant& var);

    /**
     * Parse a string into \p var.
     */
    void parseString(Variant& var);

    /**
     * Check if \p s is convertable to a string.
     */
    bool isString(StrBld& s, bool requirequotes);

    /**
     * *Quickly* check if \p s is convertable to an array. Don't rely on this method!
     */
    inline bool isArray(StrBld& s)
    {
        return s.equals('[');
    }
    inline bool isObject(StrBld& s)
    {
        return s.equals('{');
    }

    /**
     * Check if \p s is convertable to a number (specifically an integer).
     */
    inline bool isNum(StrBld& s)
    {
        char c = s[0];
        if (isDigit(c) || c == '-')
        {
            return true;
        }
        return false;
    }
    inline bool isDigit(char c)
    {
        return (c >='0' && c <= '9');
    }

protected:
    /**
     * Flags that are set on this object.
     */
    uint mFlags;
};

} // jvar

#endif // _JSON_H
