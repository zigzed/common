@echo off

if defined ProgramFiles(x86) goto WIN64
CALL "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
goto continue1
:WIN64
CALL "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
:continue1

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

mkdir build-%1
cd build-%1
echo ����vs����
cmake -G "Visual Studio 9 2008 Win64" .. -DWINPLAT=_WIN64
cd ..
if errorlevel 1 goto error  

rem ����VC,���б���
echo ��ʼ���� Mode = %BUILD_MODE%
cd build-%1
VCbuild ".\cxxlib.sln"  "%1|x64"
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

