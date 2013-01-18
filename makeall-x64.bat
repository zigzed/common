@echo off

if defined ProgramFiles(x86) goto WIN64
CALL "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
goto continue1
:WIN64
CALL "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
:continue1

rem 判定输入命令
echo %1
if "%1" == "clean" goto clean
if "%1" == "Release" goto next
if "%1" == "RelWithDebInfo" goto next
if "%1" == "MinSizeRel" goto next
if "%1" == "Debug" goto next
rem 参数错误
goto badparam
:next

mkdir build-%1
cd build-%1
echo 生成vs工程
cmake -G "Visual Studio 9 2008 Win64" .. -DWINPLAT=_WIN64
cd ..
if errorlevel 1 goto error  

rem 调用VC,进行编译
echo 开始编译 Mode = %BUILD_MODE%
cd build-%1
VCbuild ".\cxxlib.sln"  "%1|x64"
cd ..
goto end

:clean
echo 删除build目录
del /Q /S /F  build
rd /S /Q build
goto end

:badparam
@echo 参数错误，必须是 clean/Debug/Release/RelWithDebInfo/MinSizeRel
goto end

:error
@echo 发生错误

:end
pause

