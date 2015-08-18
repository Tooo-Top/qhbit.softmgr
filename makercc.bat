@ECHO OFF
CD /d %~dp0
CD lewang
python _makerc.py
CD ..
rcc -binary lewang\default.qrc -o .\essential.res