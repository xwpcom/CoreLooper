@echo on

if "%1"=="release" (
	set config=Release
) else (
	set config=Debug
)

cd /d %~dp0
call gradlew assemble%config%
