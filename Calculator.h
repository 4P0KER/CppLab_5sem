#pragma once
#include <map>
#include <memory>
#include <string>
#include "ICalcFunction.h"

class Calculator {
private:
    std::map<std::string, std::shared_ptr<ICalcFunction>> functions;

    double evaluateExpression(const std::string& expr);
    double parseNumber(const std::string& str, size_t& pos);
    std::string extractWord(const std::string& str, size_t& pos);
    double applyFunction(const std::string& funcName, double arg);
    bool isOperator(char c);
    int getPriority(char op);
    double calculate(double a, double b, char op);

public:
    Calculator();
    double evaluate(const std::string& expr);
    void listFunctions() const;
};