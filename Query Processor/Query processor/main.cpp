#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cmath>
#include <unordered_set>

// Struct to represent information about each term in the lexicon
struct LexiconEntry {
    size_t DocFreq;        // Document frequency of the term
    size_t StartPos;       // Start position of the term in the binary file
    size_t EndPos;         // End position of the term in the binary file
    size_t TotalPostings;  // Total postings for the term
};

// Struct to represent information about each document in the page table
struct PageInfo {
    int docLength;         // Length of the document
    size_t startPosition;  // Start position of the document in the Trec file
    std::string url;       // URL of the document
};

// Function to sanitize text by removing non-alphabetic characters
std::string sanitizeText(const std::string &line) {
    std::string result;

    // Iterate through each character in the input line
    for (char c : line) {
        // Check if the character is alphabetic or a space
        if (std::isalpha(c) or std::isspace(c)) {
            result += std::tolower(c); // Add the character to the result if it's valid
        }
        
        else{
            result += " ";
        }
    }

    return result; // Return the sanitized text
}

// Function to decode a term and retrieve its postings list
std::vector<size_t> decodeWord(std::string word,std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& Binarybuffer){
    
    auto item = lexiconMap.find(word);
    
    if (item == lexiconMap.end())
        return std::vector<size_t>();
    
    // Retrieve information about the term
    size_t start = item->second.StartPos;
    size_t end = item->second.EndPos;
    size_t total = item->second.TotalPostings;
    
    // Seek to the start position in the binary file
    Binarybuffer.seekg(start);
    
    std::vector<size_t> list;
    list.push_back(total);
    
    int val=0, shift=0;
    for(size_t i=start; i < end; i++){
        int binaryChar = Binarybuffer.get();
        if (binaryChar < 128) {
            val = val + (binaryChar << shift);
            shift = shift + 7;
        }
        else {
            val = val + ((binaryChar-128) << shift);
            list.push_back(val);
            val=0; shift=0;
        }
    }
    return list;
}

// Function to retrieve document IDs for a term
std::vector<size_t> docIDsByWord(std::string word,std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& Binarybuffer){
    
        std::vector<size_t> invList;
        invList = decodeWord(word, lexiconMap,Binarybuffer);
    
    if (invList.empty()) {
           
           return std::vector<size_t>(); // Returning an empty vector in this case
       }
    
        std::vector<size_t> docIDs;
        size_t totalnums = invList[0];
        auto lim = totalnums - (totalnums % 20) ;
    
    if (totalnums > 20){
        
        for (int i = 0; i < lim ; ++i) {
            if (i % 20 < 10){
                docIDs.push_back(invList[i+1]);
            }
            
            else{
                i = i + 10;
            }
        }
        for (int i = 0; i < ((totalnums-lim)/2); ++i) {
            docIDs.push_back(invList[lim +1 + i]);
        }
    }
    
    else{
        for (int i = 0; i < (totalnums/2); ++i) {
            docIDs.push_back(invList[1 + i]);
        }
    }
    return docIDs;
}

// Function to retrieve term frequency for a term and a specific document
size_t giveFreq(std::string word , int targetID,std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& Binarybuffer){
    
    std::vector<size_t> invList;
    invList = decodeWord(word, lexiconMap, Binarybuffer);
    
    // If the postings list is empty, return zero
    if (invList.empty()) {
           return 0;
       }

    size_t freq = 0;
    size_t totalnums = invList[0];

    auto lim = totalnums - (totalnums % 20) ;

if (totalnums > 20){
    
    for (int i = 0; i < lim ; ++i) {
        if (i % 20 < 10){
            if(invList[i+1] == targetID){
                freq = invList[i+1 + 10];
                break;
            }
        }
        
        else{
            i = i + 10;
        }
    }
    for (int i = 0; i < ((totalnums-lim)/2); ++i) {
        freq = invList[lim + 1 + i + ((totalnums-lim)/2)];
    }
}

else{
    for (int i = 0; i < (totalnums/2); ++i) {
        
        if (invList[1 + i] == targetID){
            freq = invList[1 + i + totalnums/2];
            break;
        }
    }
    
}

return freq;
}

// Function to check if a document ID is in a list of document IDs
bool docidInList(const std::vector<size_t>& docIDs, size_t targetDocID) {
    
    for (auto docID : docIDs) {
        if (docID == targetDocID) {
            return true; // Found the targetDocID in the vector
        }
    }
    return false; // TargetDocID not found in the vector
}


