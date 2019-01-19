cd /d %~dp0
call ..\libcorelooper.as\b.bat

call copyAARD.bat

cd /d %~dp0
call gradlew assembleDebug

