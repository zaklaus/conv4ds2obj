@echo off

set LIBS="kernel32.lib user32.lib"

msvc.bat conv4ds2obj.c D %LIBS%
