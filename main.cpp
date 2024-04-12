#include <iostream>
#include <thread>
#include <vector>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <future>
#include "main.h"
#include "utilities.h"

int main() {

    // Determine the number of CPU cores available for parallel processing
    int cores = std::thread::hardware_concurrency();
    std::cout << "hardware_concurrency: " << cores << '\n' ;
    
    // Read configuration parameters (width, height, output_file) from a JSON file
    auto [width, height, output_file, log_file, is_using_async] = ReadConfigurationFromJson("config.json");
    if(width <= 0 || height <= 0){
        std::cout << "Invalid Height/Width provided, please correct data" << std::endl;
        std::cout << std::endl;
    }
    // Generate custom image data based on the specified width and height
    auto image = GenerateCustomImage(width, height);

    if(is_using_async){
        std::cout << "Start executing with asynchronously without mutex\n";
    }else{
        std::cout << "Start executing with concurrent threads and mutex\n";
    }
    std::cout << std::endl;
    //-----------------------------------------------------------------------------------------------------

    // Iterate over the specified number of threads to test different configurations
    for( unsigned int nthreads = 1 ; nthreads < cores ; ++nthreads ) {
        std::cout << "Testing with " << nthreads << " thread(s):" << std::endl;

        // Start measuring execution time
        auto start = std::chrono::steady_clock::now();
        std::vector<Pixel> topPixels;
        // Find top pixels asynchronously using the specified number of threads
        if(is_using_async){
            topPixels = FindTopPixelsAsync(image, nthreads);
        }else{
            topPixels = FindTopPixelsConcurrently(image, nthreads);
        }

        // End measuring execution time
        auto end = std::chrono::steady_clock::now();

        // Calculate and display the execution time
        std::cout << "Execution time with " << nthreads << " thread(s): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " millisecs\n";

        //Write log file with 50 pixels captured
        WriteLogFile(log_file, topPixels);
        std::cout << std::endl;
        std::cout  << "Top 50 pixels with their locations:" << "\n";
        for (const auto& pixel : topPixels) {
            std::cout << "Value: " << pixel.value << ", Location: (" << pixel.row << ", " << pixel.col << ")" << "\n";
        }
        std::cout << std::endl;
    }
    return 0;
}

