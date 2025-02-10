#include "text_processing.h"

// Function to sanitize and clean text
std::string sanitizeText(const std::string &line) {
    std::string result;

    // Iterate through each character in the input line
    for (char c : line) {
        // Check if the character is alphabetic or a space
        if (std::isalpha(c) || std::isspace(c)) {
            result += c; // Add the character to the result if it's valid
        }
    }

    return result; // Return the sanitized text
}
