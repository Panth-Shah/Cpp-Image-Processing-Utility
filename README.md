# Problem Statement:
___
* Given a grayscale image in memory with each 16-bit word equating to a pixel value:
    - write a program to find the 50 pixels (including their location) with the highest pixel values.
    - Can be written in C or C++.
    - Please also provide any assumptions made in solving the problem.

# Solution Details:
___
Outlining approach taken here to solve the problem is broken down into smaller sub-problems and tasked as below:

1. **Capturing Custom Dimensions and Configuration Details**: Write a utility to read configuration json which would be used to capture height and width details to produce raw image data along with other configuration details (e.g. export file name, log file name, process toggle for execution).
    ```C++
    std::tuple<uint32_t, uint32_t, std::string, std::string, bool> ReadConfigurationFromJson(const std::string& filename);
    ```
2. **Generating Raw Image Data**: Write a funtion which would take width(w) and height(h) dimensions read from configuration json to produce 2D grid with each cell value considered as a pixel and each pixel of this grid will be 16-bit word.
    ```C++
    int WriteCustomImageFile(uint32_t w, uint32_t h, const std::string& out);
    ```
3. **Processing Pixes**: Define a function to process raw image as 2D grid sequentially or using multi-threading.
    ```C++
    std::vector<Pixel> FindTopPixelsAsync(const std::vector<std::vector<int>>& image,  int numThreads);

    std::vector<Pixel> FindTopPixelsConcurrently(const std::vector<std::vector<int>>& image, int numThreads);
    ```
4. **Identify Top 50 Pixels**: Write a function to define data structure and algorithm which would be used to parse raw image as a 2D grid and capture 50 pixels with heighest pixel values including their locations.
    ```C++
    std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels> GetTopPixelsWithMaxHeapAsync(std::vector<std::vector<int>> chunk, int startRow, int endRow);

    void GetTopPixelsWithMaxHeap(const std::vector<std::vector<int>>&, int startRow, int endRow, std::priority_queue<Pixel, std::vector<Pixel>, ComparePixels>& maxHeap, std::mutex& mtx);

    void GetTopPixelsWithSorting(const std::vector<std::vector<int>>& image, std::vector<Pixel>& topPixels, int startRow, int endRow, std::mutex& mtx);
    ```
5. **Merging Results**: Merge results produced by mutliple threads paralelly and produce a comined result for final output.

6. **Execution Time Tracking for Performance Analysis and Optimization**: Track total processing time equating to multiple permutations with changing degree of paralellism adjusted by number of threads available for processing.

# Design Consideration:
___
## Multithreading & Ansyncrhonous Processing for Image Pixel Value Analysis:

> In context of this project, where the goal is to identify the top 50 pixels by value alogn with it's location in a 2D grid representation of a raw image data, multi-threading and asynchronous processing plays a significant roal in improving the efficiency of the algorithm where data can be unbounded and CPU resource should be utilized to it's full potential to handle dynamic nature of input data.

### Multi-threding with Mutex:
___
* Multithreading is a technique that allows multiple threads to execute concurrently, potentially speeding up the processing of the task. 
* In this project, multithreading is employed to divide the task of analyzing the image grid among multiple threads, with each thread responsible for processing a portion of the image chunked out by available threads set with degree of parallelism.
* <ins>**Challenge with Multithreading**</ins>: When multi-threads across shares resources concurrently, such as the image grid data structure, it can lead to data race condition and inconsistencies in access input data as well as writting result.
* <ins>**Preventing Race Conditions**</ins>: To prevent issues with concurrency such as deadlocks, livelocks and race conditions, **mutextes(mutual exclusion)** is used. A mutex is a synchronization primitive that ensures only one thread can access a shared resource at a time. In this project, mutexes are utilized to protect critical sections of code where access to the image grid is required.
* <ins>**Advantage of using Multithreading with Mutex**</ins>: By leveraging multithreading with mutexes, the processing time of analyzing the image grid can be significantly reduced, as multiple threads can work concurrently without interfering with each other's access to shared resources.

### Asynchronous Processing with Futures:
___
* Asynchronous processing is another technique used to improve the efficiency of the algorithm by allowing tasks to execute concurrently without blocking the main thread of execution. In C++, asynchronous processing is often achieved using futures.
* <ins>**Using future for Async**</ins>: A future represents the result of an asynchronous computation that may not yet be available immediately. By launching asynchronous tasks using futures, the program can continue executing other tasks while waiting for the asynchronous computations to complete in the background.
* In the context of this project, asynchronous processing with futures can be employed to parallelize the analysis of the image represented as a 2D grid. 
* By launching asynchronous tasks to process different regions of the grid, the program can effectively utilize the available computational resources and minimize idle time.
* Furthermore, combining asynchronous processing with mutexes can ensure thread safety when accessing shared resources, such as the image grid, within the asynchronous tasks.

