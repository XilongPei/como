/****
*  mistring.h
*
*  Writen By Xilong Pei , 1991.10.15
****************************************************************************/

#ifndef __MISTRING_H_
#define __MISTRING_H_

#include <comosp.h>

namespace como {

class COM_PUBLIC MiString {
public:
    /**
     * A powerful wordbreak function to break a string to many word,
     *  copyed from MXSTRING.CPP
     *
     *  input:
     *        string: string to break
     *     breakchar: break chars to break string, such as ".,`{}*&^%$#@!"
     *
     *  output:
     *     num  : return the word num
     *     word : return the wordptr
     *   return : first word ptr;
     */
    static char *WordBreak(char *string, int& num, char *word[], char *breakChar);

    /**
     *  seperate string s into pieces, such as:
     *  abc'def'asdafs' into abc   def   asdafs
     *  is seed is NULL, alloc the pointer memory, else use seeds
     *  to store it.
     */
    static char **SeperateStr(char *s, char seperator, char **seeds,
                                                            int& seedsCapacity);

    static ECode SeperateStr(
                    /* [in] */ char *s,
                    /* [in] */ char seperator,
                    /* [in] */ Integer limit,
                    /* [out, callee] */ Array<String>* strArray);

    static char *shrink(char *dst, int dstSize, const char *src);

    static char *cnStrToStr(char *t, char *s, char turnChar, size_t size);

    static char *strZcpy(char *dest, const char *src, size_t maxlen);

    static char *strToSourceC(char *t, char *s, char turnChar);

    static char *MemNewBlockOnce(char *buf, size_t *poolSize, char **ss, int memSize, ...);

    static char *MemSetBlockOnce(char *pool, size_t poolSize, char *ss, int memSize, ...);

    static char *MemGetBlockOnce(char *pool, size_t poolSize, char *ss, int memSize, ...);
};

} // namespace como

#endif
