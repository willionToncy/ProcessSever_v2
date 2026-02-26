@echo off
echo 正在编译共享内存测试程序...

REM 设置Visual Studio环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if errorlevel 1 (
    echo 错误: 无法设置Visual Studio环境
    pause
    exit /b 1
)

echo 正在编译...
cl SharedMemoryManager.cpp Sever_v2.cpp /EHsc /std:c++20 /link /OUT:ShareMonyTest.exe

if errorlevel 1 (
    echo 编译失败!
    pause
    exit /b 1
)

echo 编译成功!
echo 运行测试程序...
ShareMonyTest.exe

pause