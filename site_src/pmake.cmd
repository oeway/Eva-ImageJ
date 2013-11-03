@echo off
SETLOCAL
 
set _PELICAN=pelican
set _PELICANOPTS=
 
set _BASEDIR=.
set _INPUTDIR=%_BASEDIR%\content
set _OUTPUTDIR=%_BASEDIR%\output
set _CONFFILE=%_BASEDIR%\pelicanconf.py
set _PUBLISHCONF=%_BASEDIR%\publishconf.py
 
set _FTP_HOST=$ftp_host
set _FTP_USER=$ftp_user
set _FTP_TARGET_DIR=$ftp_target_dir
 
set _SSH_HOST=$ssh_host
set _SSH_PORT=$ssh_port
set _SSH_USER=$ssh_user
set _SSH_TARGET_DIR=$ssh_target_dir
 
set _DROPBOX_DIR=$dropbox_dir
 
set _DEF=%1
set _UPLOAD=%2
 
IF "%_DEF%"=="html" (
  GOTO :html
)
IF "%_DEF%"=="clean" (
  GOTO :clean
)
IF "%_DEF%"=="regenerate" (
  GOTO :regenerate
)
IF "%_DEF%"=="publish" (
  GOTO :publish
)
IF "%_DEF%"=="serve" (
  GOTO :serve
)
IF "%_DEF%"=="devserver" (
  GOTO :devserver
)
IF "%_DEF%"=="upload" (
  GOTO :upload
)
IF "%_DEF%"=="help" (
  GOTO :help
)
 
echo No or incorrect argument given, please review your input or type
echo 'pmake help' for help.
GOTO :end
 
:help
echo Batch file for a pelican Web site
echo.
echo Usage:
echo pmake COMMAND(S)
echo.
echo Commands:
echo    html                        (re)generate the web site
echo    clean                       remove the generated files
echo    regenerate                  regenerate files upon modification
echo    publish                     generate using production settings
echo    serve                       serve site at http://localhost:8000
echo    devserver                   start/restart develop_server.sh
echo    upload ssh                  upload the web site via SSH
echo    upload rsync                upload the web site via rsync+ssh
echo    upload dropbox              upload the web site via Dropbox
echo    upload ftp                  upload the web site via FTP
echo    upload github               upload the web site via gh-pages
echo.
GOTO :end
 
:html
echo Running Pelican to generate HTML output
%_PELICAN% %_INPUTDIR% -o %_OUTPUTDIR% -s %_CONFFILE% %_PELICANOPTS%
echo Done
GOTO :end
 
:clean
echo Cleaning Output directory (deleting everything)
rmdir /S %_OUTPUTDIR%
mkdir %_OUTPUTDIR%
GOTO :end
 
:regenerate
echo Running Pelican and regenerating upon file changes
echo Press Ctrl-C to end, Y or N on batch job, either will work.
%_PELICAN% -r %_INPUTDIR% -o %_OUTPUTDIR% -s %_CONFFILE% %_PELICANOPTS%
GOTO :end
 
:serve
echo Press Ctrl-C to end the server.
echo Then Y or N to end the batch job. Both will end it.
cd %_OUTPUTDIR% && python -m SimpleHTTPServer
GOTO :end
 
:devserver
call %_BASEDIR%/devserver.cmd
GOTO :end
 
:publish
echo Generating production ready HTML
%_PELICAN% %_INPUTDIR% -o %_OUTPUTDIR% -s %_PUBLISHCONF% %_PELICANOPTS%
GOTO:EOF
 
:upload
IF "%_UPLOAD%"=="ssh" (
  GOTO :ssh
)
IF "%_UPLOAD%"=="rsync" (
  GOTO :rsync
)
IF "%_UPLOAD%"=="dropbox" (
  GOTO :dropbox
)
IF "%_UPLOAD%"=="ftp" (
  GOTO :ftp
)
IF "%_UPLOAD%"=="github" (
  GOTO :github
)
 
echo No upload location given. Please provide a destination/protocol.
echo You can use 'pmake help' to see the available options.
GOTO :end
 
:ssh
echo Running Pelican and publishing output through SSH
call :publish
echo You must have WinSCP in your PATH variable.
winscp.com /console /command "open scp://%_SSH_USER%@%_SSH_HOST%:%_SSH_PORT%" "option batch on" "option confirm off" "cd %_SSH_TARGET_DIR%" "put %_OUTPUTDIR%\*" "exit"
GOTO :end
 
:rsync
echo Running Pelican and publishing output through Rsync
call :publish
echo You must have Rsync in your PATH variable.
echo Current best Windows Rsync version located at:
echo http://sourceforge.net/projects/sereds/files/cwRsync/4.0.5/
set _RSYNC_OUTPUT=%_OUTPUTDIR:\=/%
rsync.exe -e "ssh -p %_SSH_PORT%" -P -rvz --delete "/cygdrive/c%_RSYNC_OUTPUT:~2%/*" %_SSH_USER%@%_SSH_HOST%:%_SSH_TARGET_DIR%
GOTO :end
 
:dropbox
echo Running Pelican and publishing output to Dropbox
%_PELICAN% %_INPUTDIR% -o %_DROPBOX_DIR% -s %_PUBLISHCONF% %_PELICANOPTS%
GOTO :end
 
:ftp
echo Running Pelican and publishing output through FTP
call :publish
echo You must have WinSCP in your PATH variable.
winscp.com /console /command "open ftp://%_FTP_USER%@%_FTP_HOST%" "option batch on" "option confirm off" "cd %_FTP_TARGET_DIR%" "put %_OUTPUTDIR\*" "exit"
GOTO :end
 
:github
echo Running Pelican and publishing output to Github
call :publish
ghp-import %_OUTPUTDIR%
git push origin gh-pages
 
:end
ENDLOCAL