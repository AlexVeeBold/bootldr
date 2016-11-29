///////////////////////////////////////////////////////////////
//
// tprintf: Formatted print to string
//
//
//   28.04.2008 16:29 - created
//   10.09.2010 14:28 - copied into vkern
//   14.09.2010 04:10 - removed bugs, optimized
//   21.10.2010 06:02 - fixed non-printing zero-value bug
//


#include "ktypes.h"

#include <stdarg.h>     // va_list, va_arg, etc

//#define UNICODE
//#include "tunic.h"

#include "kstring.h"
#include "tprintf.h"


/*  argument passing:
  type      on stack                value
  byte      dword                   low byte of pushed dword
  word      dword                   low word of pushed dword
  dword     dword                   as is
  qword     dword(h),dword(l)       as is (high dword pushed first, low dword pushed last)
*/



//  type format: 'K', family_byte, typesize_word
#define ARGTYPE_SIGNMASK    0xFF000000
#define ARGTYPE_TYPEMASK    0x00FF0000
#define ARGTYPE_ARG         0x4B000000
#define ARGTYPE_INTEGER     0x00000000
#define ARGTYPE_BOOLEAN     0x00010000
#define ARGTYPE_KSTRING     0x00020000
#define ARGTYPE_WSTRING     0x00030000
#define INTTYPE_SIGNED      0x00008000
#define INTTYPE_SIZEMASK    0x000000FF


/*
A format specification, which consists of optional and required fields, 
has the following form:

 %[flags] [width] [.precision] type

Flags
 -
 Left align the result within the given field width. 
 Default: Right align.
 
 +
 Prefix the output value with a sign (+ or -) if the output value is of a signed type.
 Default: Sign appears only for negative signed values (-).
 
 0
 If width is prefixed with '0', zeros are added until the minimum width is reached. 
 If '0' and '-' appear, the '0' is ignored. If '0' is specified with an non-integer 
 format (c, d, x, X), the '0' is ignored. 
 Default: No padding.


Width
 The width argument is a nonnegative decimal integer controlling the minimum number 
 of characters printed. If the number of characters in the output value is less than 
 the specified width, blanks are added to the left or the right of the values - depending 
 on whether the '-' flag (for left alignment) is specified - until the minimum width is 
 reached. If width is prefixed with '0', zeros are added until the minimum width is 
 reached (not useful for left-aligned numbers).
   The width specification never causes a value to be truncated. If the number of 
 characters in the output value is greater than the specified width, or if width is not 
 given, all characters of the value are printed.

Precision
 Types: c, d, x, X, b, B, w
 The precision has no effect.
 Default: Output value is printed.

 Types: s
 The precision specifies the maximum number of characters to be printed. Characters 
 in excess of precision are not printed.
 Default: Characters are printed until a null character is encountered.

Type
 Character: c
 Type: integer (BYTE, WORD, DWORD, QWORD)
 Output format: Wide-character string. Useful for printing ANSI-string signatures 
 packed into integer.

 Character: d
 Type: integer (BYTE, WORD, DWORD, QWORD, CHAR, SHORT, LONG, LLONG)
 Output format: Decimal integer

 Character: x
 Type: integer (BYTE, WORD, DWORD, QWORD, CHAR, SHORT, LONG, LLONG)
 Output format: Hexadecimal integer, using "abcdef"

 Character: X
 Type: integer (BYTE, WORD, DWORD, QWORD, CHAR, SHORT, LONG, LLONG)
 Output format: Hexadecimal integer, using "ABCDEF"

 Character: b
 Type: boolean (BOOL)
 Output format: "true" or "false"

 Character: B
 Type: boolean (BOOL)
 Output format: "TRUE" or "FALSE"

 Character: s
 Type: string (KSTR)
 Output format: Wide-character string. Characters are printed up to the first 
 null character or until the string buffer length is reached.
 
 Character: w
 Type: string (ANSI, UNICODE)
 Output format: Wide-character string. Characters are printed up to the first 
 null character or until the string buffer length is reached.
 
*/

