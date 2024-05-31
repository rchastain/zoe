@echo off

set wboard=%~d0\applications\echecs\winboard-aa\WinBoard\winboard.exe

%wboard% -size average -debug -debugfile %~dp0wboard.debug -timeControl 5:00 ^
-cp -fcp %~dp0zoe.exe ^
-fd %~dp0

exit /b

%wboard% -size average -debug -debugfile %~dp0wboard.debug -timeControl 5:00 ^
-fcp %~dp0durandal64.exe ^
-fd %~dp0 -fUCCI /variant=gothic


%wboard% -zp -ics -icshost winboard.nl -icshelper timeseal -fcp durandal32.exe -fd C:\Roland\pascal\echecs\capablanca -autoKibitz -fUCI
