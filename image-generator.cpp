#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"
#include "main.h"
#include "utilities.h"

/**
 * @brief Writes a custom image file with given dimensions and pixel values.
 * 
 * @param w Width of the image.
 * @param h Height of the image.
 * @param out Filepath to save the image.
 * @return 0 if successful, -1 if file cannot be opened.
 */

int WriteCustomImageFile(uint32_t w, uint32_t h, const std::string& out) {
    try {
        // Open the file for writing
        std::ofstream f(out);
        if (!f.is_open()) {
            // Check if the file is opened successfully
            return -1; // Return -1 if failed to open file
        }

        // Write image dimensions to the file
        f << w << " " << h << "\n";

        // Generate and write pixel values to the file
        for (unsigned long long i = 0; i < w * h; i++) {
            f << std::hex << (rand() % 0x10000) << " ";
        }
        f << "\n";

        // Close the file
        f.close();

        return 0; // Return 0 if successful
    } catch (const std::exception& e) {
        // Print error message
        std::cerr << "Error writing image file: " << e.what() << std::endl;
        return -1; // Return -1 on error
    }
}
/**
 * @brief Writes a log file for given results of 50 pixels.
 * 
 * @param out Filepath to save the image.
 * @return 0 if successful, -1 if file cannot be opened.
 */
int WriteLogFile(const std::string& out, std::vector<Pixel>& topPixels) {
    try {
        // Open the file for writing
        std::ofstream f(out);
        if (!f.is_open()) {
            // Check if the file is opened successfully
            return -1; // Return -1 if failed to open file
        }

        // Write image dimensions to the file
        f << "Top 50 pixels with their locations:" << "\n";

        // Print the top 50 pixels along with their locations
        for (const auto& pixel : topPixels) {
            f << "Value: " << pixel.value << ", Location: (" << pixel.row << ", " << pixel.col << ")" << "\n";
        }
        f << "\n";

        // Close the file
        f.close();

        return 0; // Return 0 if successful
    } catch (const std::exception& e) {
        // Print error message
        std::cerr << "Error writing log file: " << e.what() << std::endl;
        return -1; // Return -1 on error
    }
}
/**
 * @brief Generates dummy image data with given dimensions.
 * 
 * @param w Width of the image.
 * @param h Height of the image.
 * @return A 2D vector representing the generated image.
 */
std::vector<std::vector<int>> GenerateCustomImage(uint32_t w, uint32_t h) {
    // Initialize a 2D vector to store image data
    std::vector<std::vector<int>> image(h, std::vector<int>(w));

    // Seed the random number generator with current time
    srand(time(0));

    // Loop through each pixel and generate random pixel values
    for (uint32_t i = 0; i < h; ++i) {
        for (uint32_t j = 0; j < w; ++j) {
            // Generate random pixel value
            image[i][j] = rand() % (1 << 16);
        }
    }

    return image; // Return the generated image data
}