#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <algorithm>
#include <mutex>
#include <fstream>
#include <string>
#include "json.hpp" // Include for JSON parsing
#include "main.h" // Custom header
#include "utilities.h" // Custom header
#include <queue>
#include <future>

/**
 * @brief Process image data and find top pixels.
 * 
 * @param image 2D vector representing the image data.
 * @param topPixels Vector to store top pixels.
 * @param startRow Starting row index for processing.
 * @param endRow Ending row index for processing.
 * @param mtx Mutex for thread safety.
 */
// Process image data and find top pixels using sorting and applying mutext for thread safety
void GetTopPixelsWithSorting(const std::vector<std::vector<int>>& image, std::vector<Pixel>& topPixels, int startRow, int endRow, std::mutex& mtx) {
    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < image[i].size(); ++j) {
            Pixel pixel;
            pixel.value = image[i][j];
            pixel.row = i;
            pixel.col = j;
            // Lock mutex for scoped access
            std::lock_guard<std::mutex> lock(mtx); 
            // Check if topPixels is not full
            if (topPixels.size() < 50) { 
                //If topPixels is not full, Add pixel to topPixels
                topPixels.push_back(pixel); 
                std::sort(topPixels.begin(), topPixels.end(), [](const Pixel& a, const Pixel& b) {
                    // Sort in descending order based on pixel value
                    return a.value > b.value; 
                });
            // Check if pixel value is greater than the smallest value in topPixels
            } else if (pixel.value > topPixels.back().value) {
                // Remove the smallest value from topPixels
                topPixels.pop_back(); 
                // Add new pixel to topPixels
                topPixels.push_back(pixel); 
                std::sort(topPixels.begin(), topPixels.end(), [](const Pixel& a, const Pixel& b) {
                    // Sort in descending order based on pixel value
                    return a.value > b.value;
                });
            }
        }
    }
}

/**
 * @brief Process a chunk of image data and update a Priority Queue of top pixels.
 * 
 * @param chunk Chunk of image data to process.
 * @param startRow Starting row index of the chunk.
 * @param endRow Ending row index of the chunk.
 * @param maxHeap Priority Queue to store top pixels.
 * @param mtx Mutex for thread safety.
 */
// Process a chunk of image data asynchronously and return a Priority Queue of top pixels with mutext applied for concurrency control
void GetTopPixelsWithMaxHeap(const std::vector<std::vector<int>>& chunk, int startRow, int endRow, std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels>& maxHeap, std::mutex& mtx) {
    for (int i = 0; i < chunk.size(); ++i) {
        for (int j = 0; j < chunk[i].size(); ++j) {
            // Lock mutex for scoped access
            std::lock_guard<std::mutex> lock(mtx);
            // Add pixel to maxHeap
            maxHeap.push({chunk[i][j], startRow + i, j});
            // Check if maxHeap size exceeds limit
            if (maxHeap.size() > 50) {
                // Remove the smallest value from maxHeap
                maxHeap.pop();
            }
        }
    }
}

/**
 * @brief Process a chunk of image data asynchronously and return a Priority Queue of top pixels.
 * 
 * @param chunk Chunk of image data to process.
 * @param startRow Starting row index of the chunk.
 * @param endRow Ending row index of the chunk.
 * @return std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels> Priority Queue containing top pixels.
 */
// Process a chunk of image data asynchronously and return a Priority Queue of top pixels with no mutext applied
std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels> GetTopPixelsWithMaxHeapAsync(std::vector<std::vector<int>> chunk, int startRow, int endRow) {
    std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels> maxHeap;
    for (int i = 0; i < chunk.size(); ++i) {
        for (int j = 0; j < chunk[i].size(); ++j) {
            // Add pixel to maxHeap
            maxHeap.push({chunk[i][j], startRow + i, j});
            // Check if maxHeap size exceeds limit
            if (maxHeap.size() > 50) {
                // Remove the smallest value from maxHeap
                maxHeap.pop();
            }
        }
    }

    return maxHeap;
}

/**
 * @brief Find top pixels in the image using multiple threads with mutex.
 * 
 * @param image Reference to the 2D vector representing the image.
 * @param numThreads Number of threads to use for processing.
 * @return A vector containing the top pixels found.
 */
