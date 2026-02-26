@echo off
echo Setting up VS2022 environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if errorlevel 1 (
    echo Error: Failed to set up VS2022 environment
    pause
    exit /b 1
)

echo Compiling with VS2022 toolset...
cl CoreMain.cpp SharedMemoryManager.cpp CoreMainTest.cpp /EHsc /std:c++20 /utf-8 /link /OUT:CoreMainTest.exe

if errorlevel 1 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo Compilation successful!
echo Running test program...
CoreMainTest.exe

pause