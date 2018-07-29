#pragma once

#include "token.hpp"

class Tokenizer 
{
public:
    Tokenizer () {}

    bool Tokenize (std::string const& fl_name, std::vector<Token>& tokens);
};
