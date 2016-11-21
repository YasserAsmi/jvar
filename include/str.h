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
std::string formatr(const char* fmt, ...);

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
 * Trims whitespace from the left of the string
 *
 * @param str String to trim
 */
void trimLeft(std::string& str);

/**
 * Trims whitespace from the right of the string
 *
 * @param str String to trim
 */
void trimRight(std::string& str);

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
 * Compares strings to see if they are equal (not case-insensitive)
 *
 * @param  s1         String 1
 * @param  s2         String 2
 *
 * @return            True if strings are equal, false otherwise
 */
bool equal(const char* s1, const char* s2);
/**
 * Compares strings to see if they are equal (case-insensitive)
 *
 * @param  s1         String 1
 * @param  s2         String 2
 *
 * @return            True if strings are equal, false otherwise
 */
bool equalCI(const char* s1, const char* s2);

/**
 * Finds the index of a char in a string
 *
 * @param  s   String to search in
 * @param  c   Char to search
 * @param  pos If not null and match found, returns the position of the match
 *
 * @return     True if found, false otherwise
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
std::string pathParent(const std::string p, const char* delim = NULL);

/**
 * Given a string separated by path delimiters, returns the child.  Returns
 * empty string if the string is same as the delimiter.
 *
 * @param  p     Path (ex: ~/dir1/dir2/file)
 * @param  delim Delimiter (ex: /), if NULL, file system separator is used
 *
 * @return       Child path
 */
std::string pathChild(const std::string p, const char* delim = NULL);

/**
 * Returns the executable file system path for this process
 *
 * @param replfn    If not NULL, the executable name is removed and this
 *                  relative path is appended.
 *
 * @return Path
 */
std::string pathThisProc(const char* replfn = NULL);

/**
 * Calculates hash value for the string
 *
 * @param  str  Pointer to string
 * @param  len  Length of the string
 *
 * @return      Hash value
 */
uint strHashSedgewick(const char* str, size_t len);

/**
 * Replaces all instances of 'match' with 'with' for 'str'
 *
 * @param str   [description]
 * @param match [description]
 * @param with  [description]
 */
void replaceAll(std::string& str, const std::string& match, const std::string& with);


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


class StrBld
{
public:
    StrBld(const char* str) :
        mLen(0)
    {
        int l = strlen(str);
        mBuf.alloc(l + 1);
        append(str, l);
    }
    StrBld() :
        mLen(0)
    {
        mBuf.alloc(64);
    }
    StrBld(size_t minsize) :
        mLen(0)
    {
        mBuf.alloc(minsize);
    }
    StrBld(const StrBld& src) :
        mBuf(src.mBuf),
        mLen(src.mLen)
    {
    }
    StrBld(StrBld& src) :
        mBuf(src.mBuf),
        mLen(src.mLen)
    {
    }

    inline void append(char c)
    {
        char* buf = ensureAlloc(mLen + 2);
        buf[mLen++] = c;
    }
    inline void append(const char* s, int l)
    {
        char* buf = ensureAlloc(mLen + l + 1);
        memcpy(buf + mLen, s, l);
        mLen += l;
    }
    inline void append(const char* s)
    {
        append(s, strlen(s));
    }
    inline void append(const std::string& s)
    {
        append(s.c_str(), s.length());
    }
    inline void append(StrBld& sb)
    {
        append(sb.c_str(), sb.length());
    }
    inline void replaceLast(char c)
    {
        char* buf = ensureAlloc(mLen + 1);
        if (mLen > 0)
        {
            buf[mLen - 1] = c;
        }
    }
    inline void eraseLast()
    {
        if (mLen > 0)
        {
            mLen--;
        }
    }
    void clear()
    {
        mLen = 0;
    }
    const char* c_str()
    {
        char* buf = (char*)mBuf.ptr();
        if(buf)
        {
            // Lazy null termination
            buf[mLen] = '\0';
        }
        // May return buffer without zero termination,
        // but better than writing to null pointer.
        return buf;
    }
    std::string toString()
    {
        char* buf = (char*)mBuf.ptr();
        return std::string(buf, mLen);
    }
    inline uint length()
    {
        return mLen;
    }
    inline bool empty()
    {
        return mLen == 0;
    }

    inline int compare(const char* s)
    {
        const char* cp = c_str();

        if (cp)
        {
            return strcmp(cp, s);
        }
        else
        {
            // cp is null, fallback to direct comparison
            return cp != s;
        }
    }
    inline bool equals(const char* s)
    {
        return compare(s) == 0;
    }
    inline bool equals(char c)
    {
        return (mLen == 1 && *(char*)mBuf.ptr() == c);
    }

    inline char operator[](uint i)
    {
        char* buf = (char*)mBuf.ptr();
        return (i < mLen) ? buf[i] : '\0';
    }

    inline bool existCh(char c)
    {
        return (strchr(c_str(), c) != NULL);
    }

    bool appendVFmt(const char* fmt, va_list varg);
    bool appendFmt(const char* fmt, ...);

    /**
     * Strip quotes from the current token string if found
     *
     * @param allowsingle If true, also allows stripping single quotes
     */
    void stripQuotes(bool allowsingle);

    void moveFrom(StrBld& sb)
    {
        mBuf.moveFrom(sb.mBuf);
        mLen = sb.mLen;
        //sb.mBuf.alloc(64);
        sb.mLen = 0;
    }

