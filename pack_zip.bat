rem Requires ResourceHacker directory in PATH

if not exist "%~dp0release" mkdir "%~dp0release"
if not exist "%~dp0release/assets" mkdir "%~dp0release/assets"

move /y "%~dp0bin\Release-windows-x86_64\ste\ste.exe" "%~dp0release\ste.exe"
move /y "%~dp0assets\fonts" "%~dp0release\assets\fonts"
move /y "%~dp0assets\icon32.png" "%~dp0release\assets\icon32.png"
move /y "%~dp0assets\config.json" "%~dp0release\assets\config.json"

ResourceHacker  -open "%~dp0release\ste.exe" -save "%~dp0release\stei.exe" -action addskip -res "%~dp0assets\icon.ico" -mask ICONGROUP,MAINICON,
move /y "%~dp0release\ste.exe" "%~dp0bin\Release-windows-x86_64\ste\ste.exe"
move /y "%~dp0release\stei.exe" "%~dp0release\ste.exe"

pushd release
powershell -Command "Compress-Archive -Path .\*" -DestinationPath "ste_windows_x64.zip"
popd
if exist "%~dp0ste_windows_x64.zip" del "%~dp0ste_windows_x64.zip"
move /y "%~dp0release\ste_windows_x64.zip" "%~dp0ste_windows_x64.zip"

move /y "%~dp0release\assets\fonts" "%~dp0assets\fonts"
move /y "%~dp0release\assets\icon32.png" "%~dp0assets\icon32.png"
move /y "%~dp0release\assets\config.json" "%~dp0assets\config.json"

del "%~dp0release\ste.exe"
rmdir /q "%~dp0release\assets"
rmdir /q "%~dp0release"

pause