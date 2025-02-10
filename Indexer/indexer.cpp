#include "indexer.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <list>

// Indexer function to build an inverted index
void indexer(std::unordered_map<std::string, std::list<std::pair<int, int>>> *Index, std::vector<std::string> words, int id) {
    for (std::string& word : words) {
        // Convert each character in the word to lowercase
        for (char& term : word) {
            term = std::tolower(term);
        }

        // Create or retrieve the entry for the word in the index
        auto& wordEntry = (*Index)[word];
        bool found = false;

        // Check if the document ID already exists in the word's entry and update its frequency
        for (auto& pair : wordEntry) {
            if (pair.first == id) {
                pair.second++; // Increment the frequency
                found = true;
                break;
            }
        }

        // If the document ID doesn't exist for the word, add a new entry with frequency 1
        if (!found) {
            wordEntry.emplace_back(id, 1);
        }
    }
}

// Function to save the index to a file
void saveIndex(std::string name, std::unordered_map<std::string, std::list<std::pair<int, int>>> *Index) {
    std::string fileName = "/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/subinvertedindx/" + name + ".txt";

    std::ofstream file(fileName, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Error2: Unable to open file " << fileName << " to write." << std::endl;
        return;
    }

    // Loop through the map and save its content to the file
    for (const auto& pair : *Index) {
        file << pair.first << ":";
        for (const auto& entry : pair.second) {
            file << entry.first << "," << entry.second << "  ";
        }
        file << "\n";
    }

    // Close the file
    file.close();
    (*Index).clear(); // Clear the index after saving
}
