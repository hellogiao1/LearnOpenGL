@echo off
echo Building LearnOpenGL project...
echo.

REM 尝试使用MSBuild编译
echo Attempting to build with MSBuild...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\LearnOpenGL.sln /p:Configuration=Debug /p:Platform=x64 /verbosity:minimal

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful!
    echo.
    echo Running the program...
    cd ..\x64\Debug
    LearnOpenGL.exe
) else (
    echo.
    echo Build failed. Please check the errors above.
    echo.
    echo You can also try building in Visual Studio:
    echo 1. Open LearnOpenGL.sln in Visual Studio
    echo 2. Press F5 to build and run
    echo 3. Or press Ctrl+Shift+B to build only
)

pause 