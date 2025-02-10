#include "file_operations.h"
#include "indexer.h"
#include "text_processing.h"
#include "page_table.h"
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <sys/resource.h>

// Function to read a TREC file, process its content, and build an inverted index.
int readTRECFile(std::string fileName, std::unordered_map<int, std::string> *DocID) {
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
    
    while (std::getline(file, line)){
        
        // Check memory usage periodically
        currentMemoryUsageMB = getCurrentMemoryUsageMB();
        if (currentMemoryUsageMB >= MEMORY_THRESHOLD_MB) {
            saveIndex(std::to_string(counter), &Index);
            // Reset memory usage tracker
            currentMemoryUsageMB = 0;
        }
        
        
        if (line.find("</TEXT>") != std::string::npos){
            text = false;
            if (docID % 1000 == 0){
                std::cout << "Doc:"<< counter <<"Ready! " << std::endl;
                counter++;
                saveIndex(std::to_string(counter), &Index);
            }
        }

        if (text){
            std::vector<std::string> words;
            line = sanitizeText(line);
            std::stringstream iss(line);
            std::string word;

            while (iss >> word){
                words.push_back(word);
            }
            indexer(&Index, words, docID);
        }

        if (url){
            createPageTable(DocID, docID, line);
            url = false;
            text = true;
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
        outputFile << pair.first << ": ";
        for (const std::string &value : pair.second)
        {
            outputFile << value << " ";
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
    while (std::getline(file1, line1) && (std::getline(file2, line2)))
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
            outputFile << key1 << ": "<< value1 << "\n";
            outputFile << key2 << ": "<< value2 << "\n";

        }
        else if (comparisonResult > 0)
        {
            outputFile << key2 << ": "<< value2 << "\n";
            outputFile << key1 << ": "<< value1 << "\n";
        }
        else
        {
             outputFile << key1 << ": "<< value1 << value2 << "\n";
        }
    }
    while (std::getline(file1, line1)){
        size_t colonPos1 = line1.find(':');
        std::string key1 = line1.substr(0, colonPos1);
        std::string value1 = line1.substr(colonPos1 + 1);
        outputFile << key1 << ": "<< value1 << "\n";
    }
    
    while (std::getline(file2, line2)){
        size_t colonPos2 = line2.find(':');
        std::string key2 = line2.substr(0, colonPos2);
        std::string value2 = line2.substr(colonPos2 + 1);
        outputFile << key2 << ": "<< value2 << "\n";
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
    }

    
    return iterationCount;

}

// Function to encode an integer value using variable-byte (VByte) encoding.
std::vector<unsigned char> encodeVByte(uint32_t value)
{
    std::vector<unsigned char> bytes;

    do
    {
        unsigned char byte = value & 0x7F;
        value >>= 7;
        if (value > 0)
        {
            byte |= 0x80;
        }
        bytes.push_back(byte);
    } while (value > 0);

    return bytes;
}

// Function to compress the final index data and create two output files.
void compress(std::string outputFilename){

    std::ifstream file(outputFilename);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open your file "
                  << "fileName" << std::endl;
        return ;
    }
    std::ofstream outFile("/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/output/final_compressed_index.bin", std::ios::binary);
    std::ofstream outFileLexicon("/Users/nikhilsoni/Desktop/inverted_index/inverted_indexer/inverted_indexer/output/lexicon.txt", std::ios::app);
    
    std::string line;
    std::stringstream compressedString;
    std::vector<std::string> compressData;
    int lineNumber = 0;
    int freqDoc = 0;
    int wordId = 0;
    while (std::getline(file, line))
    {
        lineNumber++;
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            wordId++;
            std::string value = line.substr(colonPos + 1, line.length() + 1);

            int docID = 0;
            int frequecy;
            std::string result;
            for (char c : line)
            {
                if (std::isdigit(c))
                {
                    result += c;
                }
                else if (c == ',') //seperator of docID and Freq
                {
                    if (result.length())
                    {
                        docID = std::stoi(result);
                        freqDoc++;
                        result.clear();
                    }
                }
                else
                {
                    if (result.length())
                    {
                        frequecy = std::stoi(result);
                        result.clear();
                        std::vector<unsigned char> compressedBytes_frequency = encodeVByte(frequecy);
                        std::vector<unsigned char> compressedBytes_docID = encodeVByte(docID);
                        for (size_t i = 0; i < compressedBytes_docID.size(); ++i)
                        {
                            compressedString << compressedBytes_docID[i];
                        }
                        compressedString << ",";
                        for (size_t i = 0; i < compressedBytes_frequency.size(); ++i)
                        {
                            compressedString << compressedBytes_frequency[i];
                        }
                        compressedString << "  ";
                    }
                }
            }

            outFile << compressedString.str() << "\n";
            compressedString.str(std::string());
            outFileLexicon << key << " : "  << wordId << " " << freqDoc << " " << lineNumber<< std:: endl;
            freqDoc = 0;
            std::cout << "Processed line " << lineNumber << std::endl;
        }
    }
    outFile.close();
    file.close();
    outFileLexicon.close();
    return ;
    
}

// Function to get the current memory usage in megabytes.
long getCurrentMemoryUsageMB() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // Convert kilobytes to megabytes.
        long residentSetSizeKB = usage.ru_maxrss;
        long residentSetSizeMB = residentSetSizeKB / 1024;
        return residentSetSizeMB;
    } else {
        // Return -1 to indicate an error.
        return -1;
    }
}
