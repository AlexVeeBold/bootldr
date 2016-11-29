@ECHO off

CLS

CALL _setenv

CD %ASMDIR%
CALL asm_ldr1
CD ..
IF NOT EXIST %LDR1_IMAGE% GOTO fail

ECHO Updating VM settings...

%VBM% -q storageattach %VMNAME% --storagectl %VMSTGCTL% --port 0 --device 0 --type hdd --medium none
%VBM% -q closemedium disk %VMHDIMAGE%
MOVE %VMHDIMAGE% %VMHDBACKUP%
%VBM% -q convertfromraw %LDR1_IMAGE% %VMHDIMAGE% --format VDI --variant Fixed
%VBM% -q storageattach %VMNAME% --storagectl %VMSTGCTL% --port 0 --device 0 --type hdd --medium %VMHDIMAGE%

:done
ECHO Done.
GOTO end

:fail
GOTO failed

:failed
@rem PAUSE

:end