// BM25 ranking function for a term and a specific document
double BM25(std::string word, int targetDocID, std::unordered_map<int, PageInfo>& pageTable,std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& Binarybuffer){
    
    size_t termFrequency;
    size_t documentLength = 0;
    size_t documentFrequency = 0;
    size_t totalDocuments = 3213835;
    size_t averageDocumentLength = 100;
    double k1 = 1.2;
    double b = 0.75;
    std::vector<size_t> docids;
    std::vector<size_t> list;
    
    // Retrieve document information from the page table
    if (pageTable.find(targetDocID) != pageTable.end()) {
            PageInfo info = pageTable[targetDocID];
        documentLength = info.docLength;
            
        } else {
            return 0.0000;
        }
    
    // Retrieve term information from the lexicon
    if (lexiconMap.find(word) != lexiconMap.end()) {
        LexiconEntry entry = lexiconMap[word];
        documentFrequency = entry.DocFreq;
            
        } else {
            return 0.0000;
        }
    
    // Retrieve document IDs for the term
    docids = docIDsByWord(word, lexiconMap,Binarybuffer);
    bool found = docidInList(docids, targetDocID);

    if (found){
     
        list = docids;
        
    }
    
    else{
        return 0.0000;
    }
    
    termFrequency = giveFreq(word, targetDocID, lexiconMap,Binarybuffer);
    
    // Calculate BM25 score components
    double idf = log10((totalDocuments - documentFrequency + 0.5) / (documentFrequency + 0.5));
    double term1 = ((k1 + 1.0) * termFrequency) / (k1 * ((1.0 - b) + b * (documentLength / averageDocumentLength)) + termFrequency);
    
   
    return  idf * term1;
    
    
}

// Function to calculate BM25 scores for a list of words in a specific document
double BM25forWords(int targetDocID,std::vector<std::string> words, std::unordered_map<int, PageInfo>& pageTable,std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& Binarybuffer){
    
    double sum = 0;
    for (const std::string& word : words) {
        
        sum = sum + BM25(word, targetDocID, pageTable, lexiconMap, Binarybuffer);
        
        }
    
    return sum;
}

// Function to combine BM25 scores for a list of documents and words
std::vector<double> BM25Combine(std::vector<int> targetDocIDs,std::vector<std::string> words, std::unordered_map<int, PageInfo>& pageTable, std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& Binarybuffer){
    
    std::vector<double> scores;
    // Calculate BM25 scores for each document and accumulate the scores
    for (const int& targetDocID : targetDocIDs) {
        
        scores.push_back(BM25forWords(targetDocID, words, pageTable, lexiconMap, Binarybuffer));
        
        }
    
    return scores;
}


// Function to generate a snippet for a given word
std::string generateSnippet(const std::string& content, const std::string& targetWord) {
    
    
    std::istringstream contentStream(content);
        std::vector<std::string> wordsInContent;

        std::string line;
        while (std::getline(contentStream, line)) {
            // Sanitize each line using the provided function
            std::string sanitizedLine = sanitizeText(line);

            // Split the sanitized line into words
            std::istringstream lineStream(sanitizedLine);
            std::vector<std::string> wordsInLine{
                std::istream_iterator<std::string>(lineStream),
                std::istream_iterator<std::string>()
            };

            // Append words in the line to the overall list of words
            wordsInContent.insert(wordsInContent.end(), wordsInLine.begin(), wordsInLine.end());
        }

        // Find the index of the target word in the content
        auto wordIndex = std::find(wordsInContent.begin(), wordsInContent.end(), targetWord);

        std::string snippet;

        if (wordIndex != wordsInContent.end()) {
            size_t startIndex = std::distance(wordsInContent.begin(), wordIndex);

            // Calculate start and end indices within a valid range
            size_t snippetStart = (startIndex > 10) ? (startIndex - 10) : 0;
            size_t snippetEnd = std::min(startIndex + 11, wordsInContent.size());

            // Extract the snippet based on calculated indices
            for (size_t j = snippetStart; j < snippetEnd; ++j) {
                snippet += wordsInContent[j] + " ";
            }
            snippet = snippet.substr(0, snippet.size() - 1);  // Remove the trailing space
        }

        return snippet;

    }


// Function to retrieve the document text for a given document ID and target word
std::string getDocumentText(int targetDocID, std::string targetWord, std::unordered_map<int, PageInfo>& pageTable, std::ifstream& inputFile ){
    
    size_t startPos = 0;
    std::string content;
    
    // Retrieve document information from the page table
    if (pageTable.find(targetDocID) != pageTable.end()) {
            PageInfo info = pageTable[targetDocID];
        
        startPos = info.startPosition;
            
        } else {
            std::cerr << "DocId " << targetDocID << " not found in the table." << std::endl;
        }
    
    inputFile.seekg(startPos);

        // Check if the seek operation was successful
        if (!inputFile) {
            std::cerr << "Seek operation failed" << std::endl;
        }

        // Read the file from the seeked position until "</TEXT>" is found
        std::string line;
        while (std::getline(inputFile, line)) {
            
            if (line.find("</TEXT>") != std::string::npos) {
                break;
            }
            content += line; // Concatenate the current line to the content string
            content += '\n';
        }
    
    // Generate a snippet for the target word within the document content
    std::string snippet = generateSnippet(content, targetWord);
    
    return snippet;
    
    }
    
