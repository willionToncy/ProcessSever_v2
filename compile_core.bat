@echo off
echo 正在编译 CoreMain 测试程序...

REM 设置Visual Studio环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if errorlevel 1 (
    echo 错误: 无法设置Visual Studio环境
    pause
    exit /b 1
)

echo 正在编译...
cl CoreMain.cpp SharedMemoryManager.cpp CoreMainTest.cpp /EHsc /std:c++20 /link /OUT:CoreMainTest.exe

if errorlevel 1 (
    echo 编译失败!
    pause
    exit /b 1
)

echo 编译成功!
echo 运行测试程序...
CoreMainTest.exe

pause