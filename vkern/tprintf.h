///////////////////////////////////////////////////////////////
//
// tprintf: Formatted print to string
//
//
//   15.09.2010 02:03 - created
//

#ifndef _TPRINTF_H_INC_
#define _TPRINTF_H_INC_


EXTERN_C_BEGIN


//[INTERNAL]
#define INTTYPE_BYTE    0x4B000001
#define INTTYPE_WORD    0x4B000002
#define INTTYPE_DWORD   0x4B000004
#define INTTYPE_QWORD   0x4B000008
#define INTTYPE_CHAR    0x4B008001  // signed BYTE
#define INTTYPE_SHORT   0x4B008002  // signed WORD
#define INTTYPE_INT     0x4B008004  // signed DWORD
#define INTTYPE_LLONG   0x4B008008  // signed QWORD
#define GENTYPE_BOOL    0x4B010000
#define GENTYPE_KSTR    0x4B020000
#define GENTYPE_ANSI    0x4B030001
#define GENTYPE_WIDE    0x4B030002

// kSprintf (variable) arguments
#define ABYTE(val)          (DWORD)INTTYPE_BYTE, (BYTE)val
#define AWORD(val)          (DWORD)INTTYPE_WORD, (WORD)val
#define ADWORD(val)         (DWORD)INTTYPE_DWORD, (DWORD)val
#define AQWORD(val)         (DWORD)INTTYPE_QWORD, (QWORD)val
#define ASCHAR(val)         (DWORD)INTTYPE_CHAR, (CHAR)val
#define ASHORT(val)         (DWORD)INTTYPE_SHORT, (SHORT)val
#define AINT(val)           (DWORD)INTTYPE_INT, (INT)val
#define ALLONG(val)         (DWORD)INTTYPE_LLONG, (LLONG)val
#define ABOOL(val)          (DWORD)GENTYPE_BOOL, (BOOL)val
#define AKSTR(val)          (DWORD)GENTYPE_KSTR, (KSTR*)val
#define AANSI(str,len)      (DWORD)GENTYPE_ANSI, (ACHAR*)str, (DWORD)len
#define AWIDE(str,len)      (DWORD)GENTYPE_WIDE, (WCHAR*)str, (DWORD)len

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
DWORD KPIV kSprintf(KSTR* pksString, KSTR* pksFormat, ...);
// usage: kSprintf(&ksBuf, &ksFmt, AWORD(wSize), ABOOL(bUsed));


EXTERN_C_END


#endif //_TPRINTF_H_INC_