// character types
enum CHARTYPE {
    CH_OTHER,           // 0 // non-special character
    CH_PERCENT,         // 1 // '%'
    CH_FLAG,            // 2 // '-', '+'
    CH_DOT,             // 3 // '.'
    CH_ZERO,            // 4 // '0'
    CH_DIGIT,           // 5 // '1'..'9'
    CH_TYPE,            // 6 // 'c', 'd', 'x', 'X', 'b', 'B', 's', 'w'
    // must be LAST entry
    NUM_CHARTYPES       // 7
};

const char _charTable[0x60] = {
    //SPC ! " # $ % & '   ( ) * + , - . / 
    CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_PERCENT, CH_OTHER, CH_OTHER, 
    CH_OTHER, CH_OTHER, CH_OTHER, CH_FLAG, CH_OTHER, CH_FLAG, CH_DOT, CH_OTHER, 
    //  0 1 2 3 4 5 6 7   8 9 : ; < = > ? 
    CH_ZERO, CH_DIGIT, CH_DIGIT, CH_DIGIT, CH_DIGIT, CH_DIGIT, CH_DIGIT, CH_DIGIT, 
    CH_DIGIT, CH_DIGIT, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, 
    //  @ A B C D E F G   H I J K L M N O 
    CH_OTHER, CH_OTHER, CH_TYPE, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, 
    CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, 
    //  P Q R S T U V W   X Y Z [ \ ] ^ _ 
    CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, 
    CH_TYPE, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, 
    //  ` a b c d e f g   h i j k l m n o 
    CH_OTHER, CH_OTHER, CH_TYPE, CH_TYPE, CH_TYPE, CH_OTHER, CH_OTHER, CH_OTHER, 
    CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, 
    //  p q r s t u v w   x y z { | } ~ BKS
    CH_OTHER, CH_OTHER, CH_OTHER, CH_TYPE, CH_OTHER, CH_OTHER, CH_OTHER, CH_TYPE, 
    CH_TYPE, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER, CH_OTHER
};
// state definitions
enum STATE {
    ST_NORMAL,              // 0 // normal state: outputting literal chars
    ST_REVERT=ST_NORMAL,    // 0 // reverted to normal state (bad sequence)
    ST_PERCENT,             // 1 // just read '%'
    ST_FLAG,                // 2 // just read flag character
    ST_WIDTH,               // 3 // just read width specifier
    ST_DOT,                 // 4 // just read '.'
    ST_PRECIS,              // 5 // just read precision specifier
    ST_TYPE,                // 6 // just read type specifier
    // must be LAST entry
    NUM_STATES              // 7
};
/*
 %[flags] [width] [.precision] type

  %             '%'
  flags         '-' '+' '0'
  width         integer
  dot           '.'
  precision     integer
  type          'c' 'd' 'x' 'X' 'b' 'B' 's' 'w'

%c signature value  (ANSI)          integer family
%d decimal value    (0...9)         integer family
%x heximal value    (0...9,a...f)   integer family
%X heximal value    (0...9,A...F)   integer family
%b boolean value    (true,false)    BOOL
%B boolean value    (TRUE,FALSE)    BOOL
%s buffered string  (unicode)       KSTR
%w ansi string      (ANSI+len)      BYTE* + DWORD
%w unicode string   (UNICODE+len)   WORD* + DWORD
*/
const char _stateTable[NUM_CHARTYPES * NUM_STATES] = {
    //ch_OTHER ch_PERCENT  ch_FLAG    ch_DOT     ch_ZERO    ch_DIGIT   ch_TYPE
    //   0        1           2          3          4          5          6   
    ST_NORMAL, ST_PERCENT, ST_NORMAL, ST_NORMAL, ST_NORMAL, ST_NORMAL, ST_NORMAL,  // 0 st_NORMAL 
    ST_REVERT, ST_NORMAL,  ST_FLAG,   ST_DOT,    ST_FLAG,   ST_WIDTH,  ST_TYPE,    // 1 st_PERCENT 
    ST_REVERT, ST_REVERT,  ST_FLAG,   ST_DOT,    ST_FLAG,   ST_WIDTH,  ST_TYPE,    // 2 st_FLAG 
    ST_REVERT, ST_REVERT,  ST_REVERT, ST_DOT,    ST_WIDTH,  ST_WIDTH,  ST_TYPE,    // 3 st_WIDTH 
    ST_REVERT, ST_REVERT,  ST_REVERT, ST_REVERT, ST_PRECIS, ST_PRECIS, ST_REVERT,  // 4 st_DOT 
    ST_REVERT, ST_REVERT,  ST_REVERT, ST_REVERT, ST_PRECIS, ST_PRECIS, ST_TYPE,    // 5 st_PRECIS 
    ST_REVERT, ST_REVERT,  ST_REVERT, ST_REVERT, ST_REVERT, ST_REVERT, ST_NORMAL   // 4 st_TYPE 
};
#define getCharType(ch)         \
    ( (ch) < L' ' || (ch) > L'x' ? CH_OTHER : (CHARTYPE) _charTable[(ch) - L' '] )
