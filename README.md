# User-Mode Memory Test

A simple yet effective C++ console application for testing a region of your system's RAM. This tool is designed to find memory errors by writing various patterns to a user-defined block of memory and then verifying the integrity of that data.

It is a multi-threaded application that leverages the available CPU cores to perform tests quickly and efficiently.

## Features

- **User-configurable:** Specify the size of the memory block (in MB) and the number of test iterations.
- **Multi-threaded:** Utilizes all available hardware threads to parallelize the workload and speed up the testing process.
- **Comprehensive Test Patterns:** Employs a variety of patterns to detect different types of memory failures:
    - **Fixed Patterns:** `0xAA`, `0x55`, `0xFF`, `0x00`. These patterns are good for finding bits that are stuck high or low.
    - **Incrementing Pattern:** Writes a unique, predictable value to each memory location.
    - **Random Pattern:** Writes random data to memory to catch more complex, data-dependent errors.
- **Detailed Progress:** Displays a real-time progress bar for each write and verify operation.
- **Error Reporting:** If a mismatch is found, the application immediately reports the memory address, the expected value, and the actual value.
- **Performance Metrics:** Reports the time taken for each iteration and the total test duration.

## How It Works

The application allocates a block of memory in user space based on the size you provide. It then divides this block among a number of worker threads (equal to the number of logical CPU cores).

Each thread runs a suite of tests on its assigned memory chunk:
1.  **Write Phase:** The thread fills its memory chunk with a specific pattern.
2.  **Verify Phase:** The thread reads the data back and verifies that it matches the pattern that was written.

This process is repeated for all defined test patterns and for the number of iterations specified by the user. If any verification fails, the test stops and reports the error.

## Building

### Visual Studio (Windows)
The project includes a Visual Studio solution (`mem_test.sln`) and project file (`mem_test/mem_test.vcxproj`).

1.  Open `mem_test.sln` in Visual Studio.
2.  Select a build configuration (e.g., `Release`).
3.  Build the solution (F7 or Build > Build Solution).
4.  The executable will be located in a subfolder like `x64/Release/`.

### g++ (Linux/macOS)
If you are on a different platform, you can compile the source code using g++:
```bash
g++ -std=c++17 -O2 -pthread mem_test/mem_test.cpp -o mem_test
```

## Usage

Run the executable from your terminal. The application will prompt you to enter the following information:

1.  **Block size in MB:** The amount of RAM you want to test (e.g., `1024` for 1GB).
2.  **Number of iterations:** How many times the entire test suite should run.

### Example
```
User-Mode Memory Test
----------------------
Enter block size in MB (e.g., 100 for 100MB): 512
Enter number of iterations: 5

Starting memory test with:
  Block Size: 512 MB (536870912 bytes)
  Iterations: 5
  Using 16 threads

--- Iteration 1 ---
  Pattern 0xAA (Write): [================================================> ]  99%
  Pattern 0xAA (Verify): [================================================> ] 100%
  ...
```

## License

This project is licensed under the terms of the license specified in the `LICENSE` file.