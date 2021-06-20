@echo off
pushd %~dp0\..\
echo [32m-----Electro's project file generation started-----[0m
call mkdir build
call cd build
call cmake ..
call cd ..
call cd ExampleApp
call mkdir build
call cd build
call cmake ..
echo [32m-----Building ExampleApp(C#) - Release-----[0m
call cmake --build . --config Release --target ALL_BUILD
echo [32m-----Building ExampleApp(C#) - Debug-----[0m
call cmake --build . --config Debug --target ALL_BUILD
echo [32mElectro.sln is generated inside the build directory! Simply open that file, hit F5 to build the engine(Set ElectroEditor as the startup project to run the editor) You can safely close this window now![0m
popd
PAUSE