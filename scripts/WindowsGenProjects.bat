@echo off
pushd %~dp0\..\
echo Electro's project file generation started...
call mkdir build
call cd build
call cmake ..
popd
echo Electro.sln generated in build directory! Open that in VisualStudio 2019 and hit the play/hit F5 to build the engine(and to run the editor). You can close this window(safely) now!
PAUSE