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
    int maxNum = seedsCapacity;

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
