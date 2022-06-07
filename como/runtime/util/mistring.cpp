/*************
 *
 * string utility
 *
 * Writen by Xilong Pei   Sep. 12  1989
 *
 **************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cassert>
#include "mistring.h"

namespace como {

/**
 *  A powerful wordbreak function to break a string to many word,
 *
 *  input:
 *        string: string to break
 *           num: max word number
 *     breakchar: break chars to break string, such as ".,`{}*&^%$#@!"
 *
 *  output:
 *     num  : return the word num
 *     word : return the wordptr
 *   return : first word ptr;
 */
char *MiString::WordBreak(char *string, int& num, char *word[], char *breakChar)
{
    char *rp;
    int maxNum = num;

    if (maxNum < 1)
        maxNum = 1;

    if ((nullptr == string) || (nullptr == word) || (nullptr == breakChar))
        return nullptr;

    num = 0;
    rp = string;

    while (isspace(*rp) || strchr(breakChar, *rp)) {
        rp++;   // skip top space
        if (*rp == '\0')
            break;
    }

    word[num] = rp;
    while (*rp) {
        while ((!isspace(*rp)) && (!strchr(breakChar, *rp)) && (*rp != '\0'))
            rp++;

        if (*rp) {
            *rp = '\0';
            rp++;
        }

        if (*rp && strchr(breakChar, *rp) != nullptr)
            rp++;

        while (isspace(*rp))
            rp++;   // skip blanks

        if (*rp && (strchr(breakChar, *rp) != nullptr))
            rp++;

        if (num >= maxNum)
            break;

        word[++num] = rp;
    }

    return word[0];
}


/**
 * seperate string s into pieces, such as:
 *  abc'def'asdafs' into abc   def   asdafs
 *  is seed is nullptr, alloc the pointer memory, else use seeds
 *  to store it.
 */
char **MiString::SeperateStr(char *s, char seperator, char **seeds, int& seedsCapacity)
{
    char *sz;

    if (seeds == nullptr) {
        seedsCapacity = 2;
        sz = s;
        while (*sz != '\0') {
            if (*sz++ == seperator)
                seedsCapacity++;
        }
        if ((seeds = (char **)calloc(seedsCapacity, sizeof(char *))) == nullptr) {
            return nullptr;
        }
    }

    int maxNum = seedsCapacity;
    seedsCapacity = 0;
    seeds[seedsCapacity] = s;
    while (*s) {
        if (seedsCapacity >= maxNum)
            break;

        if (*s == seperator)   {
            *s = '\0';
            seeds[++seedsCapacity] = ++s;
        }
        else {
            s++;
        }
    }

    return seeds;
}

/**
 * Seperate string into como::Array
 */
ECode MiString::SeperateStr(
    /* [in] */ char *s,
    /* [in] */ char seperator,
    /* [in] */ Integer limit,
    /* [out, callee] */ Array<String>* strArray)
{
    if ((nullptr == s) || (nullptr == strArray))
        return E_ILLEGAL_ARGUMENT_EXCEPTION;

    char *sz;
    int separatorCount = 1;
    sz = s;
    while (*sz != '\0') {
        if (*sz++ == seperator)
            separatorCount++;
    }

    if (limit <= 0)
        limit = separatorCount;
    else if (separatorCount > limit)
        separatorCount = limit;

    Array<String> result(separatorCount);
    if (result.IsNull()) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    int i = 0;
    sz = s;
    while ('\0' != *s) {
        if (*s == seperator)   {
            *s = '\0';
            result[i++] = String(sz);
            *s = seperator;
            sz = s + 1;

            if (i >= separatorCount)
                break;
        }
        s++;
    }

    if ((sz != s) && ((i < separatorCount))) {
        assert('\0' == *s);
        result[i++] = String(sz);
    }

    return NOERROR;
}

/**
 * shrink space in string
 */
char *MiString::shrink(char *dst, int dstSize, const char *src)
{
    int i, j = 0;

    dstSize--;
    dst[dstSize] = '\0';
    for (i = 0;  i < dstSize - 1 ;  i++) {
        if ('\0' == src[i]) {
            dst[j++] = '\0';
            break;
        }

        if (! isspace(src[i])) {
            dst[j++] = src[i];
        }
    }
    return dst;
}

/**
 * translate C++ style string into memory inside string
 */
