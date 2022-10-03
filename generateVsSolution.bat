@echo off
where premake5
IF %ERRORLEVEL% NEQ 0 (
	IF not exist premake-5.0.0-beta1-windows (
		curl -LO https://github.com/premake/premake-core/releases/download/v5.0.0-beta1/premake-5.0.0-beta1-windows.zip
		powershell -Command "Expand-Archive premake-5.0.0-beta1-windows.zip"
		del premake-5.0.0-beta1-windows.zip
	)
	premake-5.0.0-beta1-windows\\premake5.exe vs2019
) else (
	premake5 vs2019
)