// Function to perform conjunctive retrieval
std::vector<int> conjunctive(std::vector<std::string> queryWords,std::unordered_map<int, PageInfo>& pageTable, std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& Binarybuffer){
        
        std::vector<std::vector<int>> allDocIDs;
        std::vector<double> scores;

            for (const std::string& word : queryWords) {
                // Use docIDsByWord to get the document IDs for the current word
                std::vector<size_t> docIDs = docIDsByWord(word, lexiconMap, Binarybuffer);
                size_t numCols = docIDs.size();

                std::vector<int> row(numCols);

                for (size_t i = 0; i < numCols; ++i) {
                    row[i] = static_cast<int>(docIDs[i]);
                }

                allDocIDs.push_back(row);
            }
        
    std::vector<int> found;
    
    for (int x : allDocIDs[0]) {
            bool isFound = true;
            for (int y = 1; y < allDocIDs.size(); ++y) {
                if (std::find(allDocIDs[y].begin(), allDocIDs[y].end(), x) == allDocIDs[y].end()) {
                    isFound = false;
                    break;
                }
            }
            if (isFound) {
                found.push_back(x);
            }
        }
    
    if (found.empty()){
        
        return std::vector<int>();
    }
    
    
    // Getting the scores
    scores = BM25Combine(found, queryWords, pageTable,lexiconMap, Binarybuffer);
    
    
    
    // Combine the DocIDs and scores into a pair vector for sorting
        std::vector<std::pair<int, double>> combined;
        for (size_t i = 0; i < found.size(); ++i) {
            combined.push_back(std::make_pair(found[i], scores[i]));
        }

        // Sort the combined vector in descending order based on scores
        std::sort(combined.begin(), combined.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            return a.second > b.second;
        });

        // Extract the top 10 DocIDs
        std::vector<int> topDocIDs;
        for (size_t i = 0; i < 10 && i < combined.size(); ++i) {
            topDocIDs.push_back(combined[i].first);
        }

        return topDocIDs;
    }

// Function to perform disjunctive retrieval
std::vector<int> disjunctive(std::vector<std::string> queryWords,std::unordered_map<int, PageInfo>& pageTable, std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& Binarybuffer){
    
    std::vector<double> scores;
    
    std::vector<int> allDocIDs;
    std::unordered_set<int> uniqueDocIDs;

    for (const std::string& word : queryWords) {
        // Use docIDsByWord to get the document IDs for the current word
        std::vector<size_t> docIDs = docIDsByWord(word, lexiconMap, Binarybuffer);

        // Append the document IDs to the set of unique IDs
        for (size_t i = 0; i < docIDs.size(); ++i) {
            uniqueDocIDs.insert(static_cast<int>(docIDs[i]));
        }
    }

    // Transfer unique IDs from the set to the vector
    for (int uniqueID : uniqueDocIDs) {
        allDocIDs.push_back(uniqueID);
    }

    
scores = BM25Combine(allDocIDs, queryWords, pageTable,lexiconMap, Binarybuffer);
    


// Combine the DocIDs and scores into a pair vector for sorting
    std::vector<std::pair<int, double>> combined;
    for (size_t i = 0; i < allDocIDs.size(); ++i) {
        combined.push_back(std::make_pair(allDocIDs[i], scores[i]));
    }

    // Sort the combined vector in descending order based on scores
    std::sort(combined.begin(), combined.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second;
    });

    // Extract the top 10 DocIDs
    std::vector<int> topDocIDs;
    for (size_t i = 0; i < 10 && i < combined.size(); ++i) {
        topDocIDs.push_back(combined[i].first);
    }

    return topDocIDs;
}

