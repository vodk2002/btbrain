@echo off
REM run.bat - BTBRAIN: pregunta hilos, retoma estado solo.
REM --db apunta a tu lista de direcciones ricas (una por linea,
REM opcional: direccion<TAB>balance). Ver README.md.
cd /d "%~dp0"
btbrain.exe --db ..\..\btcscan\db\btc_funded.tsv
pause
