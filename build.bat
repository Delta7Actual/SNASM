@echo off
set CC=gcc
set CFLAGS=-Wall -Wextra -pedantic -Iinclude
set OBJDIR=build
set EXEC=SNASM.exe

echo Creating output directory...
if not exist %OBJDIR% mkdir %OBJDIR%

echo Compiling sources...
for %%f in (src\*.c) do (
    %CC% %CFLAGS% -c %%f -o %OBJDIR%\%%~nf.o
)

echo Linking...
%CC% %CFLAGS% %OBJDIR%\*.o -o %EXEC%

echo Done. Output: %EXEC%