## Finding Top 50 Pixels by Value with MaxHeap:

> Max Heap - a binary tree-based data structure where each node has a value greater than or equal to the values of it's children. In the context of this project, the priority queue **'maxHeap'** is implemented using a max heap. Processing each grid pixel value, pixels with the heighest value (priority) will always be at the top of the heap, allowing for efficient retrieval of the top 50 pixels.

### Advantages Max Heap over Sorting:
* **Efficiency**: Maintaining a max heap allows for efficient retrieval of the top 50 pixels by value. The time complexity of `inserting` an element into a max heap is `O(logN)`, and the time complexity of `removing` the maximum element is also `O(logN)`, where N is number of elements in the heap. This is more efficient than `sorting entire data set`, which has a typical time complexity of `O(NlogN)`.
* **Incremental Updates**: With sorting requiring reordering of entire dataset every time new element is inserted or deleted, updating a max heap involves only adjusting the heap structure locally, making it more suitable for incremental updates in a multi-threading environment.
* **Memory Efficiency**: Sorting the entire data set requires addiitonal memory space to store the sorted result, whereas a max heap can be maintained in-place, requiring less memory overhead.

### Peformance Consideration in Concurrent Program:
* **Thread Safety**: Using mutext to ensure thread safety when accessing the shared 'maxHeap' in multi-threaded environment. However, locking and unlocking mutext involves overhead, potentially impacting performance, if higher number of threads are accessing same data structure for updates, which eventually involve more locking.
* **Optimization**: An alternative syncrhonization mechanism can be employed on top of maxHeap to further enhance the performance of the algorithm.
* **Data Distribution**: Peformance of the algorithm greatly depends upon distribution of pixel values across 2D grid, and whether single thread is capturing high pixel value cluster than other threads, which also leads to thread starvation if heap is locked for more time. Hence, algorithm will perform better or worse compared to uniformaly distrubuted pixel values.

# Exeuction Workflow & Performance Analaysis:

> In context of this project, feature flag is employed in configuration file to switch between using concurrency with mutex approach or asynchronous processing with future approach and performance for both these approaches vary based upon height, width and pixel value density distrution of the image.

### Assumptions:

* Degree of Parallelism is determined by the number of CPU cores available for parallel processing on host machine executing this program.
    ```C++
    int cores = std::thread::hardware_concurrency();
    ```
* Width (W) and Height (H) to generate imput raw image is not defined and so for the estimation is taken (800 x 800). However, users can set this value from <ins>config.json</ins> file without rebuilding entire application to test algorithm with different test data. It is assumed that user will provide valid user inputs for system to process.
* Identifying top 50 pixels are implemented using `maxHeap`, however performance of this approach greatly varies based upon height, width and pixel value density distrution of the image. A thorough analysis is needed adjusting these parameters to come up with the best performance of the algorithm.
* It is assumed that users will not need to extract generated image out and every time program executes, it will generate a new 2d grid image for processing. However, a method is impelmented which can be used to export generated 2d raw image data which can be used over and over again for multiple iterations instead of generating new data every single time.
* It is assumed that no specific order of grid values are exported if number of same value pixels exceed over 50. For example, if there are more than 50 pixels identified with heighest value, any random top 50 pixels will be returned as they're processed and merged parallely and key constraint for filtering is pixel value and not their grid location.

### Execution Workflow:

**Step 1. Determining CPU Cores:**
> Application starts by determining number of CPU cores available for parallel processing. This information is crucial for optimizing the use of parallelism and determine degree of parallelism.

**Step 2. Reading Configuration Parameters**
> Program reads configuration parameters such as image width, height, output file path, log file path, and whether to use asynchronous processing or multithreading using mutex approach to run the code from JSON file using custom utility which uses json parser. `config.json` added as follows:
```JSON
{
    "Width": 800,
    "Height": 800,
    "Output_file": "output.txt",
    "Log_file" : "log.txt",
    "IsUsingAsync" : true
}
```
**Step 3. Generating Image Data:**
> Custom Image Data is generated based on specified height and width where each pixel value is `16-bit word` where pixel values range from 0 to 65535.

