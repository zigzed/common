@echo off
CALL "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
rem �ж���������
echo %1
if "%1" == "clean" goto clean
if "%1" == "Release" goto next
if "%1" == "RelWithDebInfo" goto next
if "%1" == "MinSizeRel" goto next
if "%1" == "Debug" goto next
rem ��������
goto badparam
:next

mkdir build
cd build
echo ����vs����
cmake -G "Visual Studio 9 2008" .. -DWINPLAT=WIN32
cd ..
if errorlevel 1 goto error  

rem ����VC,���б���
echo ��ʼ���� Mode = %BUILD_MODE%
cd build
VCbuild ".\cxxlib.sln"  "%1|Win32"
cd ..
goto end

:clean
echo ɾ��buildĿ¼
del /Q /S /F  build
rd /S /Q build
goto end

:badparam
@echo �������󣬱����� clean/Debug/Release/RelWithDebInfo/MinSizeRel
goto end

:error
@echo ��������

:end
pause

