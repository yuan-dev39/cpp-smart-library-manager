@echo off
setlocal
set "ROOT=%~dp0"
set "CXX=g++"
set "CXXFLAGS=-std=c++17 -O2 -Wall -Wextra -pedantic -static -static-libgcc -static-libstdc++"

if exist "C:\msys64\ucrt64\bin\g++.exe" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
where g++ >nul 2>nul
if errorlevel 1 goto :compiler_missing

if not exist "%ROOT%build" mkdir "%ROOT%build"
if not exist "%ROOT%release" mkdir "%ROOT%release"
if exist "%ROOT%build\library_cli.exe" del /q "%ROOT%build\library_cli.exe"
if exist "%ROOT%build\library_tests.exe" del /q "%ROOT%build\library_tests.exe"
if exist "%ROOT%build\library_manager.exe" del /q "%ROOT%build\library_manager.exe"

"%CXX%" %CXXFLAGS% -municode -mwindows -I"%ROOT%include" "%ROOT%src\library.cpp" "%ROOT%src\windows_gui.cpp" -o "%ROOT%build\library_manager.exe" -lcomctl32
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)
copy /y "%ROOT%build\library_manager.exe" "%ROOT%release\SmartLibraryManager.exe" >nul
if errorlevel 1 (
    echo Release copy failed.
    exit /b 1
)
echo GUI build completed successfully: build\library_manager.exe
echo Ready-to-run release: release\SmartLibraryManager.exe
exit /b 0

:compiler_missing
echo g++ was not found. Please install MSYS2 UCRT64.
exit /b 1