    inline void copyTo(std::string& s)
    {
        s.assign((char*)mBuf.ptr(), mLen);
    }

private:
    inline char* ensureAlloc(size_t neededsize)
    {
        if (neededsize > mBuf.size())
        {
            mBuf.dblOr(neededsize);
        }
        return (char*)mBuf.ptr();
    }

private:
    jvar::Buffer mBuf;
    uint mLen;
};


/**
 * Parser class implements a text parser which follows simple rules to build tokens.
 *    .Sequence of letters, digits, and '_' is a token
 *    .Opening or a closing bracket is a token
 *    .Sequence of punctuation symbols like * - ; = , etc is a token (can force single punctuation per token)
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

    // TODO: Rename to tokenStr
    inline const char* c_str()
    {
        parseToken();
        return mToken.c_str();
    }

    inline StrBld& token()
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
        parseToken();
        return mToken.equals(val);
    }
    inline bool tokenEquals(char c)
    {
        parseToken();
        return mToken.equals(c);
    }

    /**
     * Advances to the next token by parsing it
     */
    inline void advance()
    {
        mTokParsed = false;
    }
    inline void advance(const char* match)
    {
        if (mErr)
        {
            return;
        }
        if (tokenEquals(match))
        {
            advance();
        }
        else
        {
            expectErr(0, match);
        }
    }

    inline void advance(char matchc)
    {
        if (mErr)
        {
            return;
        }

        if (tokenEquals(matchc))
        {
            advance();
        }
        else
        {
            expectErr(matchc, NULL);
        }
    }

    /**
     * Sets an error message and signal failure
     *
     * @param msg Error text
     */
    void setError(const char* msg);

	/** Enables or disables single punctuation mode.  In single punc mode, a token
      * consists of only a single char.  When this mode is disabled (default), multiple
      * adjacent chars are combined into a single token.
      */
    inline void setSinglePunc(bool enable)
    {
        mSinglePunc = enable;
    }

    /**
     * Capture everything up to \p delim into the current token.
     */
    void captureDelim(const char* delim);

    inline void ensureTok()
    {
        parseToken();
    }
    std::string tokFullStr()
    {
        std::string s;
        int l = mTokEndPos - mTokStartPos;
        if (mTokStartPos + l <= mTxtLen)
        {
            s = std::string(mTxt + mTokStartPos, l);
        }
        return s;
    }
    inline int tokEndPos()
    {
        return mTokEndPos;
    }
    inline int tokStartPos()
    {
        return mTokStartPos;
    }
    inline bool tokIsWord()
    {
        return tokType() == WordTok;
    }
    inline bool tokIsPunc()
    {
        return tokType() == PuncTok;
    }
    inline bool tokIsQuot()
    {
        return tokType() == QuoteTok;
    }
    inline bool tokIsBrac()
    {
        return tokType() == BracTok;
    }
    inline bool tokIs(const char* val)
    {
        return tokenEquals(val);
    }

private:
    enum TokenStateEnum
    {
        NullTok, WordTok, PuncTok, BracTok, QuoteTok
    };

    const char* mTxt;
    StrBld mToken;
    bool mTokParsed;
    int mPos;
    int mLineNum;
    bool mErr;
    int mTxtLen;
    std::string mErrMsg;
    int mTokStartPos;
    int mTokEndPos;
    bool mSinglePunc;

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
    inline void parseToken()
    {
        if (mErr)
        {
            mToken.clear();
            return;
        }
        if (mTokParsed)
        {
            return;
        }
        internalParse();
    }

    void internalParse();
    void expectErr(char c, const char* str);

    inline void append(char c)
    {
        mToken.append(c);
    }
    inline void append(const std::string& s)
    {
        mToken.append(s);
    }
    inline void replaceLast(char c)
    {
        mToken.replaceLast(c);
    }
    inline void eraseLast()
    {
        mToken.eraseLast();
    }
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

        return PuncTok;
    }


    TokenStateEnum tokType();
};


class Replacer
{
public:
    Replacer() :
        mOrgStr(NULL),
        mOrgMax(0),
        mOrgPos(0),
        mBufPos(0)
    {
    }
    Replacer(const char* str) :
        mOrgStr(NULL),
        mOrgMax(0),
        mOrgPos(0),
        mBufPos(0)
    {
        setSrc(str);
    }

    void setSrc(const char* str);

    inline void setSrc(const std::string& str)
        {
        setSrc(str.c_str());
    }
    bool replace(int orgpos, int orglen, const char* with);

    inline bool replace(int orgpos, int orglen, const std::string& with)
    {
        return replace(orgpos, orglen, with.c_str());
    }

    const char* c_str();
    std::string str();

private:
    void copy(const char* from, int len, int orglen);
    void ensureAlloc(size_t neededsize);

private:
    const char* mOrgStr;
    int mOrgMax;
    int mOrgPos;
    jvar::Buffer mBuf;
    int mBufPos;
};


class Splitter : public jvar::Parser
{
public:
    Splitter(const char* str, const char* delim) :
        Parser(str),
        mDelim(delim)
    {
        // Note 1: Splitter doesn't work if delim is whitespace--whitespace is ignored
        //         unless inside of quotes.
        // Note 2: delim is ignored if inside quotes.

        setSinglePunc(true);
    }

    std::string get();

private:
    std::string mDelim;
};



} // jvar


#endif // _STR_H

