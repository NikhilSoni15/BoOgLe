#pragma once
#include <string>
#include <sys/resource.h>
#include <unordered_map>
#include <vector>

int readTRECFile(std::string fileName, std::unordered_map<int, std::string> *DocID);
void mergeFiles(const std::vector<std::string> &filenames, const std::string &outputFilename);
void mergeSort(std::string filename1, std::string filename2, std::string outputfilename);
int batchedMergeSort(int batchSize, int totalFiles, std::string openFrom, std::string writeTo);
void compress(std::string);
long getCurrentMemoryUsageMB();

