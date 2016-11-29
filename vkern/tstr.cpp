///////////////////////////////////////////////////////////////
//
// String functions (use with Tiny Unicode helper)
//
//
//   17.06.2009 10:56 - created
//   26.06.2009 03:30 - double-compile version (ansi/unicode)
//                    all code moved (and guarded) to header
//   26.08.2010 06:07 - copied into vkern project
//

#include "kdef.h"


#define _TSTR_CLPASS_

// compile unicode functions
#define UNICODE
#include "tunic.h"
#include "tstrp.h"

// compile ansi functions
#undef UNICODE
#include "tunic.h"
#include "tstrp.h"

