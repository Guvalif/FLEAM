rem call C:\Borland\Bcc55\bccenv.bat
mkdir Release
copy sqlite3.dll Release\sqlite3.dll /Y
make
pause
brc32 resource.rc Release\FLE@M.exe
pause