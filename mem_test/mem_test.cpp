#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <numeric> // For std::iota

// Function to perform a single memory test iteration
bool runMemoryTest(size_t blockSize, unsigned int pattern, std::mt19937& rng) {
    std::vector<char> buffer(blockSize);

    // Write pattern
    for (size_t i = 0; i < blockSize; ++i) {
        buffer[i] = static_cast<char>(pattern);
    }

    // Verify pattern
    for (size_t i = 0; i < blockSize; ++i) {
        if (buffer[i] != static_cast<char>(pattern)) {
            std::cerr << "Error: Mismatch at index " << i << ". Expected "
                      << std::hex << static_cast<int>(pattern) << ", got "
                      << std::hex << static_cast<int>(buffer[i]) << std::endl;
            return false;
        }
    }
    return true;
}

// Function to perform a memory test with incrementing values
bool runIncrementingMemoryTest(size_t blockSize) {
    std::vector<char> buffer(blockSize);

    // Write incrementing values
    for (size_t i = 0; i < blockSize; ++i) {
        buffer[i] = static_cast<char>(i % 256);
    }

    // Verify incrementing values
    for (size_t i = 0; i < blockSize; ++i) {
        if (buffer[i] != static_cast<char>(i % 256)) {
            std::cerr << "Error: Incrementing pattern mismatch at index " << i << ". Expected "
                      << std::hex << static_cast<int>(i % 256) << ", got "
                      << std::hex << static_cast<int>(buffer[i]) << std::endl;
            return false;
        }
    }
    return true;
}

// Function to perform a memory test with random values
bool runRandomMemoryTest(size_t blockSize, std::mt19937& rng) {
    std::vector<char> buffer(blockSize);
    std::vector<char> original_values(blockSize);
    std::uniform_int_distribution<unsigned int> dist(0, 255);

    // Write random values
    for (size_t i = 0; i < blockSize; ++i) {
        original_values[i] = static_cast<char>(dist(rng));
        buffer[i] = original_values[i];
    }

    // Verify random values
    for (size_t i = 0; i < blockSize; ++i) {
        if (buffer[i] != original_values[i]) {
            std::cerr << "Error: Random pattern mismatch at index " << i << ". Expected "
                      << std::hex << static_cast<int>(original_values[i]) << ", got "
                      << std::hex << static_cast<int>(buffer[i]) << std::endl;
            return false;
        }
    }
    return true;
}

int main() {
    std::cout << "C++ User-Mode Memory Test\n";
    std::cout << "--------------------------\n";

    size_t blockSizeMB;
    long long iterations;

    std::cout << "Enter block size in MB (e.g., 100 for 100MB): ";
    std::cin >> blockSizeMB;

    std::cout << "Enter number of iterations: ";
    std::cin >> iterations;

    if (blockSizeMB <= 0 || iterations <= 0) {
        std::cerr << "Error: Block size and iterations must be positive." << std::endl;
        return 1;
    }

    size_t blockSizeBytes = blockSizeMB * 1024 * 1024;

    std::cout << "\nStarting memory test with:\n";
    std::cout << "  Block Size: " << blockSizeMB << " MB (" << blockSizeBytes << " bytes)\n";
    std::cout << "  Iterations: " << iterations << std::endl;

    std::random_device rd;
    std::mt19937 rng(rd());

    bool overallSuccess = true;
    auto totalStartTime = std::chrono::high_resolution_clock::now();

    for (long long i = 0; i < iterations; ++i) {
        std::cout << "\n--- Iteration " << (i + 1) << " ---\n";
        auto iterationStartTime = std::chrono::high_resolution_clock::now();

        // Test with fixed patterns
        if (!runMemoryTest(blockSizeBytes, 0xAA, rng)) { overallSuccess = false; break; }
        if (!runMemoryTest(blockSizeBytes, 0x55, rng)) { overallSuccess = false; break; }
        if (!runMemoryTest(blockSizeBytes, 0xFF, rng)) { overallSuccess = false; break; }
        if (!runMemoryTest(blockSizeBytes, 0x00, rng)) { overallSuccess = false; break; }

        // Test with incrementing pattern
        if (!runIncrementingMemoryTest(blockSizeBytes)) { overallSuccess = false; break; }

        // Test with random pattern
        if (!runRandomMemoryTest(blockSizeBytes, rng)) { overallSuccess = false; break; }

        auto iterationEndTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> iterationDuration = iterationEndTime - iterationStartTime;
        std::cout << "Iteration " << (i + 1) << " completed in " << std::fixed << std::setprecision(2)
                  << iterationDuration.count() << " ms." << std::endl;

        if (!overallSuccess) {
            std::cout << "Memory test failed during iteration " << (i + 1) << std::endl;
            break;
        }
    }

    auto totalEndTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> totalDuration = totalEndTime - totalStartTime;

    std::cout << "\n--------------------------\n";
    if (overallSuccess) {
        std::cout << "All memory tests passed successfully!\n";
    } else {
        std::cout << "Memory test failed!\n";
    }
    std::cout << "Total test duration: " << std::fixed << std::setprecision(2)
              << totalDuration.count() << " ms." << std::endl;

    return overallSuccess ? 0 : 1;
}
