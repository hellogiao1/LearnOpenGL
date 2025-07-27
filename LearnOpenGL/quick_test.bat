@echo off
echo Quick compilation test...
echo.

REM 尝试编译
echo Building project...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\LearnOpenGL.sln /p:Configuration=Debug /p:Platform=x64 /verbosity:minimal /nologo

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ Build successful!
    echo.
    echo 🎉 Ready to run!
    echo.
    echo To run the program:
    echo cd ..\x64\Debug
    echo LearnOpenGL.exe
    echo.
    echo Or in Visual Studio: Press F5
) else (
    echo.
    echo ❌ Build failed!
    echo.
    echo Please check the errors above.
)

echo.
echo Press any key to continue...
pause > nul 