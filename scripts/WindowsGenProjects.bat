@echo off
pushd %~dp0\..\
call mkdir build
call cd build
call cmake ..
popd
PAUSE
