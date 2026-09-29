#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file> [comment_starter]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string comment_starter = (argc >= 3) ? argv[2] : "//";

    fs::path originalPath(filename);

    if (!fs::exists(originalPath)) {
        std::cerr << "Error: File '" << originalPath.string() << "' does not exist." << std::endl;
        return 1;
    }

    fs::path tempPath = originalPath.string() + ".tmp";

    std::ifstream input(originalPath);
    std::ofstream output(tempPath);

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "Error opening files for processing." << std::endl;
        return 1;
    }

    std::string line;
    size_t commentLen = comment_starter.length();

    while (std::getline(input, line)) {
        std::string cleanedLine = "";
        bool inString = false;
        bool inChar = false;

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];

            if (c == '"' && !inChar) {
                if (i == 0 || line[i - 1] != '\\' || (i >= 2 && line[i - 2] == '\\')) {
                    inString = !inString;
                }
            } 
            else if (c == '\'' && !inString) {
                if (i == 0 || line[i - 1] != '\\' || (i >= 2 && line[i - 2] == '\\')) {
                    inChar = !inChar;
                }
            }

            if (!inString && !inChar && commentLen > 0) {
                if (line.compare(i, commentLen, comment_starter) == 0) {
                    break;
                }
            }

            cleanedLine += c;
        }

        output << cleanedLine << "\n";
    }

    input.close();
    output.close();

    fs::rename(tempPath, originalPath);
    std::cout << "Comments removed: " << originalPath.string() << std::endl;
    return 0;
}
