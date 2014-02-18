// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "str.h"

using namespace std;

namespace jvar
{

void upperCase(std::string& str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        str[i] = toupper(str[i]);
    }
}

void lowerCase(std::string& str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        str[i] = tolower(str[i]);
    }
}

bool equal(const char* s1, const char* s2, bool ignorecase)
{
    if (s1 && s2)
    {
        if (ignorecase)
        {
            return (strcasecmp(s1, s2) == 0);
        }
        else
        {
            return (strcmp(s1, s2) == 0);
        }
    }
    else
    {
        return (s1 == s2);
    }
}

bool vformat(string& outstr, const char* fmt, va_list varg)
{
    const int stackbufsize = 256;
    char stackbuf[stackbufsize];
    int count;
    va_list orgvarg;

    va_copy(orgvarg, varg);

    // Try to format using the small stack buffer.

    count = vsnprintf(stackbuf, stackbufsize, fmt, varg);
    if (count < 0)
    {
        // Negative number means formatting error.

        return false;
    }

    if (count < stackbufsize)
    {
        // Non-negative and less than the given size so string was completely written.

        outstr.assign(stackbuf);
    }
    else
    {
        // Format the string with a suffiently large heap buffer.

        void* buf = malloc(count + 1);
        if (buf == NULL)
        {
            return false;
        }

        vsnprintf((char*)buf, count + 1, fmt, orgvarg);

        outstr.assign((const char*)buf);
        free(buf);
    }
    return true;
}

void format(string& outstr, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    bool ret = vformat(outstr, fmt, va);
    va_end(va);

    if (!ret)
    {
        dbgerr("String format failed\n");
    }
}

string format(const char* fmt, ...)
{
    string outstr;

    va_list va;
    va_start(va, fmt);
    bool ret = vformat(outstr, fmt, va);
    va_end(va);

    if (!ret)
    {
        dbgerr("String format failed\n");
    }
    return outstr;
}


string int2str(longint n)
{
    //TODO: call format() that returns a string
    string s;
    format(s, "%ld", n);
    return s;
}

string dbl2str(double d)
{
    string s;
    format(s, "%lf", d);
    return s;
}

longint str2baseint(const string& str, int base, bool* valid /* = NULL */)
{
    char* end;
    bool res = true;

    errno = 0;
    int value = strtol(str.c_str(), &end, base);
    if ((errno != 0 && value == 0) || (*end != '\0'))
    {
        value = 0;
        res = false;
    }

    if (valid != NULL)
    {
        *valid = res;
    }
    return value;
}

double str2dbl(const string& str, bool* valid  /* = NULL */)
{
    char* end;
    bool res = true;

    double value = strtod(str.c_str(), &end);
    if ((*end != '\0'))
    {
        value = 0.0;
        res = false;
    }

    if (valid != NULL)
    {
        *valid = res;
    }
    return value;
}


std::string makeUTF8(uint charcode)
{
    std::string s;

    s.reserve(8);
    if (charcode < 0x80)
    {
        s = (char)charcode;
    }
    else
    {
        int firstbits = 6;
        const int otherbits = 6;
        int firstval = 0xC0;
        int t = 0;
        while (charcode >= (1 << firstbits))
        {
            t = 0x80 | (charcode & ((1 << otherbits)-1));
            charcode >>= otherbits;
            firstval |= 1 << (firstbits);
            firstbits--;
            s.insert(0, 1, (char)t);
        }
        t = firstval | charcode;
        s.insert(0, 1, (char)t);
    }
    return s;
}


uint makeUnicode(const char* s, int maxlen, int* lenused /*= NULL*/)
{
    assert(s);

    int charcode = 0;
    int ofs = 0;
    int t = (unsigned char)s[ofs];
    ofs++;

    if (t < 0x80)
    {
        charcode = t;
    }
    else
    {
        int highbitmask = (1 << 6) -1;
        int highbitshift = 0;
        int totalbits = 0;
        const int otherbits = 6;
        while ( ((t & 0xC0) == 0xC0) && (ofs < maxlen) )
        {
            t <<= 1;
            t &= 0xff;
            totalbits += 6;
            highbitmask >>= 1;
            highbitshift++;
            charcode <<= otherbits;

            charcode |= (int)s[ofs] & ((1 << otherbits)-1);
            ofs++;
        }
        charcode |= ((t >> highbitshift) & highbitmask) << totalbits;
    }

    if (lenused)
    {
        *lenused = ofs;
    }

    return charcode;
}