#define getState(charType, currentState)         \
    ( (STATE) _stateTable[((currentState) * NUM_CHARTYPES) + (charType)] )


DWORD printInt(WCHAR* pwzBuf, DWORD bufSize, LLONG number, DWORD width, DWORD radix, 
        BOOL bSigned, BOOL bUpCase, BOOL bForceSign, BOOL bLeftAlign, BOOL bPadZeroes)
{
    QWORD nNumber;
    DWORD nDigit;
    DWORD nRet;
    WCHAR wchA;
    WCHAR wchSign;
    WCHAR wchPad;
    WCHAR* pwzBase;
    DWORD padSize;
    DWORD valueSize;
    DWORD valueRemain;
    INT bufRemain;

    if((radix < 2) || (radix > 36))    // 0...9,A...Z
    {
        return 0;
    }

    bufRemain = bufSize - 1;    // reserve space for trailing zero
    pwzBase = pwzBuf;

    // adjust flags etc
    if(width == 0)
    {
        width = 1;
    }
    if(radix != 10)     // set all non-decimal to non-signed
    {
        bSigned = FALSE;
    }
    wchA = SMALLACHAR;
    if(bUpCase != FALSE)
    {
        wchA = CAPACHAR;
    }
    wchPad = SPACECHAR;
    if(bPadZeroes != FALSE)
    {
        wchPad = NUMZEROCHAR;
    }
    nNumber = number;
    valueSize = 0;
    if(nNumber == 0)
    {
        valueSize++;
    }
    else
    {
        while(nNumber != 0)         // get # of digits
        {
            nNumber /= radix;
            valueSize++;
        }
    }
    wchSign = 0;    // none
    if(bSigned != FALSE)
    {
        if(number < 0)
        {
            wchSign = MINUSCHAR;
            valueSize++;
        }
        if((number > 0) & (bForceSign != FALSE))
        {
            wchSign = PLUSCHAR;
            valueSize++;
        }
    }
    // set left/right padding size (positive only)
    padSize = (width - valueSize) * (width > valueSize);

    // print right-align padding
    if(bLeftAlign == FALSE)
    {
        while((padSize != 0) & (bufRemain > 0))
        {
            *pwzBuf = wchPad;
            pwzBuf++;
            padSize--;
            bufRemain--;
        }
    }

    // print number
    pwzBuf += valueSize;
    bufRemain -= valueSize;
    valueRemain = valueSize;
    nNumber = number;
    //while(nNumber != 0)
    while(valueRemain != 0)
    {
        pwzBuf--;
        bufRemain++;
        valueRemain--;
        nDigit = (DWORD)nNumber % radix;
        nNumber /= radix;
        // print only in buffer bounds
        if(bufRemain > 0)
        {
            if(nDigit < 10)
                *pwzBuf = (WCHAR)(NUMZEROCHAR + nDigit);
            else
                *pwzBuf = (WCHAR)(wchA + (nDigit - 10));
        }
    }
    pwzBuf += valueSize;
    bufRemain -= valueSize;

    // print left-align padding
    if(bLeftAlign != FALSE)
    {
        while((padSize != 0) & (bufRemain > 0))
        {
            *pwzBuf = wchPad;
            pwzBuf++;
            padSize--;
            bufRemain--;
        }
    }

    // place trailing zero
    if(bufRemain > 0)
    {
        *pwzBuf = ZEROCHAR;
    }

    // return number of characters written
    nRet = pwzBuf - pwzBase;
    return nRet;
}

