echo off
setlocal

echo.
echo === Compiling LuaScript Static Library... ===
echo.

call "%ProgramFiles%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
if "%DevEnvDir%"=="" goto :NOT_IN_MSDEVCMD

set _GW_MSDEV="%ProgramFiles%\Microsoft Visual Studio 9.0\VC\vcpackages\vcbuild.exe"

set _GW_SOLUTION="LuaScript_VC90.sln"
set _GW_CONFIG= "Debug|Win32";"Release|Win32";

for /D %%c in ( %_GW_CONFIG% ) do (
    %_GW_MSDEV% /rebuild %_GW_SOLUTION% %%c 

    if ERRORLEVEL 3 goto :ERR_COMPILE
    if ERRORLEVEL 2 goto :ERR_COMPILE
    if ERRORLEVEL 1 goto :ERR_COMPILE
)

echo.
echo *******************************************
echo *** �������� ���������� �Ϸ�Ǿ����ϴ�. ***
echo *******************************************

goto :end


:ERR_COMPILE

echo.
echo -------------------------------------
echo --- �������߿� ������ ������ϴ�. ---
echo -------------------------------------
goto :end

:NOT_IN_MSDEVCMD
echo Visual Studio .NET 2003 ��� ������Ʈ���� �����ؾ� �մϴ�.

:end
endlocal
pause
