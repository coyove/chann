echo off
:loop
ping 127.0.0.1 -n 10 >nul
tasklist | find "sql.exe" > nul && goto loop
echo the server crashed ! %TIME%
start start-server.bat
goto loop