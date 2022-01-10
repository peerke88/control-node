@echo off
rem batch file to build all maya plugins at once

cd %~dp0\rigSystem

python resourcecompiler.py
echo "build resources.h"

pause

set list= 2022

IF not exist %~dp0\build (mkdir %~dp0\build)
cd %~dp0\build


del *.* /Q

for %%a in (%list%) do (
    echo %%a
    cmake -G "Visual Studio 16 2019" -DMAYA_VERSION=%%a ../
    cmake --build . --config Release --target Install
    del *.* /Q
)
pause