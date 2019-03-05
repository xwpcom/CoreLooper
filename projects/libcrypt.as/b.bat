@echo "b" to compile debug version
@echo "b release" to compile release version

if "%1"=="release" (
	set config=Release
) else (
	set config=Debug
)


@echo compile libs
cd /d %~dp0
call ..\libcorelooper.as\b.bat %1

@echo copy libs
cd /d %~dp0
set libname=libcorelooper
copy /Y ..\%libname%.as\%libname%\build\outputs\aar\%libname%-%config%.aar %libname%\%libname%.aar

@echo compile current project
cd /d %~dp0
call gradlew assemble%config%
