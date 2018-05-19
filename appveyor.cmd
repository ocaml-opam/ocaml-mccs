@rem Do not call setlocal!
@echo off

set Path=%CYG_ROOT%\bin;%Path%
set OCAML_PREV_PATH=%PATH%
set OCAML_PREV_LIB=%LIB%
set OCAML_PREV_INCLUDE=%INCLUDE%

rem CYGWIN_PACKAGES is the list of required Cygwin packages (cygwin is included
rem in the list just so that the Cygwin version is always displayed on the log).
rem CYGWIN_COMMANDS is a corresponding command to run with --version to test
rem whether the package works. This is used to verify whether the installation
rem needs upgrading.
set CYGWIN_PACKAGES=cygwin make diffutils unzip patch
set CYGWIN_COMMANDS=cygcheck make diff unzip patch

if "%PORT%" equ "mingw" (
  set CYGWIN_PACKAGES=%CYGWIN_PACKAGES% mingw64-i686-gcc-core
  set CYGWIN_COMMANDS=%CYGWIN_COMMANDS% i686-w64-mingw32-gcc
)
if "%PORT%" equ "mingw64" (
  set CYGWIN_PACKAGES=%CYGWIN_PACKAGES% mingw64-x86_64-gcc-core
  set CYGWIN_COMMANDS=%CYGWIN_COMMANDS% x86_64-w64-mingw32-gcc
)

set CYGWIN_INSTALL_PACKAGES=
set CYGWIN_UPGRADE_REQUIRED=0

for %%P in (%CYGWIN_PACKAGES%) do call :CheckPackage %%P
call :UpgradeCygwin

call :SaveVars

if "%PORT%" equ "msvc64" call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
if "%PORT%" equ "msvc" call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"

bash -lc "$APPVEYOR_BUILD_FOLDER/appveyor.sh"
if errorlevel 1 exit /b 1

set PATH=C:\OCaml\%OCAML_VERSION%\%PORT%\bin;%PATH%

if "%PORT%" equ "mingw" set PATH=%CYG_ROOT%\usr\i686-w64-mingw32\sys-root\mingw\bin;%PATH%
if "%PORT%" equ "mingw64" set PATH=%CYG_ROOT%\usr\x86_64-w64-mingw32\sys-root\mingw\bin;%PATH%

set MCCS_LPSOLVER=%APPVEYOR_BUILD_FOLDER%\_build\default\test\cbclp.exe

goto :EOF

:SaveVars
set OCAML_PREV_PATH=%PATH%
set OCAML_PREV_LIB=%LIB%
set OCAML_PREV_INCLUDE=%INCLUDE%
goto :EOF

:RestoreVars
set PATH=%OCAML_PREV_PATH%
set LIB=%OCAML_PREV_LIB%
set INCLUDE=%OCAML_PREV_INCLUDE%
goto :EOF

:CheckPackage
"%CYG_ROOT%\bin\bash.exe" -lc "cygcheck -dc %1" | findstr %1 > nul
if %ERRORLEVEL% equ 1 (
  echo Cygwin package %1 will be installed
  set CYGWIN_INSTALL_PACKAGES=%CYGWIN_INSTALL_PACKAGES%,%1
)
goto :EOF

:UpgradeCygwin
if "%CYGWIN_INSTALL_PACKAGES%" neq "" "%CYG_ROOT%\setup-x86_64.exe" --quiet-mode --no-shortcuts --no-startmenu --no-desktop --only-site --root "%CYG_ROOT%" --site "%CYG_MIRROR%" --local-package-dir "%CYG_CACHE%" --packages %CYGWIN_INSTALL_PACKAGES:~1% > nul
for %%P in (%CYGWIN_COMMANDS%) do "%CYG_ROOT%\bin\%%P.exe" --version > nul 2>&1 || "%CYG_ROOT%\bin\%%P.exe" -v > nul 2>&1 || set CYGWIN_UPGRADE_REQUIRED=1
"%CYG_ROOT%\bin\bash.exe" -lc "cygcheck -dc %CYGWIN_PACKAGES%"
if %CYGWIN_UPGRADE_REQUIRED% equ 1 (
  echo Cygwin package upgrade required - please go and drink coffee
  "%CYG_ROOT%\setup-x86_64.exe" --quiet-mode --no-shortcuts --no-startmenu --no-desktop --only-site --root "%CYG_ROOT%" --site "%CYG_MIRROR%" --local-package-dir "%CYG_CACHE%" --upgrade-also > nul
  "%CYG_ROOT%\bin\bash.exe" -lc "cygcheck -dc %CYGWIN_PACKAGES%"
)
goto :EOF

