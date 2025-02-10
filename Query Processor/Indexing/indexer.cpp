#include "indexer.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <list>

// Indexer function to build an inverted index
void indexer(std::unordered_map<std::string, std::list<std::pair<int, int>>> *Index, std::vector<std::string> words, int docID) {
    for (std::string& word : words) {
        // Convert each character in the word to lowercase
        for (char& term : word) {
            term = std::tolower(term);
        }

        // Create or retrieve the entry for the word in the index
        auto& wordEntry = (*Index)[word];
        bool found = false;

        // Check if the document docID already exists in the word's entry and update its frequency
        for (auto& pair : wordEntry) {
            if (pair.first == docID) {
                pair.second++; // Increment the frequency
                found = true;
                break;
            }
        }

        // If the document docID doesn't exist for the word, add a new entry with frequency 1
        if (!found) {
            wordEntry.emplace_back(docID, 1);
        }
    }
}

// Function to save the index to a file
void saveIndex(std::string counter, std::unordered_map<std::string, std::list<std::pair<int, int>>> *Index) {
    std::string fileName = "/Users/nikhilsoni/Desktop/big_test/index_test/subinverted/" + counter + ".txt";

    std::ofstream file(fileName, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Error2: Unable to open file " << fileName << " to write." << std::endl;
        return;
    }

    // Loop through the map and save its content to the file
    for (const auto& pair : *Index) {
        file << pair.first << ":";
        for (const auto& entry : pair.second) {
            file << entry.first << "," << entry.second << " ";
        }
        file << "\n";
    }

    // Close the file
    file.close();
    (*Index).clear(); // Clear the index after saving
}

// Function to create a page table entry
void createPageTable(int docID, std::string url, std::streampos docPos, int word_count) {
    // Define the file path for the page table
    
    std::string filePath = "/Users/nikhilsoni/Desktop/big_test/index_test/output/PageTable.txt";

    // Open the file in append mode for writing
    std::ofstream file(filePath, std::ios::app);
    
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file PageTable.txt for writing." << std::endl;
        return;
    }

    // Write the page table entry in the format "key: value"
    file << docID << ":" << word_count << " " << docPos << " " << url << '\n';
   
    // Close the file
    file.close();
}

// Function to sanitize and clean text
std::string sanitizeText(const std::string &line) {
    std::string result;

    // Iterate through each character in the input line
    for (char c : line) {
        // Check if the character is alphabetic or a space
        if (std::isalpha(c) || std::isspace(c)) {
            result += c; // Add the character to the result if it's valid
        }
        
        else{
            result += " ";
        }
    }

    return result; // Return the sanitized text
}
