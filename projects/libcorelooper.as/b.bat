@echo on

if "%1"=="release" (
	set config=Release
) else (
	set config=Debug
)

@echo compile %config% libs

cd /d %~dp0
@echo call gradlew assemble%config%
call gradlew assemble%config%
