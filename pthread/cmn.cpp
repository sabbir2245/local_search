#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    // ==========================================
    // CONFIGURATION (Edit these two variables)
    // ==========================================
    std::string comment_starter = "//";     // e.g., "//" for C++/Java, "#" for Python
    std::string filename        = "solve2.cpp"; // Target file name

    fs::path originalPath(filename);

    // 1. Verify source file exists
    if (!fs::exists(originalPath)) {
        std::cerr << "Error: File '" << originalPath.string() << "' does not exist." << std::endl;
        return 1;
    }

    // 2. Construct output filename (e.g., input.cpp -> input_2.cpp, script.py -> script_2.py)
    fs::path copyPath = originalPath.stem().string() + "_2" + originalPath.extension().string();

    std::ifstream input(originalPath);
    std::ofstream output(copyPath);

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "Error opening files for processing." << std::endl;
        return 1;
    }

    std::string line;
    size_t commentLen = comment_starter.length();

    // 3. Process line by line
    while (std::getline(input, line)) {
        std::string cleanedLine = "";
        bool inString = false;
        bool inChar = false;

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];

            // Handle string literals (double quotes)
            if (c == '"' && !inChar) {
                if (i == 0 || line[i - 1] != '\\' || (i >= 2 && line[i - 2] == '\\')) {
                    inString = !inString;
                }
            } 
            // Handle character literals (single quotes)
            else if (c == '\'' && !inString) {
                if (i == 0 || line[i - 1] != '\\' || (i >= 2 && line[i - 2] == '\\')) {
                    inChar = !inChar;
                }
            }

            // Check if comment starter matches at current position (outside string/char literals)
            if (!inString && !inChar && commentLen > 0) {
                if (line.compare(i, commentLen, comment_starter) == 0) {
                    break; // Stop parsing the rest of this line
                }
            }

            cleanedLine += c;
        }

        output << cleanedLine << "\n";
    }

    input.close();
    output.close();

    std::cout << "Original file untouched." << std::endl;
    std::cout << "Created stripped copy: " << copyPath.string() << std::endl;
    return 0;
}