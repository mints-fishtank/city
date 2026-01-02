@echo off
setlocal enabledelayedexpansion

:: City - Launch Script for Windows
:: Usage: run.bat [command] [options]

set BUILD_DIR=build\debug
set SERVER_BIN=%BUILD_DIR%\src\server\city_server.exe
set CLIENT_BIN=%BUILD_DIR%\src\client\city_client.exe
set DEFAULT_PORT=7777

if "%1"=="" goto :help
if "%1"=="help" goto :help
if "%1"=="--help" goto :help
if "%1"=="-h" goto :help
if "%1"=="server" goto :server
if "%1"=="client" goto :client
if "%1"=="connect" goto :connect
if "%1"=="build" goto :build

echo Unknown command: %1
echo Run 'run.bat help' for usage
exit /b 1

:help
echo City Launch Script
echo.
echo Usage: run.bat ^<command^> [options]
echo.
echo Commands:
echo   server [port]           Start dedicated server (default port: %DEFAULT_PORT%)
echo   client [name]           Start client (creates local server automatically)
echo   connect [name] [host]   Connect to external server
echo   build                   Build both server and client
echo   help                    Show this help
echo.
echo Examples:
echo   run.bat client              # Single player (local server)
echo   run.bat client Alice        # Single player with name
echo   run.bat server              # Start dedicated server for others to join
echo   run.bat connect Bob         # Join server at localhost:%DEFAULT_PORT%
echo   run.bat connect Bob 192.168.1.5:7777  # Join remote server
exit /b 0

:check_build
if not exist "%SERVER_BIN%" goto :do_build
if not exist "%CLIENT_BIN%" goto :do_build
exit /b 0

:do_build
echo Binaries not found. Building...
cmake --build "%BUILD_DIR%" --target city_server city_client
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)
exit /b 0

:server
call :check_build
if errorlevel 1 exit /b 1
set PORT=%2
if "%PORT%"=="" set PORT=%DEFAULT_PORT%
echo Starting dedicated server on port %PORT%...
"%SERVER_BIN%" %PORT%
exit /b %errorlevel%

:client
call :check_build
if errorlevel 1 exit /b 1
set NAME=%2
if "%NAME%"=="" set NAME=Player
echo Starting client as %NAME% (local server)...
"%CLIENT_BIN%" --name "%NAME%"
exit /b %errorlevel%

:connect
call :check_build
if errorlevel 1 exit /b 1
set NAME=%2
set HOST=%3
if "%NAME%"=="" set NAME=Player
if "%HOST%"=="" set HOST=localhost:%DEFAULT_PORT%
echo Connecting to %HOST% as %NAME%...
"%CLIENT_BIN%" --connect "%HOST%" --name "%NAME%"
exit /b %errorlevel%

:build
echo Building server and client...
cmake --build "%BUILD_DIR%" --target city_server city_client
exit /b %errorlevel%
