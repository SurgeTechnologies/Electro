@echo off
call cd ..
call cd ExampleApp
call cd build
echo [32m-----Building ExampleApp(C#) - Release-----[0m
call cmake --build . --config Release --target ALL_BUILD
echo [32m-----Building ExampleApp(C#) - Debug-----[0m
call cmake --build . --config Debug --target ALL_BUILD
pause