///////////////////////////////////////////////////////////////
//
// ktypes: kernel types
//
//
//   19.09.2010 08:21 - created (moved out from kdef)
//

#ifndef _KTYPES_H_INC_
#define _KTYPES_H_INC_


#include "kdef.h"


union WBYTE {   // little-endian word
    struct {
        BYTE byte0;     // bits: 00..07
        BYTE byte1;     // bits: 08..15
    };
    BYTE byte[2];   // array of 2 BYTEs
    WORD word;      // bits: 00..15
};

union DWBYTE {  // little-endian dword
    struct {
        BYTE byte0;     // bits: 00..07
        BYTE byte1;     // bits: 08..15
        BYTE byte2;     // bits: 16..23
        BYTE byte3;     // bits: 24..31
    };
    struct {
        WORD word0;     // bits: 00..15
        WORD word1;     // bits: 16..31
    };
    BYTE byte[4];   // array of 4 BYTEs
    WORD word[2];   // array of 2 WORDs
    DWORD dword;    // bits: 00..31
};

union QWBYTE {  // little-endian quad-word
    struct {
        BYTE byte0;     // bits: 00..07
        BYTE byte1;     // bits: 08..15
        BYTE byte2;     // bits: 16..23
        BYTE byte3;     // bits: 24..31
        BYTE byte4;     // bits: 32..39
        BYTE byte5;     // bits: 40..47
        BYTE byte6;     // bits: 48..55
        BYTE byte7;     // bits: 56..63
    };
    struct {
        WORD word0;     // bits: 00..15
        WORD word1;     // bits: 16..31
        WORD word2;     // bits: 32..47
        WORD word3;     // bits: 48..63
    };
    struct {
        DWORD dword0;   // bits: 00..31
        DWORD dword1;   // bits: 32..63
    };
    BYTE byte[8];   // array of 8 BYTEs
    WORD word[4];   // array of 4 WORDs
    DWORD dword[2]; // array of 2 DWORDs
    QWORD qword;    // bits: 00..63
};



#endif //_KTYPES_H_INC_
