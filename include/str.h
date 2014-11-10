/**
 * @file include/str.h
 * Declares methods and classes for use with strings.
 * @copyright Copyright (c) 2014 Yasser Asmi; Released under the MIT
 *            License (http://opensource.org/licenses/MIT)
 */

#ifndef _STR_H
#define _STR_H

#include "util.h"

/**
 * Punctuation chars for the parser
 */
#define PUNC_CHARS   "&!|/:;=+*-.$@^%?`,\\"

/**
 * Bracket chars for the parser
 */
#define BRAC_CHARS "<([{}])>"

/**
 * Escape codes, ESCAPE_CODES must match ESCAPE_CHARS) exactly
 */
#define ESCAPE_CODES  "nrtbf\\\"/"

/**
 * Escape chars, ESCAPE_CODES must match ESCAPE_CHARS) exactly
 */
#define ESCAPE_CHARS  "\n\r\t\b\f\\\"/"

namespace jvar
{

/**
 * Formats a string using printf style parameters in varg form
 *
 * @param  fmt    printf fmt string
 *
 * @return        Formatted string
 */
std::string format(const char* fmt, ...);

/**
 * Formats a string using printf style parameters
 *
 * @param  outstr string
 * @param  fmt    printf fmt string
 */
void format(std::string& outstr, const char* fmt, ...);

/**
 * Formats a string using printf style parameters in varg form
 *
 * @param  outstr string
 * @param  fmt    printf fmt string
 * @param  varg   variable argument
 *
 * @return        Success
 */
bool vformat(std::string& outstr, const char* fmt, va_list varg);

/**
 * Converts characters in a string into uppercase characters
 *
 * @param str String to convert
 */
void upperCase(std::string& str);

/**
 * Converts characters in a string into lowercase characters
 *
 * @param str String to convert
 */
void lowerCase(std::string& str);

/**
 * Reads characters from utf8 string and builds a unicode character
 *
 * @param  s       String containing utf8 chars
 * @param  maxlen  Maximum number of chars remaining in the string
 * @param  lenused NULL or a pointer to recieve how many chars were consumed
 *
 * @return         Unicode character
 */
uint makeUnicode(const char* s, int maxlen, int* lenused = NULL);

/**
 * Converts a unicode character into a string of utf8 characters
 *
 * @param  charcode Unicode char
 *
 * @return          String conaining utf8 equivelent
 */
std::string makeUTF8(uint charcode);

/**
 * Compares strings to see if they are equal
 *
 * @param  s1         String 1
 * @param  s2         String 2
 * @param  ignorecase True ignores case, otherwise case is considered
 *
 * @return            True if strings are equal, false otherwise
 */
bool equal(const char* s1, const char* s2, bool ignorecase = false);

/**
 * Find \p c in \p s.
 */
inline bool strfind(const char* s, char c, int* pos = NULL)
{
    const char* foundp = strchr(s, c);
    if (foundp)
    {
        if (pos)
        {
            *pos = (foundp - s);
        }
        return true;
    }
    return false;
}

/**
 * Convert an integer into a string
 *
 * @param  n Integer to convert
 *
 * @return   String representation of the integer
 */
std::string int2str(longint n);

/**
 * Converts an integer into a string
 *
 * @param s Reference to a string to recieve the output
 * @param n Integer to convert
 */
inline void int2str(std::string& s, longint n)
{
    format(s, "%ld", n);
}

/**
 * Convert an double into a string
 *
 * @param  n double to convert
 *
 * @return   String representation of the double
 */
std::string dbl2str(double d);

/**
 * Converts an double into a string
 *
 * @param s Reference to a string to recieve the output
 * @param n Double to convert
 */
inline void dbl2str(std::string& s, double d)
{
    format(s, "%lf", d);
}

/**
 * Converts a string into an integer
 *
 * @param  str   Number string to convert
 * @param  base  Base (ex: 10)
 * @param  valid Optional NULL or a pointer to recieve conversion success
 *
 * @return  Converted integer
 */
longint str2baseint(const std::string& str, int base, bool* valid = NULL);

/**
 * Converts a string into an base 10 integer
 *
 * @param  str   Number string to convert
 * @param  valid Optional NULL or a pointer to recieve conversion success
 *
 * @return  Converted integer
 */
inline longint str2int(const std::string& str, bool* valid = NULL)
{
    return str2baseint(str, 10, valid);
}

/**
 * Converts a string into a double
 *
 * @param  str   Number string to convert
 * @param  valid Optional NULL or a pointer to recieve the conversion success
 *
 * @return       Converted double
 */
double str2dbl(const std::string& str, bool* valid  = NULL);

/**
 * Given a string separated by path delimeters, returns the parent.   Returns
 * empty string if the string is same as the delimiter.
 *
 * @param  p     Path (ex: ~/dir1/dir2/file)
 * @param  delim Delimter (ex: /)
 *
 * @return       Parent path
 */
inline std::string pathParent(const std::string p, const char* delim)
{
    return p.substr(0, p.find_last_of(delim));
}

/**
 * Given a string separated by path delimeters, returns the child.   Returns
 * empty string if the string is same as the delimiter.
 *
 * @param  p     Path (ex: ~/dir1/dir2/file)
 * @param  delim Delimter (ex: /)
 *
 * @return       Child path
 */
inline std::string pathChild(const std::string p, const char* delim)
{
    return p.substr(p.find_last_of(delim) + 1);
}

/**
 * FixedStr class template is used to declare a string object which has enough
 * space to store a string of MAXFIXED size.  If the string being assigned is
 * longer, heap memory is allocated.  Otherwise no memory is allocated.
 */
template <int MAXFIXED>
class FixedStr
{
public:
    /**
     * Construct an empty FixedStr.
     */
    inline FixedStr()
    {
        mFixed[0] = '\0';
    }

