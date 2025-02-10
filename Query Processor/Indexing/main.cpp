#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sys/resource.h>
#include "indexer.h"
#include "file_operations.h"

int main() {
    std::string fileName = "/Users/nikhilsoni/Desktop/big_test/index_test/test_trec.txt";

    // Read and process the TREC file, extracting document content and URLs
    int totalFiles = readTRECFile(fileName);
    std::cout << "TREC File reading complete" << std::endl;
    
    // Define the batch size for processing documents in batches
    int batchSize = 0;

    // Set the batch size to 100 if there are 100 or more total files, otherwise use the total number of files
    if (totalFiles >= 100) {
        batchSize = 100; // taking batch size of 100
    } else {
        batchSize = totalFiles;
    }

    // Create a vector to store file names
    std::vector<std::string> filenames;

    // Initialize an iteration count for naming intermediate files
    int iterationCount = 0;
    std::string outputFilepath;

    // Iterate through the documents, processing them in batches
    for (int batchStart = 1; batchStart <= totalFiles; batchStart += batchSize) {
        std::vector<std::string> fileBatch;

        // Collect file names for the current batch
        for (int i = batchStart; i < batchStart + batchSize && i <= totalFiles; i++) {
            fileBatch.push_back("/Users/nikhilsoni/Desktop/big_test/index_test/subinverted/" + std::to_string(i) + ".txt");
        }

        // Define the intermediate file path for merging the current batch
        std::string intermediateFile = "/Users/nikhilsoni/Desktop/big_test/index_test/merged/" + std::to_string(iterationCount) + ".txt";

        // Merge the files in the current batch and increment the iteration count
        mergeFiles(fileBatch, intermediateFile);
        iterationCount++;
        std::cout << "File Merged!" << std::endl;
    }

    // Store the final iteration count in a variable
    int finalList = iterationCount;
    bool value = false;

    // Continue batched merge sorting until a single merged file is left
    while (true) {
        if (value) {
            // Perform batched merge sorting and check if a single file is left
            finalList = batchedMergeSort(2, finalList, "/Users/nikhilsoni/Desktop/big_test/index_test/subinverted/", "/Users/nikhilsoni/Desktop/big_test/index_test/merged/");
            if (finalList == 1) {
                // If only one merged file is left, mark the merge process as complete
                std::cout << "File merge complete2" << std::endl;
                // Set the output folder to the merged folder
                outputFilepath = "/Users/nikhilsoni/Desktop/big_test/index_test/merged/";
                break;
            }
            value = false;
        }
        else {
            // Perform batched merge sorting and check if a single file is left
            finalList = batchedMergeSort(2, finalList, "/Users/nikhilsoni/Desktop/big_test/index_test/merged/", "/Users/nikhilsoni/Desktop/big_test/index_test/subinverted/");
            if (finalList == 1) {
                // If only one merged file is left, mark the merge process as complete
                std::cout << "File merge complete1" << std::endl;
                // Set the output folder to the subinvertedindx folder
                outputFilepath = "/Users/nikhilsoni/Desktop/big_test/index_test/subinverted/";
                break;
            }
            value = true;
        }
    }

    // Construct the final output filename based on the output folder
    std::string finalOutputFilename = outputFilepath + "0.txt";

    // Initiate the compression process on the final output file
    std::cout << "Starting file compression" << std::endl;
    compressIndex(finalOutputFilename);
    std::cout << "File compressed" << std::endl;

    return 0;
}
