#pragma once
#include "ast.hpp"
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

namespace kumu {

class Interpreter {
public:
    void execute(std::vector<std::unique_ptr<Statement>>& program, Context& ctx);
};

}
