@echo off
pushd %~dp0\..\
echo [32m-----Builidng Electro-----[0m
call mkdir build
call cd build
call cmake ..
echo [32m-----Builidng Electro(C++ - Debug)-----[0m
call cmake --build . --config Debug --target ALL_BUILD
echo [32m-----Builidng Electro(C++ - Release)-----[0m
call cmake --build . --config Release --target ALL_BUILD
call cd ..
call cd ExampleApp
call mkdir build
call cd build
call cmake ..
echo [32m-----Building ExampleApp(C# - Release)-----[0m
call cmake --build . --config Release --target ALL_BUILD
echo [32m-----Building ExampleApp(C# - Debug)-----[0m
call cmake --build . --config Debug --target ALL_BUILD
pause