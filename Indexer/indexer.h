#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <vector>

void indexer(std::unordered_map<std::string, std::list<std::pair<int, int>>> *Index, std::vector<std::string> words, int id);
void saveIndex(std::string name, std::unordered_map<std::string, std::list<std::pair<int, int>>> *Index);
