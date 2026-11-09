@echo off
cd /d "%~dp0"
echo Building AlgoLab C++ server...
g++ -std=c++17 -O2 server.cpp -o server.exe -lws2_32
if errorlevel 1 (
  echo.
  echo Build failed. Check that g++ is installed and available in PATH.
  pause
  exit /b 1
)
echo.
echo Build successful.
echo Starting server at http://127.0.0.1:8080
echo Keep this window open while using the frontend.
server.exe
pause
