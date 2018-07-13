@echo off
cd build
g++ -g -std=c++11 -c ..\src\asm\jump_x86_64_ms_pe_gas.S -o jump.o && ^
g++ -g -std=c++11 -c ..\src\asm\make_x86_64_ms_pe_gas.S -o make.o && ^
g++ -g -std=c++11 -c ..\src\asm\ontop_x86_64_ms_pe_gas.S -o ontop.o && ^
g++ -g -std=c++11 -c ..\src\main.cpp -o main.o && ^
g++ -g -std=c++11 jump.o make.o ontop.o main.o -o ..\main.exe && ^
..\main.exe
cd ..