char *MiString::cnStrToStr(char *t, char *s, char turnChar, size_t size)
{
    short i = 0;

    size--;     //for hold tail 0
    while ( *s != '\0' && *s != '"' && i < size ) {
        if ( *s == turnChar ) {
           switch( *(++s) ) {
            case 't':
                t[i++] = '\t';
                break;
            case 'n':
                t[i++] = '\n';
                break;
            case 'F':
                t[i++] = '\xFF';
                break;
            case '"':
                t[i++] = '"';
                break;

            case '\0':
                t[i] = '\0';
                return  t;

            case 'x':
            case 'X':
            {
                int j, num = 0;

                s++;
                for (j = 0;  j < 2 && *s;  j++ ) {
                    *s = toupper(*s);
                    if ( *s >= 'A' && *s <= 'Z' )
                        num = num * 16 + (*s - 'A' + 10 );
                    else if ( isdigit(*s) )
                        num = num * 16 + (*s - '0') ;
                    s++;
                }

                t[i++] = num;
                continue;
            }
            break;

            case '0':   case '1':   case '2':
            case '3':   case '4':   case '5':
            case '6':   case '7':   case '8':
            case '9':
            {
                int j, num = 0;

                for (j = 0;  j < 3 && *s;  j++ ) {
                    if (isdigit(*s)) {
                        num = num * 10 + (*s - '0') ;
                    } else {
                        break;
                    }
                    s++;
                }

                t[i++] = (unsigned char)num;
                continue;
            }
            break;

            default:
                t[i++] = *s;
                break;
           }
        } else {
            t[i++] = *s;
        }
        s++;
    } // end of while

    t[i] = '\0';

    return  t;

}  // end of function strToSourceC()

/**
 *
 * strZcpy()
 *
 */
char *MiString::strZcpy(char *dest, const char *src, size_t maxlen)
{
    maxlen--;
    dest[maxlen] = '\0';
    return  strncpy(dest, src, maxlen);
}

/**
 * strToSourceC
 */
char *MiString::strToSourceC(char *t, char *s, char turnChar)
{
    char *sz = t;

    while( *s != '\0' ) {
        if( *s == turnChar ) {
            *sz++ = turnChar;
            *sz++ = turnChar;
        } else {
            *sz++ = *s;
        }
        s++;
    } // end of while
    *sz = '\0';

    return  t;

}  // end of function strToSourceC()

/**
 * Call method:
 *      char *s1, *s2, *s3, *s4, *s;
 *      s = memNewBlockOnce(&s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
 */
char *MiString::memNewBlockOnce(char **ss, size_t memSize, ...)
{
    va_list ap;
    char **ss1;
    char *pool;
    unsigned short poolSize, count;

    poolSize = memSize;

    // count mem alloc
    va_start(ap, memSize);
    while ((ss1 = va_arg(ap, char **)) != nullptr) {
        poolSize += va_arg(ap, size_t);
    }
    va_end(ap);
    if (poolSize <= 0)
        return  nullptr;

    if ((pool = (char*)malloc(poolSize) ) == nullptr)
        return  nullptr;
    memset( pool, 0, poolSize );

    *ss = pool;
    count = memSize;

    va_start(ap, memSize);
    while ((ss1 = va_arg(ap, char **)) != nullptr) {
        *ss1 = (char *)(pool + count);
        count += va_arg(ap, size_t);
    }
    va_end(ap);

    return pool;

} // end of function memNewBlockOnce()

} // namespace como

#if 0
int main(int argc, char *argv[])
{
    int num = 2;
    char *word[3];
    char  buf[256];
    strcpy(buf, (char*)"name=tongji");
    char *str = como::MiString::WordBreak(buf, num, word, (char*)"=");
    printf("como::MiString::WordBreak():\n%s\n%s\n", word[0], word[1]);

    char s[80] = {"c:\\tg\\sub0.dbf"};
    char *seeds[10];
    int seedsCapacity = 10;
    como::MiString::SeperateStr(s, '\\', seeds, seedsCapacity);
    printf("%d\n%s\n%s\n%s\n", seedsCapacity, seeds[0], seeds[1], seeds[2]);

    char buf1[256];
    como::MiString::shrink(buf1, 256, (const char*)"unsigned char *src");
    printf("shrink: %s\n", buf1);

    return 0;
}
#endif
