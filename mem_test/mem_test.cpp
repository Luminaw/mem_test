#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <numeric> // For std::iota
#include <thread>
#include <mutex>
#include <condition_variable>

// Function to print a progress bar
void printProgressBar(long long current, long long total, int width) {
    float progress = (float)current / total;
    int barWidth = width - 7; // Account for " [xx%] "
    int pos = barWidth * progress;

    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::setw(3) << (int)(progress * 100.0) << "%" << std::flush;
}

// Mutex for protecting shared resources
std::mutex progress_mutex;

// Function to perform a single memory test iteration on a chunk of memory
bool runMemoryTestChunk(std::vector<char>& buffer, size_t start, size_t end,
                        unsigned int pattern, const std::string& testName) {
    long long chunkSize = (end - start) / 100; // Update progress every 1%

    std::cout << "  " << testName << " (Write): ";
    // Write pattern
    for (size_t i = start; i < end; ++i) {
        buffer[i] = static_cast<char>(pattern);
        if (chunkSize > 0 && ((i - start) % chunkSize == 0 || i == end - 1)) {
            std::lock_guard<std::mutex> lock(progress_mutex);
            printProgressBar(i - start + 1, end - start, 50);
        }
    }
    std::cout << std::endl;

    std::cout << "  " << testName << " (Verify): ";
    // Verify pattern
    for (size_t i = start; i < end; ++i) {
        if (buffer[i] != static_cast<char>(pattern)) {
            std::cerr << "\nError: Mismatch at index " << i << ". Expected "
                      << std::hex << static_cast<int>(pattern) << ", got "
                      << std::hex << static_cast<int>(buffer[i]) << std::endl;
            return false;
        }
        if (chunkSize > 0 && ((i - start) % chunkSize == 0 || i == end - 1)) {
            std::lock_guard<std::mutex> lock(progress_mutex);
            printProgressBar(i - start + 1, end - start, 50);
        }
    }
    std::cout << std::endl;
    return true;
}

// Function to perform a memory test with incrementing values on a chunk of memory
bool runIncrementingMemoryTestChunk(std::vector<char>& buffer, size_t start, size_t end,
                                    const std::string& testName) {
    long long chunkSize = (end - start) / 100; // Update progress every 1%

    std::cout << "  " << testName << " (Write): ";
    // Write incrementing values
    for (size_t i = start; i < end; ++i) {
        buffer[i] = static_cast<char>((i + start) % 256);
        if (chunkSize > 0 && ((i - start) % chunkSize == 0 || i == end - 1)) {
            std::lock_guard<std::mutex> lock(progress_mutex);
            printProgressBar(i - start + 1, end - start, 50);
        }
    }
    std::cout << std::endl;

    std::cout << "  " << testName << " (Verify): ";
    // Verify incrementing values
    for (size_t i = start; i < end; ++i) {
        if (buffer[i] != static_cast<char>((i + start) % 256)) {
            std::cerr << "\nError: Incrementing pattern mismatch at index " << i << ". Expected "
                      << std::hex << static_cast<int>((i + start) % 256) << ", got "
                      << std::hex << static_cast<int>(buffer[i]) << std::endl;
            return false;
        }
        if (chunkSize > 0 && ((i - start) % chunkSize == 0 || i == end - 1)) {
            std::lock_guard<std::mutex> lock(progress_mutex);
            printProgressBar(i - start + 1, end - start, 50);
        }
    }
    std::cout << std::endl;
    return true;
}

// Function to perform a memory test with random values on a chunk of memory
bool runRandomMemoryTestChunk(std::vector<char>& buffer, size_t start, size_t end,
                              std::mt19937& rng, const std::string& testName) {
    long long chunkSize = (end - start) / 100; // Update progress every 1%
    std::vector<char> original_values(end - start);
    std::uniform_int_distribution<unsigned int> dist(0, 255);

    std::cout << "  " << testName << " (Write): ";
    // Write random values
    for (size_t i = start; i < end; ++i) {
        original_values[i - start] = static_cast<char>(dist(rng));
        buffer[i] = original_values[i - start];
        if (chunkSize > 0 && ((i - start) % chunkSize == 0 || i == end - 1)) {
            std::lock_guard<std::mutex> lock(progress_mutex);
            printProgressBar(i - start + 1, end - start, 50);
        }
    }
    std::cout << std::endl;

    std::cout << "  " << testName << " (Verify): ";
    // Verify random values
    for (size_t i = start; i < end; ++i) {
        if (buffer[i] != original_values[i - start]) {
            std::cerr << "\nError: Random pattern mismatch at index " << i << ". Expected "
                      << std::hex << static_cast<int>(original_values[i - start]) << ", got "
                      << std::hex << static_cast<int>(buffer[i]) << std::endl;
            return false;
        }
        if (chunkSize > 0 && ((i - start) % chunkSize == 0 || i == end - 1)) {
            std::lock_guard<std::mutex> lock(progress_mutex);
            printProgressBar(i - start + 1, end - start, 50);
        }
    }
    std::cout << std::endl;
    return true;
}

// Thread function for running memory tests
void workerThread(size_t threadId, size_t numThreads, size_t blockSizeBytes,
                  std::mt19937& rng, bool& overallSuccess) {
    size_t chunkSize = blockSizeBytes / numThreads;
    size_t start = threadId * chunkSize;
    size_t end = (threadId == numThreads - 1) ? blockSizeBytes : start + chunkSize;

    std::vector<char> buffer(blockSizeBytes);

    // Test with fixed patterns
    if (!runMemoryTestChunk(buffer, start, end, 0xAA, "Pattern 0xAA")) { overallSuccess = false; return; }
    if (!runMemoryTestChunk(buffer, start, end, 0x55, "Pattern 0x55")) { overallSuccess = false; return; }
    if (!runMemoryTestChunk(buffer, start, end, 0xFF, "Pattern 0xFF")) { overallSuccess = false; return; }
    if (!runMemoryTestChunk(buffer, start, end, 0x00, "Pattern 0x00")) { overallSuccess = false; return; }

    // Test with incrementing pattern
    if (!runIncrementingMemoryTestChunk(buffer, start, end, "Incrementing")) {
        overallSuccess = false;
        return;
    }

    // Test with random pattern
    if (!runRandomMemoryTestChunk(buffer, start, end, rng, "Random")) {
        overallSuccess = false;
        return;
    }
}

int main() {
    std::cout << "User-Mode Memory Test\n";
    std::cout << "----------------------\n";

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

    // Get number of hardware threads (CPU cores)
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) {
        std::cerr << "Error: Could not determine the number of CPU cores. Defaulting to 1 thread." << std::endl;
        numThreads = 1;
    }

    std::cout << "  Using " << numThreads << " threads" << std::endl;

    std::random_device rd;
    std::mt19937 rng(rd());

    bool overallSuccess = true;
    auto totalStartTime = std::chrono::high_resolution_clock::now();

    for (long long i = 0; i < iterations; ++i) {
        std::cout << "\n--- Iteration " << (i + 1) << " ---\n";
        auto iterationStartTime = std::chrono::high_resolution_clock::now();

        // Create and launch threads
        std::vector<std::thread> threads;
        for (size_t threadId = 0; threadId < numThreads; ++threadId) {
            threads.emplace_back(workerThread, threadId, numThreads, blockSizeBytes,
                                 std::ref(rng), std::ref(overallSuccess));
        }

        // Join all threads
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

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
