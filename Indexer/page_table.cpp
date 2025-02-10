#include "page_table.h"
#include <iostream>
#include <fstream>

// Function to create a page table entry
void createPageTable(std::unordered_map<int, std::string>* DocID, int key, std::string value) {
    // Define the file path for the page table
    std::string filePath = "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/output/PageTable.txt";

    // Open the file in append mode for writing
    std::ofstream file(filePath, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file PageTable.txt for writing." << std::endl;
        return;
    }

    // Write the page table entry in the format "key: value"
    file << key << ": " << value << std::endl;

    // Close the file
    file.close();
}
