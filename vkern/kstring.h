///////////////////////////////////////////////////////////////
//
// kstring: kernel string (unicode)
//
//
//   15.09.2010 02:12 - created
//

#ifndef _KSTRING_H_INC_
#define _KSTRING_H_INC_


EXTERN_C_BEGIN


// unicode string
struct KSTR {
    WORD strLen;        // used characters count (without traling zero)
    WORD bufSize;       // total characters count (with trailing zero)
    WCHAR* pwzBuffer;   // pointer to data buffer
};


//[INTERNAL] kstring (variable): "worker"
#define _KS_CREATEBUF_WORKER(ksVarName,bufferSize,srcLine)      \
    WCHAR __ksbuf##srcLine[(bufferSize)];   \
    KSTR ksVarName;                         \
    ksVarName.strLen = 0;                   \
    ksVarName.bufSize = (bufferSize);       \
    ksVarName.pwzBuffer = __ksbuf##srcLine
//[INTERNAL] kstring (variable): __LINE__ extractor
#define _KS_CREATEBUF_TOKENIZER(ksVarName,bufferSize,srcLine)   \
    _KS_CREATEBUF_WORKER(ksVarName,bufferSize,srcLine)
//[INTERNAL] kstring (constant): utility code
#define _KS_STRSIZE(wszData)                \
    ((sizeof(wszData) / sizeof(WCHAR)) - 1)
#define _KS_BUFSIZE(wszData)                \
    (sizeof(wszData) / sizeof(WCHAR))


// declare kstring (variable)
// usage: KSTR_VAR(ksSomeBuffer, 512);  <creates local 'hidden' buffer>
#define KSTR_VAR(ksVarName,bufferSize)      \
    _KS_CREATEBUF_TOKENIZER(ksVarName,bufferSize,__LINE__)

// declare kstring (constant)
// usage: KSTR_CONST(ksMessage, "Hello, World!");       (without L'')
#define KSTR_CONST(ksVarName,wszData)               \
    KSTR ksVarName;                                 \
    ksVarName.strLen = _KS_STRSIZE(L##wszData);     \
    ksVarName.bufSize = _KS_BUFSIZE(L##wszData);    \
    ksVarName.pwzBuffer = L##wszData

// set kstring to constant (can be used multiple times with one kstring)
// usage: KSTR_SET(ksMsg, "Hello, World!");       (without L'')
#define KSTR_SET(kstring,wszData)                   \
    kstring.strLen = _KS_STRSIZE(T(wszData));       \
    kstring.bufSize = _KS_BUFSIZE(T(wszData));      \
    kstring.pwzBuffer = T(wszData)

// initialize kstring with constant (array initialization)
// usage: KSTR ksVar = {KSTR_INIT("Hello, World!")};    (without L'')
#define KSTR_INIT(wszData)                          \
    _KS_STRSIZE(L##wszData), _KS_BUFSIZE(L##wszData), L##wszData


EXTERN_C_END


#endif //_KSTRING_H_INC_
