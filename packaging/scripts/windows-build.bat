SET DEP_ROOT=%1

REM curl https://download.microsoft.com/download/3/2/2/3224B87F-CFA0-4E70-BDA3-3DE650EFEBA5/vcredist_x64.exe > ..\vcredist_x64.exe
REM curl ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-9-1-release.zip > ..\pthread.zip
REM curl -L https://downloads.sourceforge.net/project/glew/glew/2.1.0/glew-2.1.0-win32.zip > ..\glew.zip

rmdir /S build
mkdir build
cd build
cmake -G "Visual Studio 15 Win64"^
 -DPTHREADS_INCLUDE_DIR=%DEP_ROOT%\pthreads\Pre-built.2\include^
 -DpthreadVC2_LIBRARY=%DEP_ROOT%\pthreads\Pre-built.2\lib\x64\pthreadVC2.lib^
 -DBUILD_WITH_QT=ON -DQT_ROOT=C:\Qt\5.9.2\msvc2017_64^
 -DBUILD_WITH_OPENCV=ON -DOPENCV_ROOT=C:\tools\opencv\build -DOpenCV_RUNTIME=vc14^
 -DGLEW_ROOT=%DEP_ROOT%\glew^
 -DBUILD_DEMOS=ON -DBUILD_EXAMPLES=ON^
 -DBUILD_APPS=ON -DBUILD_REDIST=WIX ..
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build . --target package --config Release
