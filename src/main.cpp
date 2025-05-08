#include <iostream>
#include <vector>
#include <sstream>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);

    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    
    while (iss >> token) {
      tokens.push_back(token);
    }

    if (tokens[0] == "exit") {
      break;
    } else if (tokens[0] == "echo") {
      for (size_t i = 1; i < tokens.size(); ++i) {
        std::cout << tokens[i] << " ";
      }
      std::cout << std::endl;
    } else {
      std::cout << input << ": command not found" << std::endl;
    }

  }
  return 0;
}
