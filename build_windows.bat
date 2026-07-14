@echo off
setlocal
set "ROOT=%~dp0"
set "CXX=g++"

if exist "C:\msys64\ucrt64\bin\g++.exe" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
where g++ >nul 2>nul
if errorlevel 1 goto :compiler_missing

if not exist "%ROOT%build" mkdir "%ROOT%build"
"%CXX%" -std=c++17 -O2 -Wall -Wextra -pedantic -I"%ROOT%include" "%ROOT%src\library.cpp" "%ROOT%src\main.cpp" -o "%ROOT%build\library_cli.exe"
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

"%CXX%" -std=c++17 -O2 -Wall -Wextra -pedantic -I"%ROOT%include" "%ROOT%src\library.cpp" "%ROOT%tests\library_tests.cpp" -o "%ROOT%build\library_tests.exe"
if errorlevel 1 exit /b 1

"%ROOT%build\library_tests.exe"
if errorlevel 1 exit /b 1
echo Build and tests completed successfully.
exit /b 0

:compiler_missing
echo g++ was not found. Please install MSYS2 UCRT64.
exit /b 1