**Step 4: Choosing Processing Mode:**
> Program dynamically determines the processing mode and depending upon choice there are two options to follow: Start execution with concurrent threads and mutex or use asynchronous processing using futures without blocking existing processes.

**Step 5: Iterating Over Thread Counts:**
> Program iterates over the specified number of threads, testing different configurations. It will eventually merge data from all the threads processing 2D grid concurrently.

**Step 6: Finding Top Pixels:**
> The top pixels are found either asynchronously or concurrently depending on the selection mode, If asyncrhonous mode is chosen, a maxHeap is generated merging results from all the parallel processes and finally generating a single list of top 50 pixels from the heap.

**Step 7: Measuring Execution Time:**
> Once the top pixels are found and results from multiple threads or parallel processes are merged, exectution time is calculated. This provides us an insight of time taken for algorithm to perform operation upon using different number of threads in each iterations.

**Step 8: Writting Log File:**
> The top pixels and their locations are written to a log file which is named as provided from the configuration file.

### Algorithm Walkthrough:

Explanation of algorithm utilizing Mutext for concurrency control to analyze input chunk of image pixels for thread processing the data. Step by step analysis of this algorithm is as follows:

1. **Input Parameters**:
    - `maxHeap`: A priority queue of pixels, sorted in descending order based on some criterion. This priority queue will hold the top pixels found so far and will be shared as a single copy across different threads.
    - `mtx`: A mutex, used for synchronization in multi-threaded enviornments to ensure that access to the 'maxHeap' is thread-safe.
    - `imageChunk`: A 2D vector representing a chunk of data with specific height and width depending upon chunking for parallel processing, presumably containing pixel values. 
    - `startRow`: The row index indicating the starting position of the chunk within the larger data set to store pixel index value in maxHeap.

2. **Iterating Thrtoug the Image Chunk**: The function iterates through each element of the 2D 'image chunk' of data from larger data set processed by specific thread, using nested loops performing grid traversal.

3. **Locking the Mutex**: Before accessing the shared `maxHeap` which is a shared data keeper across multiple threads, the function locks the mutex using `mtx.lock()`. This ensures that only one thread can access the `maxHap` at a time, preventing data corruption or race condition in multi-threaded environment.

4. **Pushing Pixels into the Max Heap**: For pixels in the input image chunk, the algorithm creates a `Pixel` object containing the pixel value and it's position (adjusted row index and column index). This `Pixel` object is then pushed onto the `maxHeap`.

5. **Maintaining the Max Heap Size**: After pushing a new pixel onto the `maxHeap`, the algorithm checks if the size of the `maxHeap` exceeds a pre-defined threshold (in this case 50). If the size exceeds the threshold, the algorithm removes the pixel with the lowest priority (value) from the `maxHeap` using `maxHeap.pop()`, This ensures that the `maxHeap` always contains the top 50 pixels.

6. **Unlocking the Mutes**: After updating the `maxHeap`, the function unlocks the mutex using `mtx.unlock()`, allowing other threads to access the `maxHeap`.

### Summary:
* Utilizing mutex to guard `maxHeap` while using 8 simultaneous threads can be a performance night-mare, where using a single thread with no mutex can very well produce better result in this case, which is very evident with this project's impelementation of concurrency using mutex where with increasing thread count, performance is actually going down.

*   However, `threads-by-future` is an approach which demands a close monitoring of controlling how many threads are getting created by applying `degree of parallelism` constraint. Otherwise, with continuous thread creation to process data asynchrounously will be very hardwear intensive. Key issues to handle while using `async` and `futures` are as follows:
    - Number of threads being created during the process.
    - Merging results from multiple threads to a single result.
    - How many async functions are being executed, as there could be a possibility that a single operation executed as async has further operations which are also getting executed asyn and this would create a snow-ball into very deep recursion of operations, which all would eventually fail if timed out.


### Future Enhancements:

* Add custom error codes for better exception handling upon calling this program from API endpoints. Better API documentation can be done with custom error codes to improve usability and accesibility.
* Improve exception handling and edge case analysis for crash cases in multi-threading environment.
* Add better visualization by adding more data points to analyze performance metrics of the algorithm upon using large raw image data to process with varied CPU cores available to improve degree of parallelism.
* Add implementaiton which uses `std::semaphores` or `std::counting_semaphore` to compare results with using `std::mutex`, as mutex locks the resource while one thread is accessing it, adding guard on a variable with mutex, then attempting to work with it 100% of the time with 8 threads simultaneously will result to degraded performance over using a single thread with no mutex. 
* Implement `thread pooling` to share threads and reduces overhead of creating and destroying threads while accessing large data records.
