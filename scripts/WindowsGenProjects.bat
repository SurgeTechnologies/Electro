@echo off
pushd %~dp0\..\
echo Electro's project file generation started...
call Premake\premake5.exe vs2019
popd
echo Electro.sln generated in project root directory! Open that in VisualStudio 2019 and hit the play/hit F5 to build the engine(and to run the editor). You can close this window now.
PAUSE