// Find top pixels in the image using multiple threads
std::vector<Pixel> FindTopPixelsConcurrently(const std::vector<std::vector<int>>& image, int numThreads) {
    //Max Heap to hold top elements captured from each thread
    std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels> maxHeap;
    // Mutex for thread safety
    std::mutex mtx;

    const int rows = image.size();
    const int chunkSize = (rows + numThreads - 1) / numThreads;

    // Vector to hold thread objects
    std::vector<std::thread> threads;
    // Vector to hold top pixels
    std::vector<Pixel> topPixels;

    // Loop to create threads
    for (int i = 0; i < numThreads; ++i) {
        //Calculate Start & End rows to be processed by each thread depending upon number of threads configured
        const int startRow = (i * rows) / numThreads;
        const int endRow = ((i + 1) * rows) / numThreads;
        std::cout << "startRow " << startRow << " endRow " << endRow << '\n';
        //Chunk out image data based on start and end rows
        const std::vector<std::vector<int>> chunk(image.begin() + startRow, image.begin() + endRow);
        // Start a thread to process the chunk
        threads.emplace_back(GetTopPixelsWithMaxHeap, chunk, startRow, endRow, std::ref(maxHeap), std::ref(mtx));
        // threads.emplace_back(processImageNew, std::ref(image), std::ref(topPixels), startRow, endRow, std::ref(mtx));
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }
    // Vector to hold final result
    std::vector<Pixel> result;

    // Extract top pixels from maxHeap to build result
    while (!maxHeap.empty()) {
        result.push_back(maxHeap.top());
        maxHeap.pop();
    }

    // Reverse the result to get top pixels in descending order
    std::reverse(result.begin(), result.end());
    return result;
}

/**
 * @brief Find top pixels in the image asynchronously using multiple threads without using mutex.
 * 
 * @param image Reference to the 2D vector representing the image.
 * @param numThreads Number of threads to use for processing.
 * @return A vector containing the top pixels found.
 */
// Find top pixels in the image with processing chunks of data asynchronously without blocking other processes
std::vector<Pixel> FindTopPixelsAsync(const std::vector<std::vector<int>>& image, int numThreads){
    // Vector to hold futures for asynchronous tasks
    std::vector<std::future<std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels>>> futuresAsync;
    std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels> maxHeap;

    const int rows = image.size();
    const int chunkSize = (rows + numThreads - 1) / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        const int startRow = (i * rows) / numThreads;
        const int endRow = ((i + 1) * rows) / numThreads;
        std::cout << "startRow " << startRow << " endRow " << endRow << '\n';
        const std::vector<std::vector<int>> chunk(image.begin() + startRow, image.begin() + endRow);
        // Start asynchronous task for each chunk with no blocking mechanism like mutex
        //Number of threads determine degree of parallelism
        futuresAsync.push_back(std::async(std::launch::async, GetTopPixelsWithMaxHeapAsync, chunk, startRow, endRow));
    }

    // Wait for all asynchronous tasks to complete and build a master maxHeap from returned results from all the processes
    for (auto& future : futuresAsync) {
        auto localResults = future.get();
        while(!localResults.empty()){
            maxHeap.push(localResults.top());
            localResults.pop();
        }
    }
    std::vector<Pixel> result;
    //Build result from maxheap to return top N pixels
    while(!maxHeap.empty() && result.size() <= 50){
        result.push_back(maxHeap.top());
        maxHeap.pop();
    }

    // Reverse the result to get top pixels in descending order
    std::reverse(result.begin(), result.end());
    return result;
}

/**
 * @brief Read configuration parameters from a JSON file.
 * 
 * This function reads configuration parameters such as width, height, and output file
 * from a JSON file.
 * 
 * @param filename Path to the JSON file.
 * @return A tuple containing width, height, and output file.
 */
std::tuple<uint32_t, uint32_t, std::string, std::string, bool> ReadConfigurationFromJson(const std::string& filename) {
    uint32_t width = 0, height = 0;
    std::string output_file;
    std::string log_file;
    bool is_using_async;

    try {
        // Open configuration file as passed
        std::ifstream file(filename);
        if (!file.is_open()) {
            // Throw exception if file cannot be opened
            throw std::runtime_error("Error opening JSON file."); 
        }
        
        // Parse JSON
        nlohmann::json j;
        file >> j;

        // Read width from JSON
        width = j["Width"]; 
        // Read height from JSON
        height = j["Height"];
        // Read output file path from JSON
        output_file = j["Output_file"]; 

        log_file = j["Log_file"];
        is_using_async = j["IsUsingAsync"]; 

    } catch (const std::exception& e) {
        // Print error message
        std::cerr << "Exception caught: " << e.what() << std::endl; 
    }
    // Return tuple of configuration parameters
    return {width, height, output_file, log_file, is_using_async}; 
}