    /**
     * Construct a FixedStr from another FixedStr.
     */
    inline FixedStr(const FixedStr& src)
    {
        mFixed[0] = '\0';
        set(src.get());
    }

    /**
     * Construct a FixedStr from another FixedStr.
     */
    inline FixedStr(FixedStr& src)
    {
        mFixed[0] = '\0';
        set(src.get());
    }
    ~FixedStr()
    {
        if (mFixed[0] == '\1')
        {
            if (mDyn.ptr)
            {
                free(mDyn.ptr);
            }
        }
    }

    /**
     * Copy the data in \p src into this FixedStr.
     */
    inline FixedStr& operator=(const FixedStr& src)
    {
        set(src.get());
        return *this;
    }

    /**
     * Set the string to the contents of \p val.
     */
    void set(const char* val)
    {
        int len = strlen(val);
        char* dest;

        if (mFixed[0] == '\2')
        {
            mFixed[0] = '\0';
        }

        if (len > (MAXFIXED - 2))
        {
            // Cannot fit in fixed

            if (isFixed())
            {
                mFixed[0] = '\1';

                mDyn.ptr = NULL;
                mDyn.size = 0;
            }
            ensureDyn(len + 1);
            dest = mDyn.ptr;
        }
        else
        {
            // Can fit in fixed

            if (isFixed())
            {
                dest = &(mFixed[1]);
            }
            else
            {
                ensureDyn(len + 1);
                dest = mDyn.ptr;
            }
        }

        memcpy(dest, val, len + 1);
    }

    /**
     * Set the internal string pointer.
     * Stores an external pointer and doesn't try to free it.
     * Can only be called once.  Can call set() after this, however.
     */
    void setExt(const char* val)
    {
        // Stores an external pointer and doesn't try to free it.
        // Can only be called once.  Can call set() after this, however.

        mFixed[0] = '\2';

        mDyn.ptr = (char*)val;
        mDyn.size = 0;
    }

    /**
     * Get the internal string pointer.
     */
    const char* get() const
    {
        if (isFixed())
        {
            return &(mFixed[1]);
        }
        else
        {
            return mDyn.ptr;
        }
    }

    /**
     * Empty this string.
     */
    void clear()
    {
        if (mFixed[0] == '\1')
        {
            if (mDyn.ptr)
            {
                free(mDyn.ptr);
            }
        }
        mFixed[0] = '\0';
    }

private:
    union
    {
        struct
        {
            int tag;
            char* ptr;
            int size;
        } mDyn;
        char mFixed[MAXFIXED];
    };

    inline bool isFixed() const
    {
        return mFixed[0] == '\0';
    }

