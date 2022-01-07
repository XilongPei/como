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
char *MiString::WordBreak(char *string, int *num, char *word[], char *breakChar)
{
    char *rp;
    int maxNum = *num;

    if (maxNum < 1)
        maxNum = 1;

    if ((nullptr == string) || (nullptr == num)  || (nullptr == word) || (nullptr == breakChar))
        return nullptr;

    *num = 0;
    rp = string;

    while (*rp == ' ' || *rp == '\t' || strchr(breakChar, *rp)) {
        rp++; // skip top space
        if (*rp == '\0')
            break;
    }

    word[*num] = rp;
    while (*rp != '\0') {
        while ((*rp != ' ') && (!strchr(breakChar, *rp)) && (*rp != '\0'))
            rp++;

        if (*rp != '\0') {
            *rp = '\0';
            rp++;
        }

        if ((*rp != '\0') && strchr(breakChar, *rp) != nullptr)
            rp++;

        while (*rp == ' ')
            rp++; // skip blanks

        if ((*rp != '\0') && (strchr(breakChar, *rp) != nullptr))
            rp++;

        word[++(*num)] = rp;
    }

    return word[0];
}


/**
 * seperate string s into pieces, such as:
 *  abc'def'asdafs' into abc   def   asdafs
 *  is seed is nullptr, alloc the pointer memory, else use seeds
 *  to store it.
 */
int MiString::SeperateStr(char *s, char seperator, char **seeds, int seedsCapacity)
{
    short count;
    char *sz;

    if (seeds == nullptr) {
        count = 2;
        sz = s;
        while (*sz != '\0') {
            if (*sz++ == seperator)
                count++;
        }
        if ((seeds = (char **)calloc(count, sizeof(char *))) == nullptr) {
            return -1;
        }
    }

    count = 0;
    seeds[count] = s;
    while (*s) {
        if (count >= seedsCapacity)
            break;

        if (*s == seperator)   {
            *s = '\0';
            seeds[++count] = ++s;
        }
        else {
            s++;
        }
    }
    seedsCapacity = count;

    return count;

}

} // namespace como

int main(int argc, char *argv[])
{
    int num = 2;
    char *word[3];
    char  buf[256];
    strcpy(buf, (char*)"name=tongji");
    char *str = como::MiString::WordBreak(buf, &num, word, (char*)"=");
    printf("como::MiString::WordBreak():\n%s\n%s\n", word[0], word[1]);

    char s[80] = {"c:\\tg\\sub0.dbf"};
    char *seeds[10];
    int i = como::MiString::SeperateStr(s, '\\', seeds, 10);
    printf("%d\n%s\n%s\n%s\n", i, seeds[0], seeds[1], seeds[2]);

    return 0;
}
