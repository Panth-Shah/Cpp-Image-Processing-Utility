// main.h

#ifndef _MAIN_H_
#define _MAIN_H_

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include "json.hpp" // Include for JSON parsing

// Represents a pixel in the image
struct Pixel {
    int value; // Pixel value
    int row; // Pixel row index
    int col; // Pixel column index
};

std::tuple<uint32_t, uint32_t, std::string, std::string, bool> ReadConfigurationFromJson(const std::string& filename);

//Implemented in image-generator.cpp
int WriteCustomImageFile(uint32_t w, uint32_t h, const std::string& out);
int WriteLogFile(const std::string& out, std::vector<Pixel>& topPixels);
std::vector<std::vector<int>> GenerateCustomImage(uint32_t w, uint32_t h);
#endif