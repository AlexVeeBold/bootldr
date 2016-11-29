ECHO Building %~1...

nasm.exe -f bin "%~1" -l "%~n1.lst" -o "%~2"
IF NOT EXIST "%~2" GOTO fail

:done
ECHO Done.
GOTO end

:fail
ECHO Warning: %~2 not found, must be some build error
GOTO failed

:failed
PAUSE

:end