// Function to run a user query
bool runQuery(std::unordered_map<int, PageInfo>& pageTable, std::unordered_map<std::string, LexiconEntry>& lexiconMap, std::ifstream& compressedFile, std::ifstream& TrecFile){
    
    
    
    std::string input;
    std::string line;
    std::cout << "Enter your query: ";
    std::getline(std::cin, input);

    std::vector<std::string> words;
        line = sanitizeText(input);
        std::stringstream wordStream(line);
        std::string word;
    
    // Tokenize the input query
        while (wordStream >> word){
            words.push_back(word);
        }
    
    
    std::string type;
    std::string exit;
    std::string snippet;
    std::string url;
    std::vector<int> top10Docs;
        std::cout << "c or d: ";
    std::getline(std::cin, type);
    
    if (type == "c" || type == "d") {
        } else {
            std::cerr << "Error: Input must be 'c' or 'd'" << std::endl;
            return false;
             // Exit the program with an error code
        }
    if (type == "c") {
            // Call the conjunctive function when 'c' is entered
        top10Docs = conjunctive(words, pageTable, lexiconMap, compressedFile);
        }
    else{
        // Call the disjunctive function when 'd' is entered
        top10Docs = disjunctive(words, pageTable, lexiconMap, compressedFile);
    }
    
    if (!top10Docs.empty()){
        
        for (int i = 0; i < top10Docs.size(); ++i) {
            
            std::cout << "-----------------------------------------------------------"<< "\n";
            std::cout << "Result: " << i+1 << "\n";
            std::cout << "DocID: " << top10Docs[i] << "\n";
            // Display BM25 Score
            std::cout << "BM25 Score: " << BM25forWords(top10Docs[i] ,words, pageTable, lexiconMap, compressedFile)<< "\n";
            // Display URL
            PageInfo info = pageTable[top10Docs[i]];
            url = info.url;
            std::cout << "URL: " << url << "\n";
            std::cout << "\n";
            
            // Display snippets for each word in the query
            for (const std::string& word : words) {
                snippet = getDocumentText(top10Docs[i], word, pageTable, TrecFile);
                std::cout <<snippet << "\n";
                std::cout << "\n";
            }
        }
        std::cout << std::endl;
    }
    else{
        std::cout << "No results found" << url << std::endl;
    }
    
    // Reset file positions at the beginning of the runQuery function

    compressedFile.clear();
    compressedFile.seekg(0, std::ios::beg);

    TrecFile.clear();
    TrecFile.seekg(0, std::ios::beg);

    
    std::cout << "Exit? y or n: ";
    std::getline(std::cin, exit);
    
    if(exit == "y"){
        return true;
    }
    else{
        return false;
    }

    
}

int main() {
    
    
    std::string input;
    std::unordered_map<int, PageInfo> pageTable;
    std::unordered_map<std::string, LexiconEntry> lexiconMap;
    // Loading all the necessary files
    std::cout << "Loading necessary files... " << std::endl;
    std::cout << "Loading binary file... " << std::endl;
    // Open the compressed index file for reading as a binary file
    std::ifstream compressedFile("/Users/nikhilsoni/Desktop/big_test/final/output/compressed_index.bin", std::ios::binary);
    if (!compressedFile) {
        std::cerr << "Failed to open the compressed index file for reading." << std::endl;
        return 1;
    }
    
    std::cout << "Loading Lexicon..." << std::endl;

    // Open the lexicon file for reading
    std::ifstream lexiconFile("/Users/nikhilsoni/Desktop/big_test/final/output/lexicon.txt");
    if (!lexiconFile) {
        std::cerr << "Failed to open the lexicon file for reading." << std::endl;
        return 1;
    }
    
    // Open the page table for reading
    std::ifstream UrlFile("/Users/nikhilsoni/Desktop/big_test/final/output/PageTable.txt");
    if (!UrlFile) {
        std::cerr << "Failed to open the Page table for reading." << std::endl;
        return 1;
    }
    
    // load the trec file
    std::ifstream TrecFile("/Users/nikhilsoni/Desktop/big_test/index_test/docs.trec");
    if (!TrecFile) {
        std::cerr << "Failed to open the Trec file for reading." << std::endl;
        return 1;
    }
    
    std::string line;
    while (std::getline(lexiconFile, line)) {
        std::istringstream iss(line);
        std::string word;
        size_t docFreq, startPos, endPos, totalPost;

        if (iss >> word >> docFreq >> startPos >> endPos >> totalPost) {
                // Create a LexiconEntry object and insert it into the unordered_map
            LexiconEntry entry;
            entry.DocFreq = docFreq;
            entry.StartPos = startPos;
            entry.EndPos = endPos;
            entry.TotalPostings = totalPost;
            lexiconMap[word] = entry;
        }
        
        
        else {
                std::cerr << "Error parsing line: " << line << std::endl;
            }
        }
    std::cout << "Loading PageTable..." << std::endl;
    lexiconFile.close();
    
    
    std::string line2;
        while (std::getline(UrlFile, line2)) {
            std::istringstream iss(line2);
            int docId;
            char colon;
            PageInfo pageInfo;

            // Parse the line using ':' as a delimiter.
            if (iss >> docId >> colon >> pageInfo.docLength >> pageInfo.startPosition >> pageInfo.url) {
                pageTable[docId] = pageInfo;
            } else {
                std::cerr << "Failed to parse line: " << line << std::endl;
            }
        }
    
    std::cout << "All Files Loaded!" << std::endl;
    UrlFile.close();
    
    bool exit = false;
    while(exit == false){
        exit = runQuery(pageTable,lexiconMap, compressedFile, TrecFile);
    }

    // Closing all the remaining open files
    TrecFile.close();
    compressedFile.close();
    
    return 0;
}
