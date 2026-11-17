@echo off
REM Build and run the tests and the picture-making demos on Windows with MinGW g++ (C++17).
REM The live OpenGL window is built separately: see README.md
g++ -std=c++17 -Wall -Wextra -O2 -I src tests\tests.cpp -o run_tests.exe || exit /b 1
g++ -std=c++17 -Wall -Wextra -O2 -I src tests\viewer_tests.cpp -o run_viewer_tests.exe || exit /b 1
g++ -std=c++17 -Wall -Wextra -O2 -I src src\phase2_demo.cpp -o phase2_demo.exe || exit /b 1
g++ -std=c++17 -Wall -Wextra -O2 -I src src\phase3_demo.cpp -o phase3_demo.exe || exit /b 1
run_tests.exe
run_viewer_tests.exe
phase2_demo.exe
phase3_demo.exe
