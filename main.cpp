#include <iostream>
#include <string>
#include "Calculator.h"

int main() {
    std::cout << "Basic Calculator" << std::endl;
    std::cout << "================" << std::endl;

    Calculator calculator;
    calculator.listFunctions();
    std::cout << "Supported operators: + - * /" << std::endl;
    std::cout << std::endl;

    std::string input;
    while (true) {
        std::cout << "Enter expression (or 'quit' to exit): ";
        std::getline(std::cin, input);

        if (input == "quit") break;
        if (input.empty()) continue;

        try {
            double result = calculator.evaluate(input);
            std::cout << "Result: " << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "Goodbye!" << std::endl;
    return 0;
}