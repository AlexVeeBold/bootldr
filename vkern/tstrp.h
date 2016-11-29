///////////////////////////////////////////////////////////////
//
// String functions [code] (use with Tiny Unicode helper)
//
//
//   09.07.2009 11:42 - created
//


EXTERN_C_BEGIN //extern "C" {


//////////////// //////////////// //////////////// ////////////////

#ifdef _TSTR_CLPASS_


#undef tchlwr
//#undef tszlwr
#ifndef UNICODE
#define tchlwr      achlwr
//#define tszlwr      aszlwr
#else //UNICODE
#define tchlwr      wchlwr
//#define tszlwr      wszlwr
#endif //UNICODE

#undef tszchr
#undef tszrchr
#ifndef UNICODE
#define tszchr      aszchr
#define tszrchr     aszrchr
#else //UNICODE
#define tszchr      wszchr
#define tszrchr     wszrchr
#endif //UNICODE

#undef tszLength
#ifndef UNICODE
#define tszLength   aszLength
#else //UNICODE
#define tszLength   wszLength
#endif //UNICODE

#undef tszConcat
#ifndef UNICODE
#define tszConcat   aszConcat
#else //UNICODE
#define tszConcat   wszConcat
#endif //UNICODE

#undef tszCompare
#ifndef UNICODE
#define tszCompare  aszCompare
#else //UNICODE
#define tszCompare  wszCompare
#endif //UNICODE

#undef tszIntToStr
#undef tszStrToInt
#ifndef UNICODE
#define tszIntToStr aszIntToStr
#define tszStrToInt aszStrToInt
#else //UNICODE
#define tszIntToStr wszIntToStr
#define tszStrToInt wszStrToInt
#endif //UNICODE


// Converts character to lowercase
//      returns lowercase character
TCHAR tchlwr(TCHAR ch)
{
    // tiny version: converts only a-z chars
    BOOL bInBounds;
    TCHAR tchRet;
    bInBounds = (ch >= CAPACHAR) & (ch <= CAPZCHAR);
    tchRet = ch;
    if(bInBounds != FALSE)
    {
        tchRet = (TCHAR)((INT)tchRet + ((INT)SMALLACHAR - (INT)CAPACHAR));
    }
    return tchRet;
}

//// Converts string to lowercase
////      returns string
//TSTR tszlwr(TSTR string)
//{
//    return _Win32CharLower(string);
//}

// Searches a string for a given character, which 
// may be the null character '\0'.
//      returns pointer to the first occurence of ch in string
//      returns NULL if ch does not occur in string
TSTR tszchr(CTSTR string, TCHAR ch)
{
    while(*string != ZEROCHAR)
    {
        if(*string == ch)
        {
            return (TSTR)string;
        }
        string++;
    }
    return NULL;
}

// Finds the last occurrence of ch in string.
// The terminating null character is used as part of the search.
//      returns a pointer to the last occurrence of ch in the string
//      returns NULL if ch does not occurr in the string
TSTR tszrchr(CTSTR string, TCHAR ch)
{
    TCHAR* last = NULL;

    while(*string != ZEROCHAR)
    {
        if(*string == ch)
        {
            last = (TSTR)string;    // store last found char position
        }
        string++;
    }
    return last;
}

// Finds the length in characters of the given string, 
// excluding the final null character.
// Search stops after count characters have been passed.
//      returns length of the str, without of the final null char
DWORD tszLength(CTSTR str, DWORD count)
{
    CTCHAR* eos = str;

    while((count != 0) && (*eos != ZEROCHAR))
    {
        count--;
        eos++;
    }

    return ((DWORD)(eos - str));
}

// Copies OR Appends at most count characters of the src to dest 
// OR to the end of dest, and ALWAYS terminates with a null character.
// If count is greater than the length of src, the length of src
// is used instead.
// (Unlike strncat, this routine does not terminate with null character, 
// if count is less than the length of source)
// (Unlike strncpy, this routine does not pad out to count characters)
// (Unlike strcpy, this routine returns count of copied characters, 
// except terminating null character)
//      returns count of characters copied
DWORD tszConcat(TSTR dst, CTSTR src, DWORD count, BOOL append)
{
    TCHAR* ret = dst;

    if(append != FALSE)
    {
        while(*dst != ZEROCHAR)     // find end of dst
            dst++;
    }

    while(count != 0)               // copy src to dst
    {
        *dst = *src;
        if(*dst == ZEROCHAR)
            break;
        dst++;
        src++;
        count--;
    }

    return ((DWORD)(dst - ret));
}

// Compares two strings, ignoring case, and returns an integer 
// to indicate whether the first is less than the second, the 
// two areequal, or whether the first is greater than the second.
// Comparison stops after count characters have been compared.
// Upper case characters are converted to lower case.
//      returns <0 if str1 <  str2
//      returns  0 if str1 == str2
//      returns >0 if str1 >  str2
int tszCompare(CTSTR str1, CTSTR str2, DWORD count, BOOL ignoreCase)
{
    int diff = 0;
    TCHAR ch1;
    TCHAR ch2;

    if(count > 0)
    {
        do {
            ch1 = *str1;
            ch2 = *str2;
            if(ignoreCase != FALSE)
            {
                ch1 = tchlwr(ch1);
                ch2 = tchlwr(ch2);
            }
            diff = (int)ch1 - (int)ch2;
            str1++;
            str2++;
            count--;
        } while( (count != 0) && (diff == 0) && (ch2 != ZEROCHAR) );
    }

    return diff;
}

