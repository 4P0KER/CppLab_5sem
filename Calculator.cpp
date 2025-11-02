#include "Calculator.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <cmath>

// Встроенные функции
class SinFunction : public ICalcFunction {
public:
    double execute(double arg) override { return sin(arg); }
    std::string getName() override { return "sin"; }
};

class CosFunction : public ICalcFunction {
public:
    double execute(double arg) override { return cos(arg); }
    std::string getName() override { return "cos"; }
};

Calculator::Calculator() {
    // Регистрируем встроенные функции
    functions["sin"] = std::make_shared<SinFunction>();
    functions["cos"] = std::make_shared<CosFunction>();
}

bool Calculator::isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

int Calculator::getPriority(char op) {
    switch (op) {
    case '+': case '-': return 1;
    case '*': case '/': return 2;
    default: return 0;
    }
}

std::string Calculator::extractWord(const std::string& str, size_t& pos) {
    std::string word;
    while (pos < str.length() && isalpha(str[pos])) {
        word += str[pos++];
    }
    return word;
}

double Calculator::parseNumber(const std::string& str, size_t& pos) {
    std::string numStr;
    while (pos < str.length() && (isdigit(str[pos]) || str[pos] == '.' || str[pos] == '-')) {
        numStr += str[pos++];
    }
    try {
        return std::stod(numStr);
    }
    catch (const std::exception&) {
        throw std::runtime_error("Invalid number: " + numStr);
    }
}

double Calculator::applyFunction(const std::string& funcName, double arg) {
    auto it = functions.find(funcName);
    if (it != functions.end()) {
        return it->second->execute(arg);
    }
    throw std::runtime_error("Unknown function: " + funcName);
}

double Calculator::calculate(double a, double b, char op) {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/':
        if (b == 0) throw std::runtime_error("Division by zero");
        return a / b;
    default: throw std::runtime_error("Unknown operator");
    }
}

double Calculator::evaluateExpression(const std::string& expr) {
    std::vector<double> values;
    std::vector<char> ops;

    for (size_t i = 0; i < expr.length(); i++) {
        if (isspace(expr[i])) continue;

        if (isdigit(expr[i]) || (expr[i] == '-' && (i == 0 || expr[i - 1] == '('))) {
            values.push_back(parseNumber(expr, i));
            i--;
        }
        else if (isalpha(expr[i])) {
            std::string funcName = extractWord(expr, i);
            if (i < expr.length() && expr[i] == '(') {
                i++;
                std::string argStr;
                int braceCount = 1;
                while (i < expr.length() && braceCount > 0) {
                    if (expr[i] == '(') braceCount++;
                    else if (expr[i] == ')') braceCount--;
                    if (braceCount > 0) argStr += expr[i];
                    i++;
                }
                double arg = evaluateExpression(argStr);
                values.push_back(applyFunction(funcName, arg));
            }
            else {
                throw std::runtime_error("Expected '(' after function name");
            }
        }
        else if (expr[i] == '(') {
            ops.push_back('(');
        }
        else if (expr[i] == ')') {
            while (!ops.empty() && ops.back() != '(') {
                double b = values.back(); values.pop_back();
                double a = values.back(); values.pop_back();
                char op = ops.back(); ops.pop_back();
                values.push_back(calculate(a, b, op));
            }
            if (ops.empty()) throw std::runtime_error("Mismatched parentheses");
            ops.pop_back();
        }
        else if (isOperator(expr[i])) {
            while (!ops.empty() && getPriority(ops.back()) >= getPriority(expr[i])) {
                double b = values.back(); values.pop_back();
                double a = values.back(); values.pop_back();
                char op = ops.back(); ops.pop_back();
                values.push_back(calculate(a, b, op));
            }
            ops.push_back(expr[i]);
        }
        else {
            throw std::runtime_error(std::string("Unexpected character: ") + expr[i]);
        }
    }

    while (!ops.empty()) {
        double b = values.back(); values.pop_back();
        double a = values.back(); values.pop_back();
        char op = ops.back(); ops.pop_back();
        values.push_back(calculate(a, b, op));
    }

    if (values.size() != 1) throw std::runtime_error("Invalid expression");
    return values.back();
}

double Calculator::evaluate(const std::string& expr) {
    return evaluateExpression(expr);
}

void Calculator::listFunctions() const {
    std::cout << "Available functions:";
    for (const auto& [name, func] : functions) {
        std::cout << " " << name;
    }
    std::cout << std::endl;
}