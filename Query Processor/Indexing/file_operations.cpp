#include "file_operations.h"
#include "indexer.h"
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <sys/resource.h>

// Function to read a TREC file, process its content, and build an inverted index.
int readTRECFile(std::string fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()){
        std::cerr << "Error1: Unable to open file " << fileName << std::endl;
        return 0;
    }
    
    int docID = 0;
    int counter = 0;
    std::string line;
    bool url = false;
    bool text = false;
    std::unordered_map<std::string, std::list<std::pair<int, int> >> Index;
    const long MEMORY_THRESHOLD_MB = 1024;
    long currentMemoryUsageMB;
    std::streampos docStartPos;
    std::string URL;
    int word_count = 0;
    size_t start = 0;
    
    while (std::getline(file, line)){
        
        // Check memory usage periodically
        currentMemoryUsageMB = getCurrentMemoryUsageMB();
        if (currentMemoryUsageMB >= MEMORY_THRESHOLD_MB) {
            saveIndex(std::to_string(counter), &Index);
            // Reset memory usage tracker
            currentMemoryUsageMB = 0;
            counter++;
        }
        
        
        if (line.find("</TEXT>") != std::string::npos){
            
            createPageTable(docID, URL, docStartPos, word_count);
            word_count = 0;
            URL.clear();
            
            text = false;
            if (docID % 1000 == 0){
                std::cout << "Batch:"<< counter <<"Ready! " << std::endl;
                counter++;
                saveIndex(std::to_string(counter), &Index);
            }
        }

        if (text){
            std::vector<std::string> words;
            line = sanitizeText(line);
            std::stringstream wordStream(line);
            std::string word;

            while (wordStream >> word){
                word_count++;
                words.push_back(word);
            }
            indexer(&Index, words, docID);
        }

        if (url){
            
            URL = line;
            url = false;
            text = true;
            docStartPos = file.tellg();
        }

        if (line.find("<TEXT>") != std::string::npos){
            url = true;
            docID++;
            
        }
    }
    counter++;
    saveIndex(std::to_string(counter), &Index);

    file.close();
    return counter;
}

// Function to merge data from multiple files into a single output file.
void mergeFiles(const std::vector<std::string> &filenames, const std::string &outputFilename) {
    std::map< std::string, std::vector<std::string>> mergedData;

    for (const std::string &filename : filenames)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Unable to open file " << filename << std::endl;
            continue;
        }

        std::string line;
        while (std::getline(file, line))
        {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);

                mergedData[key].push_back(value);
            }
        }
        file.close();
        remove(filename.c_str());
    }

    std::ofstream outputFile(outputFilename);
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Unable to open output file " << outputFilename << std::endl;
        return;
    }

    for (const auto &pair : mergedData)
    {
        outputFile << pair.first << ":";
        for (const std::string &elements : pair.second)
        {
            outputFile << elements;
        }
        outputFile << std::endl;
    }

    outputFile.close();
}

// Function to perform merge sort on two input files and create an output file.
void mergeSort(std::string filename1, std::string filename2, std::string outputfilename) {
    std::ifstream file1(filename1);
    std::ifstream file2(filename2);

    std::ofstream outputFile(outputfilename);
    std::string line1, line2;
    bool fileTrue1 = true, fileTrue2 = true;
    bool flap1 = false, flap2 = false;
    while (true)
    {
        if (fileTrue1)
        {
            flap1 = (bool)std::getline(file1, line1);
            fileTrue1 = false;
        }

        if (fileTrue2)
        {
            flap2 = (bool)std::getline(file2, line2);
            fileTrue2 = false;
        }

        if (!flap1 && !flap2)
        {
            break;
        }
        else if (flap1 && flap2)
        {
            size_t colonPos1 = line1.find(':');
            std::string key1 = line1.substr(0, colonPos1);
            std::string value1 = line1.substr(colonPos1 + 1);

            size_t colonPos2 = line2.find(':');
            std::string key2 = line2.substr(0, colonPos2);
            std::string value2 = line2.substr(colonPos2 + 1);

            int comparisonResult = key1.compare(key2);

            if (comparisonResult < 0)
            {
                outputFile << key1 << ":" << value1 << "\n";
                fileTrue1 = true;
            }
            else if (comparisonResult > 0)
            {
                outputFile << key2 << ":" << value2 << "\n";
                fileTrue2 = true;
            }
            else
            {
                outputFile << key1 << ":" << value1 << value2 << "\n";
                fileTrue1 = true;
                fileTrue2 = true;
            }
           
        }
        else if(flap1 && !flap2){

            size_t colonPos1 = line1.find(':');
            std::string key1 = line1.substr(0, colonPos1);
            std::string value1 = line1.substr(colonPos1 + 1);
            outputFile << key1 << ":" << value1 << "\n";
            fileTrue1 = true;
        }
        else if(!flap1 && flap2){
            size_t colonPos2 = line2.find(':');
            std::string key2 = line2.substr(0, colonPos2);
            std::string value2 = line2.substr(colonPos2 + 1);
            outputFile << key2 << ":" << value2 << "\n";
            fileTrue2 = true;
        }

    }
    file1.close();
    file2.close();
    outputFile.close();
    remove(filename1.c_str());
    remove(filename2.c_str());
}


