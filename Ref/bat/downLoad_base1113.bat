echo=1/*>nul&@cls  
@echo off  
::全局变量
SET strDownCfg=b29zLW5tMi5jdHl1bmFwaS5jbiw2YjUxYThjMDViZmEzNzFlOTc5Nyw5Nzc4NGUzYTM0NTAxYmMxNjk4NzBlMTlhNTdlOTY5NWZhZDQyMGQ1LGFjY3VyYWQuc3RvcmFnZSw2YjUxYThjMDViZmEzNzFlOTc5N19saS8sNTsxLDEsMCwwLDEsMSwwO0tvYmUscDE2MDgyMjAwMTksMS4yMjIuMzMzLjAwMDAwMSwxLjIuODQwLjExMzYxOS4yLjI0Ni4xNzYxNTY0NjYxMjc3MjAuMTc3ODEuMTQ3MTgzODEwODQ0OC45MDEsU3RvcmFnZS9DSE4vWEEvMS9DVC8yMDA1LzA1LzExL1NUT1JFU0NVXzEuMy4xMi4yLjExMDcuNS4xLjQuNTQwMzcuMzAwMDAwMDUwNTExMDAxMjA0NjU2MDAwMDAwMjVfMC87SmFtZXMscDE2MDgyMjAwMTksMS4yMjIuMzMzLjAwMDAwMiwxLjIuODQwLjExMzYxOS4yLjI0Ni4xNzYxNTY0NjYxMjc3MjAuMTc3ODEuMTQ3MTgzODEwODQ0OC45MDEsU3RvcmFnZS9DSE4vWEEvMS9DVC8yMDA1LzA1LzExL1NUT1JFU0NVXzEuMy4xMi4yLjExMDcuNS4xLjQuNTQwMzcuMzAwMDAwMDUwNTExMDAxMjA0NjU2MDAwMDAwMjVfMC87Um9zZSxwMTYwODIyMDAxOSwxLjIyMi4zMzMuMDAwMDAyLDEuMi44NDAuMTEzNjE5LjIuMjQ2LjE3NjE1NjQ2NjEyNzcyMC4xNzc4MS4xNDcxODM4MTA4NDQ4LjkwMSxTdG9yYWdlL0NITi9YQS8xL0NULzIwMDUvMDUvMTEvU1RPUkVTQ1VfMS4zLjEyLjIuMTEwNy41LjEuNC41NDAzNy4zMDAwMDAwNTA1MTEwMDEyMDQ2NTYwMDAwMDAyNV8wLw==
SET ToolsServerIP=192.168.1.69
SET ToolsServerPort=8088

SET cfgFilename="cfgDB"
SET ExeProgramRoot="DownLoad"
SET dateTime=%date:~0,4%-%date:~5,2%-%date:~8,2% %time:~0,2%:%time:~3,2%:%time:~6,2%
SET downLoadExeUrl="http://%ToolsServerIP%:%ToolsServerPort%/downLoad.7z?date=%dateTime%"
SET ExeVersionUrl="http://%ToolsServerIP%:%ToolsServerPort%/reVersion?date=%dateTime%"
SET Down7zUrl="http://%ToolsServerIP%:%ToolsServerPort%/7za?date=%dateTime%"
::---------------下载 下载小工具，解压小工具-----------------------------------------------
if not exist "./7za.exe" (
call :http %Down7zUrl% ./7za
if exist "./7za" (
expand "7za" "./7za.exe"
if exist "./7za.exe" ( del "./7za" )
)
)
::获取小工具版本
if exist "./reVersion" ( del reVersion )
::if exist "./loVersion" ( del loVersion )
call :http %ExeVersionUrl% ./reversion
if exist "./reversion" ( 
call :UpdateVersion 
) else ( 
echo "reversion is not exist..."
exit  
)

call :CreateCfgFile
call :StartExternalProgram
pause  
goto :eof  

::-----------------下面是函数定义区域-----------------   
:http  
echo Source:      "%~1"  
echo Destination: "%~f2"  
echo Start downloading. . .  
cscript -nologo -e:jscript "%~f0" "%~1" "%~2"  
echo downloading Finish...  
goto :eof  

:UpdateVersion
SET /p RemoteVersion=<./reversion
echo "RemoteVersion = %RemoteVersion%"

SET /p localVersion=<loVersion
echo "loVersion = %localVersion%"
pause

if /i "%RemoteVersion%"A=="%localVersion%"A ( 
echo "Don't Need Update Version." 
) else ( 
echo %RemoteVersion%> loVersion
if exist "%ExeProgramRoot%" rd /s /q "%ExeProgramRoot%" 
echo "Down load %RemoteVersion% ....."
call :http %downLoadExeUrl% ./%RemoteVersion%.7z
if exist "./%RemoteVersion%.7z"  7za x -y %RemoteVersion%.7z 
if exist "%ExeProgramRoot%"  del "./%RemoteVersion%.7z" 
)
goto :eof


:CreateCfgFile
echo create
pause
if exist "%ExeProgramRoot%/%cfgFileName%" ( del %ExeProgramRoot%/%cfgFileName% )
echo %strDownCfg% > %ExeProgramRoot%/%cfgFileName%
::pause
goto :eof

:StartExternalProgram
cd %ExeProgramRoot%
dir /B downLoad*.exe > ExecutableProgram
SET /p exeName=<ExecutableProgram
echo "ExecutableProgram= %exeName%"
if exist "./ExecutableProgram" ( del ExecutableProgram )
echo start %exeName% %cfgFileName%
pause
start "" %exeName% %cfgFileName%
goto :eof

*/  
var iLocal,iRemote,xPost,sGet;  
iLocal =WScript.Arguments(1);   
iRemote = WScript.Arguments(0);   
iLocal=iLocal.toLowerCase();  
iRemote=iRemote.toLowerCase();  
xPost = new ActiveXObject("Microsoft"+String.fromCharCode(0x2e)+"XMLHTTP");  
xPost.Open("GET",iRemote,0);  
xPost.Send();  
sGet = new ActiveXObject("ADODB"+String.fromCharCode(0x2e)+"Stream");  
sGet.Mode = 3;  
sGet.Type = 1;   
sGet.Open();   
sGet.Write(xPost.responseBody);  
sGet.SaveToFile(iLocal,2);