// Converts number to string. If number of output digits is smaller 
// than width, zeroes are added from left side. Radix can be any number 
// between 2 and 36 (inclusive).
//      returns count of characters written
DWORD tszIntToStr(TSTR str, int num, DWORD width, DWORD radix, BOOL bUpCase, BOOL bSigned)
{
    DWORD nSize;
    DWORD nNumber;
    DWORD nDigit;
    DWORD nRet;
    TCHAR tchA;

    if((radix < 2) || (radix > 36))    // 0...9 + A...Z
    {
        return 0;
    }
    nSize = 0;
    if(width == 0)
    {
        width = 1;
    }
    //if(num == 0)        // if it's zero, then it's zero
    //{
    //    *str = NUMZEROCHAR;
    //    return 1;
    //}
    if(radix != 10)     // set all non-decimal to non-signed (12.08.2010)
    {
        bSigned = FALSE;
    }
    if((num < 0) & (bSigned != FALSE))
    {
        *str = MINUSCHAR;
        nSize++;
    }
    if(bUpCase != FALSE)
        tchA = CAPACHAR;
    else
        tchA = SMALLACHAR;
    nNumber = num;
    while(nNumber != 0)         // get # of digits
    {
        nNumber /= radix;
        nSize++;
    }
    nRet = nSize;
    if(width > nSize)
    {
        nRet = width;
        while(width > nSize)    // pad with zeroes
        {
            *str = NUMZEROCHAR;
            str++;
            width--;
        }
    }
    str += nSize;
    *str = ZEROCHAR;        // add trailing zero
    str--;
    nNumber = num;
    while(nNumber != 0)     // print number
    {
        nDigit = nNumber % radix;
        nNumber /= radix;
        if(nDigit < 10)
            *str = (TCHAR)(NUMZEROCHAR + nDigit);
        else
            *str = (TCHAR)(tchA + (nDigit - 10));
        str--;
    }
    return nRet;
}

// Converts string to number. Radix can be any number 
// between 2 and 36 (inclusive).
//      returns TRUE if successful, FALSE otherwise
BOOL tszStrToInt(int* pnum, CTSTR str, DWORD radix)
{
    TCHAR tchDigit;
    TCHAR tchMax;
    DWORD nDigit;
    int nNumber;
    BOOL bNegative;

    if((radix == 0) | (radix > 36))    // 0...9 + A...Z
    {
        return FALSE;
    }
    if(pnum == NULL)
    {
        return FALSE;
    }
    // set max acceptable digit character & skip whitespaces
    if(radix < 10)
    {
        tchMax = NUMZEROCHAR + ((TCHAR)radix);
        while((*str != ZEROCHAR) && (*str != PLUSCHAR) && (*str != MINUSCHAR)
            && (*str < NUMZEROCHAR) && (*str > tchMax))
        {
            str++;
        }
    }
    else
    {
        tchMax = SMALLACHAR + ((TCHAR)(radix - 10));
        do {
            tchDigit = tchlwr(*str);
            if( ((tchDigit >= NUMZEROCHAR) && (tchDigit <= NUMNINECHAR)) 
                || ((tchDigit >= SMALLACHAR) && (tchDigit <= tchMax)) )
            {
                break;
            }
            str++;
        } while((*str != ZEROCHAR) && (*str != PLUSCHAR) && (*str != MINUSCHAR));
    }
    // detect sign
    bNegative = FALSE;
    if(*str == MINUSCHAR)
    {
        bNegative = TRUE;
        str++;
    }
    if(*str == PLUSCHAR)
    {
        str++;
    }
    // convert
    nNumber = 0;
    if(radix < 10)  // simple number
    {
        do {
            tchDigit = *str;
            if((tchDigit >= NUMZEROCHAR) && (tchDigit <= tchMax))
            {
                nDigit = (DWORD)(tchDigit - NUMZEROCHAR);
            }
            else
            {
                break;
            }
            nNumber *= radix;
            nNumber += nDigit;
            str++;
        } while(*str != ZEROCHAR);
    }
    else            // complex number
    {
        do {
            tchDigit = tchlwr(*str);
            if((tchDigit >= NUMZEROCHAR) && (tchDigit <= NUMNINECHAR))
            {
                nDigit = (DWORD)(tchDigit - NUMZEROCHAR);
            }
            else if((tchDigit >= SMALLACHAR) && (tchDigit <= tchMax))
            {
                nDigit = (DWORD)(tchDigit - SMALLACHAR);
            }
            else
            {
                break;
            }
            nNumber *= radix;
            nNumber += nDigit;
            str++;
        } while(*str != ZEROCHAR);
    }
    // apply sign
    if(bNegative != FALSE)
    {
        nNumber = -nNumber;
    }
    *pnum = nNumber;
    return TRUE;
}


#endif //_TSTR_CLPASS_


EXTERN_C_END //} //extern "C"

