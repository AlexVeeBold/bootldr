@ECHO off

IF NOT "%ASMDIR%" == "" GOTO asm
CALL ..\_setenv

:asm
CALL _assemble %LDR2ASM% %LDR2BIN%
