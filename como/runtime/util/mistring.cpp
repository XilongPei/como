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
#include "int64.h"
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

    if (maxNum < 1) {
        maxNum = 1;
    }

    if ((nullptr == string) || (nullptr == word) || (nullptr == breakChar)) {
        return nullptr;
    }

    num = 0;
    rp = string;

    // skip the leading breakChar
    if ('\0' != *rp) {
        while (isspace(*rp) || strchr(breakChar, *rp)) {
            rp++;
            if (*rp == '\0') {
                break;
            }
        }
    }

    word[num] = rp;
    while ('\0' != *rp) {

        while ((!isspace(*rp)) && (!strchr(breakChar, *rp)) && (*rp != '\0')) {
            rp++;
        }

        if (*rp != '\0') {
            *rp = '\0';
            rp++;
        }
        else {
            num++;
            break;
        }

        // skip to next word
        while (isspace(*rp) || strchr(breakChar, *rp)) {
            rp++;
            if (*rp == '\0') {
                break;
            }
        }

        if (*rp == '\0') {
            num++;
            break;
        }

        if (num >= maxNum) {
            break;
        }

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

    if (nullptr == s) {
        seedsCapacity = 0;
        return nullptr;
    }

    if (nullptr == seeds) {
        seedsCapacity = 1;
        sz = s;
        while (*sz != '\0') {
            if (*sz++ == seperator) {
                seedsCapacity++;

                while (*sz == seperator) {
                    sz++;
                }
            }
        }
        if ((seeds = (char **)calloc(seedsCapacity, sizeof(char *))) == nullptr) {
            return nullptr;
        }
    }
    else {
        if (seedsCapacity <= 0) {
            return nullptr;
        }
    }

    int maxNum = seedsCapacity;
    seedsCapacity = 0;

    while (*s == seperator) {
        s++;
    }

    seeds[seedsCapacity++] = s;
    while (*s != '\0') {
        if (seedsCapacity >= maxNum) {
            while ((*s != '\0') && (*s != seperator)) {
                s++;
            }
            *s = '\0';

            break;
        }

        if (*s == seperator)   {
            *s++ = '\0';

            while (*s == seperator) {
                s++;
            }

            if (*s != '\0') {
                seeds[seedsCapacity++] = s++;
            }

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
    if ((nullptr == s) || (nullptr == strArray)) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    char *sz;
    int separatorCount = 1;
    sz = s;
    while (*sz != '\0') {
        if (*sz++ == seperator) {

            while (*sz != '\0') {
                if (*sz == seperator) {
                    sz++;
                }
                else {
                    break;
                }
            }

            if (*sz != '\0') {
                separatorCount++;
            }
        }
    }

    if (limit <= 0) {
        limit = separatorCount;
    }
    else if (separatorCount > limit) {
        separatorCount = limit;
    }

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

            if (i >= separatorCount) {
                break;
            }

            while (*s != '\0') {
                if (*s == seperator) {
                    s++;
                }
                else {
                    break;
                }
            }

            if (*s != '\0') {
                sz = s;
            }
        }
        s++;
    }

    if ((sz != s) && ((i < separatorCount))) {
        assert('\0' == *s);
        result[i++] = String(sz);
    }

    *strArray = result;

    return NOERROR;
}

/**
 * shrink space in string
 */
