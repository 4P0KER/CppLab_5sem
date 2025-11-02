#pragma once
#include <string>

class ICalcFunction {
public:
    virtual ~ICalcFunction() = default;
    virtual double execute(double arg) = 0;
    virtual std::string getName() = 0;
};