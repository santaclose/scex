@echo off
where premake5
IF %ERRORLEVEL% NEQ 0 (
	IF not exist premake-5.0.0-beta2-windows (
		curl -LO https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip
		powershell -Command "Expand-Archive premake-5.0.0-beta2-windows.zip"
		del premake-5.0.0-beta2-windows.zip
	)
	premake-5.0.0-beta2-windows\\premake5.exe vs2022
) else (
	premake5 vs2022
)
pause