void INLINE copyAtoW(WORD** ppwzDst, BYTE** ppbySrc, DWORD* pdwDstRemain, DWORD* pdwSrcRemain)
{
    WCHAR wchar;
    while((*pdwDstRemain != 0) & (*pdwSrcRemain != 0))
    {
        wchar = (WCHAR) *(*ppbySrc);
        (*ppbySrc)++;
        *(*ppwzDst) = wchar;
        (*ppwzDst)++;
        (*pdwSrcRemain)--;
        (*pdwDstRemain)--;
    }
}

void INLINE copyWtoW(WORD** ppwzDst, WORD** ppwzSrc, DWORD* pdwDstRemain, DWORD* pdwSrcRemain)
{
    WCHAR wchar;
    while((*pdwDstRemain != 0) & (*pdwSrcRemain != 0))
    {
        wchar = *(*ppwzSrc);
        (*ppwzSrc)++;
        *(*ppwzDst) = wchar;
        (*ppwzDst)++;
        (*pdwSrcRemain)--;
        (*pdwDstRemain)--;
    }
}

DWORD KPIV kSprintf(KSTR* pksString, KSTR* pksFormat, ...)
{
    WCHAR wch;          // current character
    CHARTYPE charType;  // current character type
    STATE currState;    // current state
    DWORD strRemain;    // string buffer available length
    DWORD fmtRemain;    // format buffer available length
    WCHAR* pwzStr;      // pointer in string buffer
    WCHAR* pwzFmt;      // pointer in format buffer
    DWORD width;        // selected field width, default = 0;
    DWORD precision;    // selected precision, default = 0
    BOOL bForceSign;    // flag: force positive sign
    BOOL bLeftAlign;    // flag: left align
    BOOL bPadZeroes;    // flag: pad with zero
    BOOL bUpCase;       // flag: use upper case
    BOOL bSigned;       // flag: signed integer
    DWORD radix;        // integer radix
    BOOL bDoPrint;      // print flag
    BOOL bPrintInt;     // print: integer
    BOOL bPrintBool;    // print: boolean
    BOOL bPrintStr;     // print: string
    BOOL bPrintIntStr;  // print: string packed into integer
    BOOL bPrintWildStr; // print: string (ansi/unicode)
    DWORD argType;      // argument type
    QWORD intValue;     // value: integer
    BOOL boolValue;     // value: boolean
    KSTR* pkstrValue;   // value: string (pointer)
    BYTE* pbySrc;       // byte source pointer
    WCHAR* pwzSrc;      // string source pointer
    WCHAR wchar;        // character for copy
    DWORD copyLength;   // copy string length / written integer length
    va_list vap;        // variable arguments list pointer
    WCHAR wszNull[] = L"(null)";
    DWORD lenNull = 6;

    if((pksString == NULL) | (pksFormat == NULL))
    {
        return 0;
    }
    if((pksString->pwzBuffer == NULL) | (pksString->bufSize == 0)
        | (pksFormat->pwzBuffer == NULL) | (pksFormat->bufSize == 0))
    {
        return 0;
    }

    currState = ST_NORMAL;
    strRemain = pksString->bufSize;
    fmtRemain = pksFormat->bufSize;
    pwzStr = pksString->pwzBuffer;
    pwzFmt = pksFormat->pwzBuffer;
    vap = va_start(vap, pksFormat);

    bForceSign = FALSE;
    bLeftAlign = FALSE;
    bPadZeroes = FALSE;
    width = 0;
    precision = 0;

    // main loop
    wch = *pwzFmt;
    while((wch != ZEROCHAR) & (strRemain != 0) & (fmtRemain != 0))
    {
        pwzFmt++;
        fmtRemain--;
        charType = getCharType(wch);
        currState = getState(charType, currState);
        bDoPrint = FALSE;

        switch(currState)
        {
        case ST_NORMAL:
            // print character
            *pwzStr = wch;
            pwzStr++;
            strRemain--;
            break;

        case ST_PERCENT:
            // set default settings
            bForceSign = FALSE;
            bLeftAlign = FALSE;
            bPadZeroes = FALSE;
            width = 0;
            break;

        case ST_FLAG:
            // detect & set flag
            switch(wch)
            {
            case L'+':
                bForceSign = TRUE;
                break;
            case L'-':
                bLeftAlign = TRUE;
                break;
            case L'0':
                bPadZeroes = TRUE;
                break;
            }
            break;

        case ST_WIDTH:
            // add digit to current width
            width = width * 10 + (wch - L'0');
            break;

        case ST_DOT:
            // set default precision
            precision = 0;
            break;

        case ST_PRECIS:
            // add digit to current precision
            precision = precision * 10 + (wch - L'0');
            break;

        case ST_TYPE:
            // detect type & set print flags
            bPrintInt = FALSE;
            bPrintBool = FALSE;
            bPrintStr = FALSE;
            bPrintIntStr = FALSE;
            bPrintWildStr = FALSE;
            switch(wch)
            {
            case L'c':      // decimal integer
                bDoPrint = TRUE;
                bPrintIntStr = TRUE;
                break;

            case L'd':      // decimal integer
                bDoPrint = TRUE;
                bPrintInt = TRUE;
                bUpCase = FALSE;
                radix = 10;
                break;

            case L'x':      // hexadecimal integer (lowercase)
                bDoPrint = TRUE;
                bPrintInt = TRUE;
                bUpCase = FALSE;
                radix = 16;
                break;

            case L'X':      // hexadecimal integer (uppercase)
                bDoPrint = TRUE;
                bPrintInt = TRUE;
                bUpCase = TRUE;
                radix = 16;
                break;

            case L'b':      // boolean (lowercase)
                bDoPrint = TRUE;
                bPrintBool = TRUE;
                bUpCase = FALSE;
                break;

            case L'B':      // boolean (uppercase)
                bDoPrint = TRUE;
                bPrintBool = TRUE;
                bUpCase = TRUE;
                break;

            case L's':      // string
                bDoPrint = TRUE;
                bPrintStr = TRUE;
                break;

            case L'w':      // wild string
                bDoPrint = TRUE;
                bPrintWildStr = TRUE;
                break;
            }
            break;
        }

        // print value
        if(bDoPrint != FALSE)
        {
            // print string packed into integer
            if(bPrintIntStr != FALSE)
            {
                // load value type
                argType = va_arg(vap, DWORD);
                if((argType & ARGTYPE_SIGNMASK) != ARGTYPE_ARG)
                {
                    break;      // incorrect stack layout: break immediately
                }
                if((argType & ARGTYPE_TYPEMASK) != ARGTYPE_INTEGER)
                {
                    break;      // incorrect type: break immediately
                }
                if((argType & INTTYPE_SIGNED) != 0)
                {
                    break;      // signed value: break immediately
                }
                // load value by given value type
                switch(argType)
                {
                case INTTYPE_BYTE:
                    intValue = (QWORD) va_arg(vap, BYTE);
                    break;
                case INTTYPE_WORD:
                    intValue = (QWORD) va_arg(vap, WORD);
                    break;
                case INTTYPE_DWORD:
                    intValue = (QWORD) va_arg(vap, DWORD);
                    break;
                case INTTYPE_QWORD:
                    intValue = va_arg(vap, QWORD);
                    break;
                }
                // print value (from low byte to high byte)
                pbySrc = (BYTE*) &intValue;
                copyLength = argType & INTTYPE_SIZEMASK;
                copyAtoW(&pwzStr, &pbySrc, &strRemain, &copyLength);
            }
            // print integer
            if(bPrintInt != FALSE)
            {
                // load value type
                argType = va_arg(vap, DWORD);
                if((argType & ARGTYPE_SIGNMASK) != ARGTYPE_ARG)
                {
                    break;      // incorrect stack layout: break immediately
                }
                if((argType & ARGTYPE_TYPEMASK) != ARGTYPE_INTEGER)
                {
                    break;      // incorrect type: break immediately
                }
                bSigned = ((argType & INTTYPE_SIGNED) != 0);
                argType &= ~INTTYPE_SIGNED;
                // load value by given value type
                switch(argType)
                {
                case INTTYPE_BYTE:
                    intValue = (QWORD) va_arg(vap, BYTE);
                    break;
                case INTTYPE_WORD:
                    intValue = (QWORD) va_arg(vap, WORD);
                    break;
                case INTTYPE_DWORD:
                    intValue = (QWORD) va_arg(vap, DWORD);
                    break;
                case INTTYPE_QWORD:
                    intValue = va_arg(vap, QWORD);
                    break;
                }
                // print value
                copyLength = printInt(pwzStr, strRemain, intValue, width, radix, 
                    bSigned, bUpCase, bForceSign, bLeftAlign, bPadZeroes);
                pwzStr += copyLength;
                strRemain -= copyLength;
            }
            // print boolean
            if(bPrintBool != FALSE)
            {
                // load value type
                argType = va_arg(vap, DWORD);
                if((argType & ARGTYPE_SIGNMASK) != ARGTYPE_ARG)
                {
                    break;      // incorrect stack layout: break immediately
                }
                if((argType & ARGTYPE_TYPEMASK) != ARGTYPE_BOOLEAN)
                {
                    break;      // incorrect type: break immediately
                }
                // load value by given value type
                boolValue = va_arg(vap, BOOL);
                // print value
                pwzSrc = L"false";
                if(boolValue != FALSE)
                {
                    pwzSrc = L"true";
                }
                while((*pwzSrc != ZEROCHAR) & (strRemain != 0))
                {
                    wchar = *pwzSrc;
                    wchar -= (L'a' - L'A') * (bUpCase != FALSE);
                    *pwzStr = wchar;
                    pwzStr++;
                    strRemain--;
                }
            }
            // print string
            if(bPrintStr != FALSE)
            {
                // load value type
                argType = va_arg(vap, DWORD);
                if((argType & ARGTYPE_SIGNMASK) != ARGTYPE_ARG)
                {
                    break;      // incorrect stack layout: break immediately
                }
                if((argType & ARGTYPE_TYPEMASK) != ARGTYPE_KSTRING)
                {
                    break;      // incorrect type: break immediately
                }
                // load value by given value type
                pkstrValue = va_arg(vap, KSTR*);
                // handle null value / null pointer
                pwzSrc = wszNull;
                copyLength = lenNull;
                if(pkstrValue != NULL)
                {
                    if(pkstrValue->pwzBuffer != NULL)
                    {
                        pwzSrc = pkstrValue->pwzBuffer;
                        copyLength = pkstrValue->strLen;
                    }
                }
                // print value
                if(copyLength != 0)
                {
                    if((precision != 0) & (copyLength > precision))
                    {
                        copyLength = precision;
                    }
                    copyWtoW(&pwzStr, &pwzSrc, &strRemain, &copyLength);
                }
            }
            // print wild string
            if(bPrintWildStr != FALSE)
            {
                // load value type
                argType = va_arg(vap, DWORD);
                if((argType & ARGTYPE_SIGNMASK) != ARGTYPE_ARG)
                {
                    break;      // incorrect stack layout: break immediately
                }
                if((argType & ARGTYPE_TYPEMASK) != ARGTYPE_WSTRING)
                {
                    break;      // incorrect type: break immediately
                }
                if(argType == GENTYPE_ANSI)
                {
                    // load value by given value type
                    pbySrc = va_arg(vap, BYTE*);
                    copyLength = va_arg(vap, DWORD);
                    // handle null value / null pointer
                    if(pbySrc == NULL)
                    {
                        pwzSrc = wszNull;
                        copyLength = lenNull;
                        copyWtoW(&pwzStr, &pwzSrc, &strRemain, &copyLength);
                    }
                    else    // print value
                    {
                        copyAtoW(&pwzStr, &pbySrc, &strRemain, &copyLength);
                    }
                }
                else        //GENTYPE_WIDE
                {
                    // load value by given value type
                    pwzSrc = va_arg(vap, WCHAR*);
                    copyLength = va_arg(vap, DWORD);
                    // handle null value / null pointer
                    if(pbySrc == NULL)
                    {
                        pwzSrc = wszNull;
                        copyLength = lenNull;
                    }
                    // print value
                    copyWtoW(&pwzStr, &pwzSrc, &strRemain, &copyLength);
                }
            }
            // endof print value
        }

        wch = *pwzFmt;
    }

    pwzStr -= 1 * (strRemain == 0);     // if string buffer is out, step back
    *pwzStr = ZEROCHAR;                 // force trailing zero
    // don't step forward for correct return-length calculation

    pksString->strLen = (WORD)(pwzStr - pksString->pwzBuffer);
    return pksString->strLen;
}


