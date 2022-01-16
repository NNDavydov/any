//Copyright Davydov Nikolay 2022

#include <iostream>
#include <string>

#include "any.hpp"

using namespace my_any;

int main() {
    any x("Hello world");
    std::cout << any_cast<const char*>(x) << "\n";

    x = 5;
    any y(x);

    std::cout <<"x = " <<any_cast<int>(x) << " y = " << any_cast<int>(y) << "\n";

    y = 4.2;
    std::cout << any_cast<double>(y) << "\n";

    y.reset();
    if(y.has_value()) std::cout << "y has value\n";
    else std::cout << "y has not value\n";

    const std::string str = "QWERTY";
    y = str;
    std::cout << any_cast<std::string>(y) << "\n";
}
