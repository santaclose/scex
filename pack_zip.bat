rem Requires ResourceHacker directory in PATH

if not exist "%~dp0release" mkdir "%~dp0release"
if not exist "%~dp0release/assets" mkdir "%~dp0release/assets"

move /y "%~dp0bin\Release-windows-x86_64\scex\scex.exe" "%~dp0release\scex.exe"
move /y "%~dp0assets\fonts" "%~dp0release\assets\fonts"
move /y "%~dp0assets\icon32.png" "%~dp0release\assets\icon32.png"
move /y "%~dp0assets\config.json" "%~dp0release\assets\config.json"

ResourceHacker  -open "%~dp0release\scex.exe" -save "%~dp0release\scexi.exe" -action addskip -res "%~dp0assets\icon.ico" -mask ICONGROUP,MAINICON,
move /y "%~dp0release\scex.exe" "%~dp0bin\Release-windows-x86_64\scex\scex.exe"
move /y "%~dp0release\scexi.exe" "%~dp0release\scex.exe"

pushd release
powershell -Command "Compress-Archive -Path .\*" -DestinationPath "scex_windows_x64.zip"
popd
if exist "%~dp0scex_windows_x64.zip" del "%~dp0scex_windows_x64.zip"
move /y "%~dp0release\scex_windows_x64.zip" "%~dp0scex_windows_x64.zip"

move /y "%~dp0release\assets\fonts" "%~dp0assets\fonts"
move /y "%~dp0release\assets\icon32.png" "%~dp0assets\icon32.png"
move /y "%~dp0release\assets\config.json" "%~dp0assets\config.json"

del "%~dp0release\scex.exe"
rmdir /q "%~dp0release\assets"
rmdir /q "%~dp0release"

pause