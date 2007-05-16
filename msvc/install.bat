@ECHO OFF
::IF NOT exist %1 GOTO NOPATH
::IF exist %1 GOTO INSTALL
IF NOT %1x==x GOTO INSTALL

::NOPATH
ECHO please define installation path
PAUSE
GOTO END

:INSTALL
ECHO create directories
mkdir %1
mkdir %1\include
mkdir %1\lib
mkdir %1\doc
mkdir %1\doc\html

ECHO copy header files
copy ..\ICLBlob\src\*.h %1\include\*.h
copy ..\ICLCC\src\*.h %1\include\*.h
copy ..\ICLCore\src\*.h %1\include\*.h
copy ..\ICLFilter\src\*.h %1\include\*.h
copy ..\ICLIO\src\*.h %1\include\*.h
copy ..\ICLMEx\src\*.h %1\include\*.h
copy ..\ICLNN\src\*.h %1\include\*.h
copy ..\ICLQt\src\*.h %1\include\*.h
copy ..\ICLUtils\src\*.h %1\include\*.h
copy compat\*.h %1\include\*.h

ECHO copy library files
copy lib\* %1\lib\*

ECHO copy doc files
copy ..\ICLBlob\doc\html\* %1doc\html\*
copy ..\ICLCC\doc\html\* %1doc\html\*
copy ..\ICLCore\doc\html\* %1doc\html\*
copy ..\ICLFilter\doc\html\* %1doc\html\*
copy ..\ICLIO\doc\html\* %1doc\html\*
copy ..\ICLMEx\doc\html\* %1doc\html\*
copy ..\ICLNN\doc\html\* %1doc\html\*
copy ..\ICLQt\doc\html\* %1doc\html\*
copy ..\ICLUtils\doc\html\* %1doc\html\*

::ECHO setting environment variables
::set ICLROOT=%1
::set lib=%ICLROOT%\lib;%lib%
::set include=%ICLROOT%doc;%include%
::set path=%ICLROOT%\bin;%path%

ECHO installation successful
PAUSE
GOTO END

:END
