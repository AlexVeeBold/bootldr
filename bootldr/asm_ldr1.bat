@ECHO off

IF NOT "%ASMDIR%" == "" GOTO asm
CALL ..\_setenv

:asm
CALL _assemble %LDR1ASM% %LDR1BIN%