// Parser

Parser::Parser(const char* txt) :
    mTxt(txt),
    mTokParsed(false),
    mPos(0),
    mLineNum(1),
    mErr(false)
{
    if (txt)
    {
        mTxtLen = strlen(txt);
    }
    else
    {
        setError("NULL string");
    }
}


void Parser::parseToken()
{
    if (mErr)
    {
        mToken = "";
        return;
    }
    if (mTokParsed)
    {
        return;
    }

    TokenStateEnum state = NullTok;
    bool done = false;
    char lastc = '\0';
    char quotec = '\0';

    mToken.clear();
    mToken.reserve(128);
    while (!eof())
    {
        char c = mTxt[mPos];

        // Skip white space if not inside quotes.  It will also finish a token.

        if ((state != QuoteTok) && isspace(c))
        {
            if (state != NullTok)
            {
                done = true;
            }
            else if (c == '\n')
            {
                mLineNum++;
            }
        }
        else
        {
            // If a state has not been established, determine which state
            // to get into, otherwise read the rest of the token

            if (state == NullTok)
            {
                state = detState(c);
                if (state == QuoteTok)
                {
                    quotec = c;
                }

                append(c);
            }
            else
            {
                switch (state)
                {
                    case QuoteTok:
                    {
                        if (lastc == '\\')
                        {
                            // Escape char.
                            char escch = c;
                            int pos;
                            if (strfind(ESCAPE_CODES, escch, &pos))
                            {
                                escch = ESCAPE_CHARS[pos];

                                replaceLast(escch);
                                c = '\0';
                            }
                            else
                            {
                                // If the failure was due to a json hex code, read it now.
                                eraseLast();
                                if (c == 'u')
                                {
                                    string hex;
                                    for (int h = 0; h < 4; h++)
                                    {
                                        if (!eof())
                                        {
                                            mPos++;
                                            hex += mTxt[mPos];
                                        }
                                    }

                                    // If found 4 hex chars, convert to int, and the UTF8
                                    if (hex.length() == 4)
                                    {
                                        bool valid;
                                        uint cp = str2baseint(hex, 16, &valid);
                                        if (valid)
                                        {
                                            append(makeUTF8(cp));
                                        }
                                    }
                                }
                                else
                                {
                                    setError("Illegal escape char");
                                    done = true;
                                }
                            }

                        }
                        else
                        {
                            if (c != quotec)
                            {
                                append(c);
                            }
                            else
                            {
                                append(c);
                                if (!eof())
                                {
                                    lastc = c;
                                    mPos++;
                                }
                                done = true;
                            }
                        }
                    }
                    break;

                    case WordTok:
                    {
                        if (charWord(c))
                        {
                            append(c);
                        }
                        else
                        {
                            done = true;
                        }
                    }
                    break;

                    case BracTok:
                    {
                        done = true;
                    }
                    break;

                    case PuncTok:
                    {
                        if (charPunc(c))
                        {
                            append(c);
                        }
                        else
                        {
                            done = true;
                        }
                    }
                    break;

                    default:
                    // TODO: what should happen here.
                    break;
                }
            }
        }

        if (done)
        {
            break;
        }
        lastc = c;
        mPos++;
    }

    mTokParsed = true;
}


void Parser::stripQuotes(bool allowsingle)
{
    int l = mToken.length();
    char c1;
    char c2;

    if (l >= 2)
    {
        c1 = mToken[0];
        c2 = mToken[l - 1];

        if ((c1 == '"' && c2 == '"') ||
            (allowsingle && c1 == '\'' && c2 == '\'') )
        {
            mToken.erase(l - 1, 1);
            mToken.erase(0, 1);
        }
    }
}

void Parser::captureDelim(const char* delim)
{
    string s = token();

    while (!eof() && !tokenEquals(delim))
    {
        advance();
        if (!tokenEquals(delim))
        {
            s += token();
        }
    }
    mToken = s;
}

void Parser::setError(const char* msg)
{
    if (!mErr)
    {
        // format the error message but limit length.

        std::string tmsg(msg);
        tmsg.resize(48);
        format(mErrMsg, "Parser error: %s at line %d", tmsg.c_str(), mLineNum);
        mErr = true;
    }
}

void Parser::advance(const char* match)
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
        std::string s;
        format(s, "Expecting '%s' but found '%s' ", match, token().c_str());
        setError(s.c_str());
    }
}

} // jvar