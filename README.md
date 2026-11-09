# Algovisual — DSA & AoA Visualizer

A beginner-friendly educational project using C++17, Windows Winsock, HTML, CSS, and JavaScript.

## Features
- Bubble Sort, Selection Sort, Insertion Sort
- Linear Search and Binary Search
- Animated array states with Play / Pause / Prev / Next
- Comparisons, swaps, runtime measurement, and Big-O complexity
- Local C++ HTTP server; no third-party backend library required

## Requirements
- Windows
- A working GCC `g++` compiler (for example, MinGW-w64)
- A modern browser

## Run
1. Extract the entire `Algorithm_Visualizer` folder.
2. Open Command Prompt in the `Algorithm_Visualizer` folder.
3. Run `start-server.bat`.
4. Keep that terminal open.
5. Open `web/index.html` in Chrome or Edge.
6. If the browser blocks the local file from calling the server, run a tiny local static server from the project folder in a second terminal:
   - `python -m http.server 5500`
   - Then open `http://127.0.0.1:5500/web/`
7. The sidebar should say “Connected on port 8080”. Select an algorithm and press Run.

## Alternative manual compile
From the project root:
```bat
g++ -std=c++17 -O2 server.cpp -o server.exe -lws2_32
server.exe
```

## Test cases
- Sorting: `38, 12, 27, 5, 44, 19, 31, 8` → sorted output should be `5, 8, 12, 19, 27, 31, 38, 44`.
- Linear Search: use target `19` → found.
- Binary Search: use target `44` → found after the server sorts the values.
- Missing target: use `1000` → not found.

## Project structure
```text
Algorithm_Visualizer/
├── server.cpp
├── start-server.bat
├── README.md
└── web/
    ├── index.html
    ├── style.css
    └── app.js
```

## Notes for the report
- Bubble / Selection / Insertion Sort: worst-case time O(n²).
- Linear Search: worst-case time O(n).
- Binary Search: worst-case time O(log n), assuming sorted input. This implementation sorts the input first, so total time including sorting is O(n log n).
- also added best,worst and average case time Complexities 
- The runtime displayed is a measured local execution time and can vary with computer load and input size.
- This is an educational local demo, not a production server. Do not expose it to the public internet.
