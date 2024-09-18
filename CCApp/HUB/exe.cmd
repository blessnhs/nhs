==============================================

@echo off
:asd
echo %DATE% %TIME% & netstat -ano | findstr "20001" | find /C "20001"
choice /t 2 /d Y > nul
goto asd

==============================================