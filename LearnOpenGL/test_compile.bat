@echo off
echo Testing compilation...
echo.

REM 检查是否有编译错误
echo Checking for compilation errors...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\LearnOpenGL.sln /p:Configuration=Debug /p:Platform=x64 /verbosity:minimal /nologo

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ Compilation successful!
    echo.
    echo 🎉 The SDF font rendering program is ready to run!
    echo.
    echo To run the program:
    echo 1. Open Visual Studio
    echo 2. Open LearnOpenGL.sln
    echo 3. Press F5 to build and run
    echo.
    echo Or run directly:
    echo cd x64\Debug
    echo LearnOpenGL.exe
) else (
    echo.
    echo ❌ Compilation failed!
    echo.
    echo Please check the errors above and try again.
)

pause 