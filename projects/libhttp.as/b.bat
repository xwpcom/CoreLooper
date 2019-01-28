cd /d %~dp0
call ..\libcorelooper.as\b.bat

cd /d %~dp0
call ..\libcrypt.as\b.bat

cd /d %~dp0
call copyAARD.bat

cd /d %~dp0
call gradlew assembleDebug

