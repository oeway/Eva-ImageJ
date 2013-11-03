@echo off
SETLOCAL EnableDelayedExpansion
 
set _PELICAN=$pelican
set _PELICANOPTS=$pelicanopts
 
set _BASEDIR=%cd%
set _INPUTDIR=%_BASEDIR%/content
set _OUTPUTDIR=%_BASEDIR%/output
set _CONFFILE=%_BASEDIR%/pelicanconf.py
 
set _cmmd=tasklist /FI "IMAGENAME eq python.exe" /FO CSV /NH
 
IF EXIST %_BASEDIR%\devserver.pid (
  echo DevServer already running...killing...
  GOTO :killpydevserver
)
 
call :startpydevserver
GOTO:EOF
 
:startpelican
cd %_BASEDIR%
start %_PELICAN% --debug --autoreload -r %_INPUTDIR% -o %_OUTPUTDIR% -s %_CONFFILE% %_PELICANOPTS%
GOTO:EOF
 
:killpelican
taskkill /F /T /IM pelican.exe
GOTO:EOF
 
:startpydevserver
cd %_OUTPUTDIR%
start python -m SimpleHTTPServer
set o=
FOR /F "tokens=2* delims=," %%G IN ('%_cmmd%') DO (
  set /a o+=1
  set _srvpid=%%G
)
echo %_srvpid:~1,-1% > %_BASEDIR%\devserver.pid
call :startpelican
GOTO:EOF
 
:killpydevserver
FOR /F %%a in ('type "%_BASEDIR%\devserver.pid"') DO (
  echo Killing DevServer running at PID = %%a
  taskkill /PID %%a
)
del %_BASEDIR%\devserver.pid
call :killpelican
GOTO:EOF
 
ENDLOCAL