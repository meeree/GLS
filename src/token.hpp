#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

struct Token
{
    std::string str_dat;
    int int_dat;
    float float_dat;

    enum Type : uint8_t
    {
        MAPS_TO=0,              // := 
        LBRACK,                 // ( 
        RBRACK,                 // ) 
        SQ_LBRACK,              // ] 
        SQ_RBRACK,              // [ 
        LCURLY,                 // {
        RCURLY,                 // }
        QMARK,                  // ? 
        COLON,                  // :
        EOL,                    // one of \n, ;, or ;\n
        COMMA,                  // ,
        
        ARITH_OP,               // one of +, -, *, /, ^
        CND_OP,                 // one of <, >, <=, >=, ==, !=, !
        CONNECTIVE,             // one of || or &&

        AXIOM_LABEL,            // "AXIOM"
        CONSTANTS_LABEL,        // "CONSTANTS"
        PRODUCTIONS_LABEL,      // "PRODUCTIONS"
        SPECIAL_COUNT,          // Number of special tokens 

        STRING,
        INT,
        FLOAT,

        COUNT 
    } type;

    static std::vector<std::string> const sp_token_lookup[Type::SPECIAL_COUNT];
    static std::string const dbg_type_enum2str[Type::COUNT+1];
    std::string const& type_str () const {return dbg_type_enum2str[type];}

    Token (std::string const& str_dat_, Type type_ = Type::STRING) : str_dat{str_dat_}, type{type_} {}
    Token (int int_dat_) : int_dat{int_dat_}, type{Type::INT} {}
    Token (float float_dat_) : float_dat{float_dat_}, type{Type::FLOAT} {}
};
