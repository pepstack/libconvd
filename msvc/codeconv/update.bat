@ECHO OFF
set LIBPROJECT=libconvd
set APPPROJECT=codeconv

setlocal enabledelayedexpansion
for /f %%a in (%~dp0..\..\src\apps\codeconv\VERSION) do (
    echo VERSION: %%a
    set APPVER=%%a
    goto :APPVER
)
:APPVER
echo update for: %APPPROJECT%-%APPVER%

set DISTAPPDIR="%~dp0..\..\dist\%APPPROJECT%-%APPVER%"

set x64AppDistDbgDir="%DISTAPPDIR%\win64-debug\bin"
set x64AppDistRelsDir="%DISTAPPDIR%\win64-release\bin"
set x86AppDistDbgDir="%DISTAPPDIR%\win86-debug\bin"
set x86AppDistRelsDir="%DISTAPPDIR%\win86-release\bin"


if "%1" == "x64_debug" (
    copy "%~dp0..\..\deps\libiconv\bin\win64\libcharset-1.dll" "%~dp0target\x64\Debug\"
    copy "%~dp0..\..\deps\libiconv\bin\win64\libiconv-2.dll" "%~dp0target\x64\Debug\"
    copy "%~dp0..\..\deps\libiconv\bin\win64\iconv.exe" "%~dp0target\x64\Debug\"

    copy "%~dp0target\x64\Debug\%APPPROJECT%.exe" "%x64AppDistDbgDir%\"
	copy "%~dp0target\x64\Debug\libcharset-1.dll" "%x64AppDistDbgDir%\"
	copy "%~dp0target\x64\Debug\libiconv-2.dll" "%x64AppDistDbgDir%\"
	copy "%~dp0target\x64\Debug\iconv.exe" "%x64AppDistDbgDir%\"
)


if "%1" == "x64_release" (
    copy "%~dp0..\..\deps\libiconv\bin\win64\libcharset-1.dll" "%~dp0target\x64\Release\"
    copy "%~dp0..\..\deps\libiconv\bin\win64\libiconv-2.dll" "%~dp0target\x64\Release\"
    copy "%~dp0..\..\deps\libiconv\bin\win64\iconv.exe" "%~dp0target\x64\Release\"

    copy "%~dp0target\x64\Release\%APPPROJECT%.exe" "%x64AppDistRelsDir%\"
	copy "%~dp0target\x64\Release\libcharset-1.dll" "%x64AppDistRelsDir%\"
	copy "%~dp0target\x64\Release\libiconv-2.dll" "%x64AppDistRelsDir%\"
	copy "%~dp0target\x64\Release\iconv.exe" "%x64AppDistRelsDir%\"
)


if "%1" == "x86_debug" (
    copy "%~dp0..\..\deps\libiconv\bin\win86\libcharset-1.dll" "%~dp0target\Win32\Debug\"
    copy "%~dp0..\..\deps\libiconv\bin\win86\libiconv-2.dll" "%~dp0target\Win32\Debug\"
    copy "%~dp0..\..\deps\libiconv\bin\win86\libintl-8.dll" "%~dp0target\Win32\Debug\"
    copy "%~dp0..\..\deps\libiconv\bin\win86\iconv.exe" "%~dp0target\Win32\Debug\"

    copy "%~dp0target\Win32\Debug\%APPPROJECT%.exe" "%x86AppDistDbgDir%\"
	copy "%~dp0target\Win32\Debug\libcharset-1.dll" "%x86AppDistDbgDir%\"
	copy "%~dp0target\Win32\Debug\libiconv-2.dll" "%x86AppDistDbgDir%\"
	copy "%~dp0target\Win32\Debug\libintl-8.dll" "%x86AppDistDbgDir%\"
	copy "%~dp0target\Win32\Debug\iconv.exe" "%x86AppDistDbgDir%\"
)


if "%1" == "x86_release" (
    copy "%~dp0..\..\deps\libiconv\bin\win86\libcharset-1.dll" "%~dp0target\Win32\Release\"
    copy "%~dp0..\..\deps\libiconv\bin\win86\libiconv-2.dll" "%~dp0target\Win32\Release\"
    copy "%~dp0..\..\deps\libiconv\bin\win86\libintl-8.dll" "%~dp0target\Win32\Release\"
    copy "%~dp0..\..\deps\libiconv\bin\win86\iconv.exe" "%~dp0target\Win32\Release\"

    copy "%~dp0target\Win32\Release\%APPPROJECT%.exe" "%x86AppDistRelsDir%\"
	copy "%~dp0target\Win32\Release\libcharset-1.dll" "%x86AppDistRelsDir%\"
	copy "%~dp0target\Win32\Release\libiconv-2.dll" "%x86AppDistRelsDir%\"
	copy "%~dp0target\Win32\Release\libintl-8.dll" "%x86AppDistRelsDir%\"
	copy "%~dp0target\Win32\Release\iconv.exe" "%x86AppDistRelsDir%\"
)