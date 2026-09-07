@echo off
setlocal

for /f "usebackq delims=" %%i in (`
	"%programfiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" ^
	-latest ^
	-property installationPath
`) do set VSBUILDROOT=%%i

if not defined VSBUILDROOT (
	echo Visual Studio not found.
	exit /b 1
)

call "%VSBUILDROOT%\VC\Auxiliary\Build\vcvars64.bat"

set APP="flac-lame-frontend"
msbuild %APP%.slnx /p:Configuration=Release /p:Platform=x64
move /Y x64_Release\%APP%.exe .\
rmdir /S /Q x64_Release
pause
