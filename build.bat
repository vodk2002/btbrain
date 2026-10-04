@echo off
REM build.bat - compila btbrain.exe (proyecto brain autocontenido).
REM Rutas relativas al repo; override con variables de entorno si mueves cosas:
REM   set BTSCAN_SRC=C:\ruta\btcscan   (fuentes crypto.c + secp256k1 + DLLs)
REM   set CURLMINGW=C:\ruta\curl-mingw (headers/libs OpenSSL)
set BB=%~dp0
set SRC=%BB%src
if "%BTSCAN_SRC%"=="" set BTSCAN_SRC=%BB%..\..\btcscan
if "%CURLMINGW%"=="" set CURLMINGW=%BB%..\..\..\..\curl-mingw
set INC_SSL=%CURLMINGW%\curl-8.22.0_1-win64-mingw\include
set LIB_SSL=%CURLMINGW%\curl-8.22.0_1-win64-mingw\lib
set SECP=%BTSCAN_SRC%\secp256k1\secp256k1-0.8.0
gcc -O3 -march=native -flto -pthread -DSECP256K1_STATIC -I"%SRC%" -I"%BTSCAN_SRC%\src" -I"%INC_SSL%" -I"%SECP%\include" ^
  "%SRC%\btbrain.c" "%SRC%\gen5.c" "%SRC%\addr.c" ^
  "%SRC%\bloom.c" "%SRC%\net.c" "%SRC%\tui.c" "%SRC%\win.c" ^
  "%SRC%\banner.c" "%SRC%\crypto.c" ^
  "%BTSCAN_SRC%\secp256k1\obj\secp256k1.o" ^
  "%BTSCAN_SRC%\secp256k1\obj\pre1.o" ^
  "%BTSCAN_SRC%\secp256k1\obj\pre2.o" ^
  -o "%BB%\btbrain.exe" ^
  "%BTSCAN_SRC%\libcrypto-3-x64.dll" ^
  "%BTSCAN_SRC%\libssl-3-x64.dll" ^
  -L"%LIB_SSL%" -lz -lwininet -lbcrypt -lws2_32
if errorlevel 1 (echo FALLO & exit /b 1) else echo OK btbrain.exe
