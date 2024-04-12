// utilities.h

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <algorithm>
#include <mutex>
#include <queue>
#include <fstream>
#include <string>
#include "json.hpp"
#include "main.h"
#include "utilities.h"
#include <future>

// Comparator for pixels to be used in priority queue
struct ComparePixels {
    bool operator()(const Pixel& p1, const Pixel& p2) {
        return p1.value > p2.value; // Compare based on pixel values
    }
};

//Implemented in utilities.cpp
std::vector<Pixel> FindTopPixelsAsync(const std::vector<std::vector<int>>& image,  int numThreads);
std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels> GetTopPixelsWithMaxHeapAsync(std::vector<std::vector<int>> chunk, int startRow, int endRow);

std::vector<Pixel> FindTopPixelsConcurrently(const std::vector<std::vector<int>>& image, int numThreads);
void GetTopPixelsWithMaxHeap(const std::vector<std::vector<int>>&, int startRow, int endRow, std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels>& maxHeap, std::mutex& mtx);

void GetTopPixelsWithSorting(const std::vector<std::vector<int>>& image, std::vector<Pixel>& topPixels, int startRow, int endRow, std::mutex& mtx);
#endif