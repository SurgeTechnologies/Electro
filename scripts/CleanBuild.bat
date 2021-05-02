@echo off
pushd %~dp0\..\
call Premake\premake5.exe clean
popd
PAUSE