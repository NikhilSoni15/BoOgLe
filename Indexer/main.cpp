#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sys/resource.h>
#include "indexer.h"
#include "page_table.h"
#include "text_processing.h"
#include "file_operations.h"

int main() {
    std::string fileName = "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/docs.trec";
    std::unordered_map<int, std::string> DocID;

    // Read and process the TREC file, extracting document content and URLs
    int totalFiles = readTRECFile(fileName, &DocID);
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
    std::string outputFilename;

    // Iterate through the documents, processing them in batches
    for (int batchStart = 1; batchStart <= totalFiles; batchStart += batchSize) {
        std::vector<std::string> fileBatch;

        // Collect file names for the current batch
        for (int i = batchStart; i < batchStart + batchSize && i <= totalFiles; i++) {
            fileBatch.push_back("/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/subinvertedindx/" + std::to_string(i) + ".txt");
        }

        // Define the intermediate file path for merging the current batch
        std::string intermediateFile = "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/merged/" + std::to_string(iterationCount) + ".txt";

        // Merge the files in the current batch and increment the iteration count
        mergeFiles(fileBatch, intermediateFile);
        iterationCount++;
    }

    // Store the final iteration count in a variable
    int finalList = iterationCount;
    bool value = false;

    // Continue batched merge sorting until a single merged file is left
    while (true) {
        if (value) {
            // Perform batched merge sorting and check if a single file is left
            finalList = batchedMergeSort(2, finalList, "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/subinvertedindx/", "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/merged/");
            if (finalList == 1) {
                // If only one merged file is left, mark the merge process as complete
                std::cout << "File merge complete" << std::endl;
                // Set the output folder to the merged folder
                outputFilename = "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/merged/";
                break;
            }
            value = false;
        } else {
            // Perform batched merge sorting and check if a single file is left
            finalList = batchedMergeSort(2, finalList, "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/merged/", "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/subinvertedindx/");
            if (finalList == 1) {
                // If only one merged file is left, mark the merge process as complete
                std::cout << "File merge complete" << std::endl;
                // Set the output folder to the subinvertedindx folder
                outputFilename = "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/subinvertedindx/";
                break;
            }
            value = true;
        }
    }

    // Construct the final output filename based on the output folder
    std::string finalOutputFilename = outputFilename + "0.txt";

    // Initiate the compression process on the final output file
    std::cout << "Starting file compression" << std::endl;
    compress(finalOutputFilename);
    std::cout << "File compressed" << std::endl;

    return 0;
}