// Function to perform batched merge sort on a set of files.
int batchedMergeSort(int batchSize, int totalFiles, std::string openFrom, std::string writeTo) {
    int iterationCount = 0;
    for (int batchStart = 0; batchStart < totalFiles; batchStart += batchSize)
    {
        std::vector<std::string> fileBatch;

        for (int i = batchStart; i < batchStart + batchSize && i <= totalFiles; i++)
        {
            fileBatch.push_back(openFrom + std::to_string(i) + ".txt");
        }

        std::string intermediateFile = writeTo + std::to_string(iterationCount) + ".txt";
        mergeSort(fileBatch[0], fileBatch[1], intermediateFile);
        iterationCount++;
        std::cout << "MergeSort!" << std::endl;
    }

    
    return iterationCount;

}


// Function to encode a number using Variable Byte Compression
std::vector<unsigned char> encodeVarByte(int num) {
    std::vector<unsigned char> result;
    while (num > 127) {
        result.push_back(static_cast<unsigned char>(num & 127));
        num = num >> 7;
    }
    result.push_back(static_cast<unsigned char>(num + 128));
    return result;
}

int compressIndex(std::string address) {
    
    // Open the inverted index file for reading
    std::ifstream infile(address);
    int lineCount = 0;
    if (!infile) {
        std::cerr << "Failed to open the inverted index file." << std::endl;
        return 1;
    }

    // Open the compressed index file for writing as a binary file
    std::ofstream compressedFile("/Users/nikhilsoni/Desktop/big_test/final/output/compressed_index.bin", std::ios::binary);
    if (!compressedFile) {
        std::cerr << "Failed to open the compressed index file for writing." << std::endl;
        return 1;
    }

    
    // Open the lexicon file for writing
    std::ofstream lexiconFile("/Users/nikhilsoni/Desktop/big_test/final/output/lexicon.txt");
    if (!lexiconFile) {
        std::cerr << "Failed to open the lexicon file for writing." << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(infile, line)) {
        size_t pos = line.find(":");
        if (pos == std::string::npos) {
            std::cerr << "Invalid line format in the inverted index." << std::endl;
            continue;
        }

        std::string word = line.substr(0, pos);
        std::string postingList = line.substr(pos + 1);
        
        
        
        if (word.length() > 30) {
                // Word is too long, skip to the next line
                continue;
            }
    
        
        

        int docFreq = 1;
        for (char c : postingList) {
            if (c == ' ')
                docFreq++;
        }

        long long startPos = compressedFile.tellp();

        std::vector<int> docIDs;
        std::vector<int> freqs;

        std::stringstream ss(postingList);
        std::string docFreqPair;
        while (ss >> docFreqPair) {
            int docID, freq;
            if (sscanf(docFreqPair.c_str(), "%d,%d", &docID, &freq) == 2) {
                docIDs.push_back(docID);
                freqs.push_back(freq);
            }
        }

        // Encode and write the compressed posting list to the binary file in chunks
        const int chunkSize = 10;
        for (size_t i = 0; i < docIDs.size(); i += chunkSize) {
            for (int j = 0; j < chunkSize && i + j < docIDs.size(); j++) {
                std::vector<unsigned char> encodedDocID = encodeVarByte(docIDs[i + j]);
                compressedFile.write(reinterpret_cast<const char*>(&encodedDocID[0]), encodedDocID.size());
                
            }
            
            for (int j = 0; j < chunkSize && i + j < freqs.size(); j++) {
                std::vector<unsigned char> encodedFreq = encodeVarByte(freqs[i + j]);
                compressedFile.write(reinterpret_cast<const char*>(&encodedFreq[0]), encodedFreq.size());
                
            }
            
        }

        lineCount++;
        
        long long endPos = compressedFile.tellp();
        std::cout << "Line Compressed" << lineCount << std::endl;
        // Write the lexicon data to the lexicon file for each word
        lexiconFile << word << " " << docFreq << " " << startPos << " " << endPos << " " << docIDs.size() + freqs.size() << "\n";
    }

    // Close the input and output files
    infile.close();
    compressedFile.close();
    lexiconFile.close();
    

    std::cout << "Inverted index has been compressed in chunks and saved to compressed_index.bin" << std::endl;
    std::cout << "Lexicon data has been saved to lexicon.txt" << std::endl;

    return 0;
}
