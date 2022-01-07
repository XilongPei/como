/****
*  mistring.h
*
*  Writen By Xilong Pei , 1991.10.15
****************************************************************************/

#ifndef __MISTRING_H_
#define __MISTRING_H_

namespace como {

class MiString {
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
	static char *WordBreak(char *string, int *num, char *word[], char *breakChar);

	/**
	 *  seperate string s into pieces, such as:
	 *  abc'def'asdafs' into abc   def   asdafs
	 *  is seed is NULL, alloc the pointer memory, else use seeds
	 *  to store it.
	 */
	static int SeperateStr(char *s, char seperator, char **seeds, int seedsCapacity);
};

} // namespace como

#endif
