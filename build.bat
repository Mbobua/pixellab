@echo off
REM Build and run everything on Windows with MinGW g++ (C++17).
g++ -std=c++17 -Wall -Wextra -O2 -I src tests\tests.cpp -o run_tests.exe || exit /b 1
g++ -std=c++17 -Wall -Wextra -O2 -I src src\phase2_demo.cpp -o phase2_demo.exe || exit /b 1
g++ -std=c++17 -Wall -Wextra -O2 -I src src\phase3_demo.cpp -o phase3_demo.exe || exit /b 1
run_tests.exe
phase2_demo.exe
phase3_demo.exe
