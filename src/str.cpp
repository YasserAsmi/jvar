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

void trimLeft(std::string& str)
{
    size_t len = str.size();
    size_t i = 0;
    while (i < len && isspace(str[i]))
    {
        i++;
    }
    str.erase(0, i);
}

void trimRight(std::string& str)
{
    size_t i = str.size();
    while (i > 0 && isspace(str[i - 1]))
    {
        i--;
    }
    str.erase(i, str.size());
}

bool equal(const char* s1, const char* s2)
{
    if (s1 && s2)
    {
        return (strcmp(s1, s2) == 0);
    }
    else
    {
        return (s1 == s2);
    }
}

bool equalCI(const char* s1, const char* s2)
{
    if (s1 && s2)
    {
        return (strcasecmp(s1, s2) == 0);
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

string formatr(const char* fmt, ...)
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
    longint value = strtol(str.c_str(), &end, base);
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

uint strHashSedgewick(const char* str, size_t len)
{
    // Robert Sedgewick hash function
    uint b = 378551;
    uint a = 63689;
    uint hash = 0;

    while (*str)
    {
        hash = hash * a + (*str);
        a = a * b;
        str++;
    }

    return hash;
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
        while ((int)charcode >= (1 << firstbits))
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


void replaceAll(std::string& str, const std::string& match, const std::string& with)
{
    size_t startpos = 0;
    while ((startpos = str.find(match, startpos)) != std::string::npos)
    {
        str.replace(startpos, match.length(), with);
        startpos += with.length();
    }
}

std::string pathParent(const std::string p, const char* delim)
{
    if (delim == NULL)
    {
        // TODO: use OS specific value
        delim = "/";
    }
    size_t pos = p.find_last_of(delim);

    if (pos == std::string::npos)
    {
        return "";
    }
    return p.substr(0, pos);
}

std::string pathChild(const std::string p, const char* delim)
{
    if (delim == NULL)
    {
        // TODO: use OS specific value
        delim = "/";
    }
    return p.substr(p.find_last_of(delim) + 1);
}

std::string pathThisProc(const char* replfn)
{
    std::string path;
    char buf[1024];
    ssize_t ret = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (ret != -1)
    {
        buf[ret] = '\0';
        path.assign(buf, ret);
    }
    if (replfn != NULL)
    {
        path = pathParent(path);
        path += "/";
        path += replfn;
    }
    return path;
}

// StrBld::

bool StrBld::appendFmt(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    bool ret = appendVFmt(fmt, va);
    va_end(va);
    return ret;
}


bool StrBld::appendVFmt(const char* fmt, va_list varg)
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
        append(stackbuf);
    }
    else
    {
        // Format the string with a sufficiently large heap buffer.
        char* buf = ensureAlloc(mLen + count + 1);
        buf += mLen;
        int fincount = vsnprintf((char*)buf, count + 1, fmt, orgvarg);
        if (fincount < 0)
        {
            return false;
        }
        mLen += fincount;
    }
    return true;
}


void StrBld::stripQuotes(bool allowsingle)
{
    char c1;
    char c2;

    if (mLen >= 2)
    {
        char* buf = (char*)mBuf.ptr();

        c1 = buf[0];
        c2 = buf[mLen - 1];

        if ((c1 == '"' && c2 == '"') ||
            (allowsingle && c1 == '\'' && c2 == '\'') )
        {
            mLen--;

            if (mLen >= 1)
            {
                memmove(buf, buf + 1, mLen);
                mLen--;
            }
        }
    }
}


// Parser::

Parser::Parser(const char* txt) :
    mTxt(txt),
    mToken(128),
    mTokParsed(false),
    mPos(0),
    mLineNum(1),
    mErr(false),
    mTokStartPos(0),
    mTokEndPos(0),
    mSinglePunc(false)
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


void Parser::internalParse()
{
    TokenStateEnum state = NullTok;
    bool done = false;
    char lastc = '\0';
    char quotec = '\0';

    mToken.clear();
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

                mTokStartPos = mPos;

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
                        bool besingle = (lastc == ':') || (lastc == ',');
                        if (!mSinglePunc && !besingle && charPunc(c))
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

    mTokEndPos = mPos;

    mTokParsed = true;
}


void Parser::captureDelim(const char* delim)
{
    StrBld s(token());

    int startpos = mTokStartPos;
    int endpos = mTokEndPos;

    while (!eof() && !tokenEquals(delim))
    {
        advance();
        if (!tokenEquals(delim))
        {
            s.append(mToken);
            endpos = mTokEndPos;
        }
    }
    mToken.moveFrom(s);

    mTokStartPos = startpos;
    mTokEndPos = endpos;
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


void Parser::expectErr(char c, const char* str)
{
    char tmp[2];
    std::string s;
    ensureTok();
    if (c)
    {
        tmp[0] = c;
        tmp[1] = '\0';
        str = tmp;
    }
    format(s, "Expecting [%s] but found [%s] ", str, c_str());
    setError(s.c_str());
}


Parser::TokenStateEnum Parser::tokType()
{
    ensureTok();

    char c = mToken[0];

    if (c != '\0')
    {
        return detState(c);
    }

    return NullTok;
}

// Replacer::

void Replacer::setSrc(const char* str)
{
    mOrgStr = str;
    mOrgMax = strlen(str) + 1;
    mOrgPos = 0;
    mBufPos = 0;
    mBuf.alloc(mOrgMax + 1);
}

bool Replacer::replace(int orgpos, int orglen, const char* with)
{
    dbgtru(mOrgStr != NULL);

    if (mOrgStr == NULL || with == NULL)
    {
        return false;
    }
    if (orgpos > mOrgPos)
    {
        copy(mOrgStr + mOrgPos, orgpos - mOrgPos, orgpos - mOrgPos);
    }

    copy(with, strlen(with), orglen);

    return true;
}

const char* Replacer::c_str()
{
    if (mOrgStr && mOrgPos < mOrgMax)
    {
        copy(mOrgStr + mOrgPos, mOrgMax - mOrgPos, mOrgMax - mOrgPos);
        mOrgStr = NULL;
    }

    return (const char*)mBuf.cptr();
}

std::string Replacer::str()
{
    std::string s;
    const char* p = c_str();

    if (p)
    {
        s.assign(p);
    }

    return s;
}


void Replacer::copy(const char* from, int len, int orglen)
{
    if (len > 0)
    {
        ensureAlloc(mBufPos + len + 1);

        memcpy((char*)mBuf.ptr() + mBufPos, from, len);
        mBufPos += len;
    }

    mOrgPos += orglen;
}

void Replacer::ensureAlloc(size_t neededsize)
{
    if (neededsize > mBuf.size())
    {
        size_t newsize = mBuf.size() * 2;
        if (newsize < neededsize)
        {
            newsize = neededsize;
        }

        mBuf.reAlloc(neededsize);
    }
}


// Splitter::

std::string Splitter::get()
{
    std::string s;
    if (!eof())
    {
        captureDelim(mDelim.c_str());
        s = tokFullStr();
        advance();
    }
    if (s.compare(mDelim) == 0)
    {
        s.clear();
    }
    return s;
}



} // jvar