    void ensureDyn(int size)
    {
        if (mDyn.ptr != NULL && mDyn.size >= size)
        {
            return;
        }

        void* p = ::realloc(mDyn.ptr, size);
        if (p == NULL)
        {
            dbgerr("FixedStr failed to allocate %d bytes\n", size);
            return;
        }
        mDyn.ptr = (char*)p;
        mDyn.size = size;
   }
};

/**
 * Parser class implements a text parser which follows simple rules to build tokens.
 *    .Sequence of letters, digits, and '_' is a token
 *    .Opening or a closing bracket is a token
 *    .Sequence of punctuation symbols like * - ; = , etc is a token
 *    .Sequence of characters beginning and ending with a quote is a token (escape chars are consistent with JSON)
 *    .Whitespace is ignored except in quotes
 */
class Parser
{
public:
    /**
     * Constructor
     *
     * @param  txt Text to parse
     */
    Parser(const char* txt);

    /**
     * Is the entire text parsed
     *
     * @return True if at the end of text has reached
     */
    inline bool eof()
    {
        return (mPos >= mTxtLen) || mErr;
    }

    /**
     * Was there a failure
     *
     * @return True if there was an error, false otherwise
     */
    inline bool failed()
    {
        return mErr;
    }

    /**
     * Returns the error message
     *
     * @return Reference to the formatted error message string
     */
    inline const std::string& errMsg()
    {
        return mErrMsg;
    }

    /**
     * Returns the current token
     *
     * @return Pointer to current token
     */
    inline const char* c_str()
    {
        return token().c_str();
    }

    /**
     * Returns the current token
     *
     * @return Reference to current token
     */
    inline const std::string& token()
    {
        parseToken();
        return mToken;
    }

    /**
     * Compares the current token with a string
     *
     * @param  val String to compare
     *
     * @return     True if equal, false otherwise
     */
    inline bool tokenEquals(const char* val)
    {
        return token().compare(val) == 0;
    }

    /**
     * Advances to the next token by parsing it
     */
    inline void advance()
    {
        mTokParsed = false;
    }

    /**
     * Advances if a matching token is found
     *
     * @param match String to match
     */
    void advance(const char* match);

    /**
     * Sets an error message and signal failure
     *
     * @param msg Error text
     */
    void setError(const char* msg);

    /**
     * Strip quotes from the current token string if found
     *
     * @param allowsingle If true, also allows stripping single quotes
     */
    void stripQuotes(bool allowsingle);

    /**
     * Capture everything up to \p delim into the current token.
     */
    void captureDelim(const char* delim);

private:
    enum TokenStateEnum
    {
        NullTok, WordTok, PuncTok, BracTok, QuoteTok
    };

    const char* mTxt;
    std::string mToken;
    bool mTokParsed;
    int mPos;
    int mLineNum;
    bool mErr;
    int mTxtLen;
    std::string mErrMsg;

protected:
    /**
     * Check if \p c is a valid word character (alphanumeric, '_', or ''').
     */
    inline bool charWord(char c)
    {
        return (isalnum(c) || (c == '_') || (c == '\''));
    }

    /**
     * Check if \p c is a punctuation character (i.e. in \ref PUNC_CHARS).
     */
    inline bool charPunc(char c)
    {
        return (strchr(PUNC_CHARS, c) != NULL);
    }

    /**
     * Check if \p c is a bracket character (i.e. in \ref BRAC_CHARS).
     */
    inline bool charBrac(char c)
    {
        return (strchr(BRAC_CHARS, c) != NULL);
    }

private:
    /**
     * Parses the current token.
     */
    void parseToken();

    /**
     * Append \p c onto the token.
     */
    inline void append(char c)
    {
        mToken += c;
    }

    /**
     * Append \p s onto the token.
     */
    inline void append(const std::string& s)
    {
        mToken += s;
    }

    /**
     * Replace the last character of the current token with \p c.
     */
    inline void replaceLast(char c)
    {
        int l = mToken.length();
        if (l > 0)
        {
            mToken[l - 1] = c;
        }
    }

    /**
     * Remove the last character of the current token.
     */
    inline void eraseLast()
    {
        int l = mToken.length();
        if (l >= 1)
        {
            mToken.erase(l - 1, 1);
        }
    }

    /**
     * Detect the current state & return the "right" token type.
     */
    inline TokenStateEnum detState(char c)
    {
        if (c == '"' || c == '\'')
        {
            return QuoteTok;
        }
        if (charWord(c))
        {
            return WordTok;
        }
        if (charBrac(c))
        {
            return BracTok;
        }
        //TODO: Should this call charPunc?

        return PuncTok;
    }

};

} // jvar


#endif // _STR_H

