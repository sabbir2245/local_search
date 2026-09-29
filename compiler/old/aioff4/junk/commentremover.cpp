#include<iostream>
#include<fstream>
#include<string>

int main() {
    std::cout << "Hello, World!" << std::endl;
    std::string filename = "tsp.cpp";
    std::string outputFilename = "tsp2.cpp";

    // Open the input file
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    // Open the output file
    std::ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not create file " << outputFilename << std::endl;
        inputFile.close();
        return 1;
    }

    std::string line;
    int lineNumber = 0;

    // Process each line
    while (std::getline(inputFile, line)) {
        lineNumber++;
        std::string processedLine;
        bool insideString = false;
        bool insideChar = false;
        
        // Scan through the line character by character
        for (size_t i = 0; i < line.length(); i++) {
            char c = line[i];
            
            // Toggle string literal flag (handle escaped quotes)
            if (c == '"' && !insideChar && (i == 0 || line[i-1] != '\\')) {
                insideString = !insideString;
                processedLine += c;
            }
            // Toggle character literal flag (handle escaped quotes)
            else if (c == '\'' && !insideString && (i == 0 || line[i-1] != '\\')) {
                insideChar = !insideChar;
                processedLine += c;
            }
            // Check for single-line comment only when not inside string or char
            else if (!insideString && !insideChar && c == '/' && i + 1 < line.length() && line[i+1] == '/') {
                // Single-line comment found, stop processing this line
                break;
            }
            else {
                processedLine += c;
            }
        }

        // Trim trailing whitespace
        size_t end = processedLine.find_last_not_of(" \t");
        if (end != std::string::npos) {
            processedLine = processedLine.substr(0, end + 1);
        } else {
            processedLine.clear();
        }
        
        // Write the processed line if it's not empty
        if (!processedLine.empty()) {
            outputFile << processedLine << std::endl;
        }
    }

    inputFile.close();
    outputFile.close();

    std::cout << "Single-line comments removed successfully!" << std::endl;
    std::cout << "Output saved to: " << outputFilename << std::endl;

    return 0;
}