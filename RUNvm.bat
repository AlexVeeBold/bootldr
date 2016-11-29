@ECHO off

@rem CLS

CALL _setenv

IF NOT EXIST %LDR1_IMAGE% GOTO fail1
IF NOT EXIST %LDR2_IMAGE% GOTO fail2
IF NOT EXIST %LDR2_MIXED% GOTO failM

IF NOT EXIST %LOGSDIRNAME% MKDIR %LOGSDIRNAME%
CD %LOGSDIRNAME%
START "command shell" %PIPER%
CD ..
%VBM% startvm %VMNAME%

GOTO end

:fail1
ECHO Warning: %LDR1_IMAGE% not found, must be some build error
GOTO failed

:fail2
ECHO Warning: %LDR2_IMAGE% not found, must be some build error
GOTO failed

:failM
ECHO Warning: %LDR2_MIXED% not found, must be some build error
GOTO failed

:failed
PAUSE

:end
