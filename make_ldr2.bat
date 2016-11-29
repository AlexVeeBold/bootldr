@ECHO off

CLS

CALL _setenv

CD %ASMDIR%
CALL asm_ldr2
CD ..
IF NOT EXIST %LDR2_IMAGE% GOTO fail
IF NOT EXIST %LDR2_HLLMODULE% GOTO fail2


ECHO Building image...

COPY /B %LDR2_IMAGE% + %LDR2_HLLMODULE%  %LDR2_MIXED%

:done
ECHO Done.
GOTO end

:fail
GOTO failed

:fail2
ECHO Warning: %LDR2_HLLMODULE% not found
GOTO failed

:failed
@rem PAUSE

:end
