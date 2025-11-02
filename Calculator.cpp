#include "Calculator.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <cmath>

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

class SqrtFunction : public ICalcFunction {
public:
    double execute(double arg) override {
        if (arg < 0) throw std::runtime_error("Square root of negative number");
        return sqrt(arg);
    }
    std::string getName() override { return "sqrt"; }
};

Calculator::Calculator() {
    functions["sin"] = std::make_shared<SinFunction>();
    functions["cos"] = std::make_shared<CosFunction>();
    functions["sqrt"] = std::make_shared<SqrtFunction>();
}

bool Calculator::isOperator(char c) {
    return binaryOperators.find(c) != binaryOperators.end() ||
        c == '+' || c == '-' || c == '*' || c == '/';
}

int Calculator::getPriority(char op) {
    auto it = binaryOperators.find(op);
    if (it != binaryOperators.end()) {
        return it->second->getPriority();
    }

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
        try {
            return it->second->execute(arg);
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Function '" + funcName + "' error: " + e.what());
        }
    }
    throw std::runtime_error("Unknown function: " + funcName);
}

double Calculator::calculate(double a, double b, char op) {
    auto it = binaryOperators.find(op);
    if (it != binaryOperators.end()) {
        try {
            return it->second->execute(a, b);
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Operator '") + op + "' error: " + e.what());
        }
    }

    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/':
        if (b == 0) throw std::runtime_error("Division by zero");
        return a / b;
    default: throw std::runtime_error("Unknown operator: " + std::string(1, op));
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
                throw std::runtime_error("Expected '(' after function name: " + funcName);
            }
        }
        else if (expr[i] == '(') {
            ops.push_back('(');
        }
        else if (expr[i] == ')') {
            while (!ops.empty() && ops.back() != '(') {
                if (values.size() < 2) {
                    throw std::runtime_error("Not enough values for operator");
                }
                double b = values.back(); values.pop_back();
                double a = values.back(); values.pop_back();
                char op = ops.back(); ops.pop_back();
                values.push_back(calculate(a, b, op));
            }
            if (ops.empty()) throw std::runtime_error("Mismatched parentheses");
            ops.pop_back();
        }
        else if (isOperator(expr[i])) {
            while (!ops.empty() && ops.back() != '(' &&
                getPriority(ops.back()) >= getPriority(expr[i])) {
                if (values.size() < 2) {
                    throw std::runtime_error("Not enough values for operator");
                }
                double b = values.back(); values.pop_back();
                double a = values.back(); values.pop_back();
                char op = ops.back(); ops.pop_back();
                values.push_back(calculate(a, b, op));
            }
            ops.push_back(expr[i]);
        }
        else {
            throw std::runtime_error(std::string("Unexpected character: '") + expr[i] + "'");
        }
    }

    while (!ops.empty()) {
        if (values.size() < 2) {
            throw std::runtime_error("Not enough values for operator");
        }
        double b = values.back(); values.pop_back();
        double a = values.back(); values.pop_back();
        char op = ops.back(); ops.pop_back();
        values.push_back(calculate(a, b, op));
    }

    if (values.size() != 1) {
        throw std::runtime_error("Invalid expression - unable to compute result");
    }
    return values.back();
}

void Calculator::registerFunction(const std::string& name, CalcFunctionPtr func) {
    functions[name] = func;
    std::cout << "Registered function: " << name << std::endl;
}

bool Calculator::unregisterFunction(const std::string& name) {
    bool removed = functions.erase(name) > 0;
    if (removed) {
        std::cout << "Unregistered function: " << name << std::endl;
    }
    return removed;
}

void Calculator::registerBinaryOperator(char symbol, BinaryOperatorPtr op) {
    binaryOperators[symbol] = op;
    std::cout << "Registered binary operator: '" << symbol << "'" << std::endl;
}

bool Calculator::unregisterBinaryOperator(char symbol) {
    bool removed = binaryOperators.erase(symbol) > 0;
    if (removed) {
        std::cout << "Unregistered binary operator: '" << symbol << "'" << std::endl;
    }
    return removed;
}

double Calculator::evaluate(const std::string& expr) {
    if (expr.empty()) {
        throw std::runtime_error("Empty expression");
    }
    return evaluateExpression(expr);
}

bool Calculator::hasFunction(const std::string& name) const {
    return functions.find(name) != functions.end();
}

bool Calculator::hasBinaryOperator(char symbol) const {
    return binaryOperators.find(symbol) != binaryOperators.end();
}

void Calculator::listFunctions() const {
    if (functions.empty()) {
        std::cout << "No functions available" << std::endl;
        return;
    }

    std::cout << "Available functions (" << functions.size() << "):";
    for (const auto& [name, func] : functions) {
        std::cout << " " << name;
    }
    std::cout << std::endl;
}

void Calculator::listBinaryOperators() const {
    if (binaryOperators.empty()) {
        std::cout << "No binary operators available" << std::endl;
        return;
    }

    std::cout << "Available binary operators (" << binaryOperators.size() << "):";
    for (const auto& [symbol, op] : binaryOperators) {
        std::cout << " '" << symbol << "'";
    }
    std::cout << std::endl;
}