char *MiString::shrink(char *dst, int dstSize, const char *src)
{
    int j = 0;

    if (dstSize <= 0) {
        return nullptr;
    }

    dstSize--;
    dst[dstSize] = '\0';
    for (int i = 0;  i < dstSize;  i++) {
        if (src[j] == '\0') {
            dst[i] = '\0';
            break;
        }
        while (isspace(src[j])) {
            j++;
        }
        dst[i] = src[j++];
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
    while ((*s != '\0') && (i < size)) {
        if (*s == turnChar) {
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
                    if ( *s >= 'A' && *s <= 'Z' ) {
                        num = num * 16 + (*s - 'A' + 10 );
                    }
                    else if ( isdigit(*s) ) {
                        num = num * 16 + (*s - '0') ;
                    }
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
                    }
                    else {
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
        }
        else {
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
    return strncpy(dest, src, maxlen);
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
        }
        else {
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
 *      size_t poolSize;
 *      s = MemNewBlockOnce(nullptr, &poolSize, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
 */
char *MiString::MemNewBlockOnce(char *buf, size_t *poolSize, char **ss, int memSize, ...)
{
    va_list ap;
    char **sp;
    char *pool;
    size_t count;

    count = memSize;

    // count mem alloc
    va_start(ap, memSize);
    while ((sp = va_arg(ap, char **)) != nullptr) {
        count += va_arg(ap, size_t);
    }
    va_end(ap);
    if (count <= 0) {
        return  nullptr;
    }

    if (nullptr != buf) {
        if ((nullptr != poolSize) && (*poolSize < count)) {
            return nullptr;
        }
        pool = buf;
    }
    else {
        if ((pool = (char*)malloc(count)) == nullptr) {
            return  nullptr;
        }
    }

    (void)memset(pool, 0, count);

    if (nullptr != poolSize) {
        *poolSize = count;
    }

    *ss = pool;
    count = memSize;

    va_start(ap, memSize);
    while ((sp = va_arg(ap, char **)) != nullptr) {
        *sp = (char *)(pool + count);
        count += va_arg(ap, size_t);
    }
    va_end(ap);

    return pool;

} // end of function MemNewBlockOnce()

/**
 * Set data to the pool
 *
 * Call method:
 *      char *s;
 *      int  i;
 *      char buf[4096];
 *      char buf_s1[32];
 *      strcpy(buf_s1, "tongji");
 *      i = 4800;
 *      s = MiString::MemSetBlockOnce(buf, 4096, buf_s1, 0, (char*)&i, sizeof(int), nullptr);
 */
char *MiString::MemSetBlockOnce(char *pool, size_t poolSize, char *ss, int memSize, ...)
{
    va_list ap;
    char *sp;
    int count;

    if (nullptr == pool) {
        return nullptr;
    }

    if (memSize <= 0) {
        // The current content (memSize corresponding to ss) is a string, and
        // its size is unknown.
        count = strlen(ss) + 1;
        if (count > poolSize) {
            return nullptr;
        }
        (void)strcpy(pool, ss);
    }
    else {
        count = memSize;
        (void)memcpy(pool, ss, memSize);
    }

    va_start(ap, memSize);
    while ((sp = va_arg(ap, char *)) != nullptr) {
        memSize = va_arg(ap, int);

        if (memSize <= 0) {
            memSize = strlen(sp) + 1;
            count += memSize;
            if (count > poolSize) {
                return nullptr;
            }

            (void)strZcpy((char *)(pool + count), sp, memSize);
        }
        else {
            count += memSize;
            if (count > poolSize) {
                return nullptr;
            }

            (void)memcpy((char *)(pool + count), sp, memSize);
        }
    }
    va_end(ap);

    return pool;
} // end of function MemSetBlockOnce()

/**
 * Fetch data from the pool
 *
 * Call method:
 *      char *s;
 *      int  i;
 *      char buf[4096];
 *      char buf_s1[32];
 *      strcpy(buf, "tongji");
 *      *(int *)&buf[7] = 4800;
 *      s = MiString::memGetBlockOnce(buf, 4096, buf_s1, -sizeo(buf_s1),
 *                                    (char*)&i, sizeof(int), nullptr);
 */
char *MiString::MemGetBlockOnce(char *pool, size_t poolSize, char *ss, int memSize, ...)
{
    va_list ap;
    char *sp;
    int count;

    if (nullptr == pool) {
        return nullptr;
    }

    if (0 == memSize) {
        return nullptr;
    }
    else if (memSize < 0) {
        // The current content (memSize corresponding to ss) is a string, and
        // its size is unknown.
        count = strlen(pool) + 1;
        memSize = -memSize;
        if (memSize < count) {
            return nullptr;
        }

        (void)strZcpy(ss, pool, memSize);
    }
    else {
        count = memSize;
        (void)memcpy(ss, pool, memSize);
    }

    va_start(ap, memSize);
    while ((sp = va_arg(ap, char *)) != nullptr) {
        memSize = va_arg(ap, int);
        if (0 == memSize) {
            return nullptr;
        }
        else if (memSize < 0) {
            memSize = -memSize;

            /**
             * memSize indicates that the target memory has such a large space
             * that the actual string fetched may be shorter than this.
             */
            (void)strZcpy(sp, (char *)(pool + count), memSize);
            count += memSize;
        }
        else {
            (void)memcpy(sp, (char *)(pool + count), memSize);
            count += memSize;
        }

        if (count > poolSize) {
            return nullptr;
        }

    }
    va_end(ap);

    return pool;
} // end of function MemGetBlockOnce()

} // namespace como

#if 0
int main(int argc, char *argv[])
{

    char *s1, *s2, *s3, *s4, *sp;
    char buff[4096];
    sp = como::MiString::MemGetBlockOnce(buff, 4096, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);


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
