#include "tokenizer.hpp"
#include <iostream>
#include <algorithm>
#include "defines.hpp"

std::vector<std::string> const Token::sp_token_lookup[Token::Type::SPECIAL_COUNT]
{
    {":="                                                   },
    {"("                                                    },
    {")"                                                    },
    {"["                                                    },
    {"]"                                                    },
    {"?"                                                    },
    {":"                                                    },
    {"\n", ";", ";\n", "END OF FILE"                        },
    {","                                                    },
    {"+", "-", "*", "/", "^",                               },
    {"<", ">", "<=", ">=", "==", "!", "!=",                 },
    {"||", "&&"                                             },

    {"AXIOM"                                                },
    {"CONSTANTS"                                            },
    {"PRODUCTIONS"                                          },
};

std::string const Token::dbg_type_enum2str[Token::Type::COUNT+1]
{
    "Maps to",
    "Lbrack",
    "Rbrack",
    "Square lbrack",
    "Square rbrack",
    "Question mark",
    "Colon",
    "EOL",
    "Comma",
    "Arith op",
    "Cnd op",
    "Connective",
    "Axiom label",
    "Constants label",
    "Productions label",
    "INVALID",
    "String", 
    "Int", 
    "Float",
    "INVALID"
};

bool Tokenizer::Tokenize (std::string const& fl_name, std::vector<Token>& tokens)
{
    CHECK(tokens.empty());

    std::ifstream fl(fl_name);
    CHECK(fl);
    
    std::vector<std::vector<std::string>> matches(Token::Type::SPECIAL_COUNT);
    unsigned match_cnt;
    char head;
    std::string cur;
    std::string match;
    std::string end_match; //This is used if we prematurely hit end of file but have a match
    unsigned it;

    for(; fl.get(head); ++it)
    {
        //Pass whitespace
        if(head == ' ')
            continue;

        //Reset parsing info per token parsed
        std::copy(Token::sp_token_lookup, Token::sp_token_lookup+Token::Type::SPECIAL_COUNT, matches.begin());
        match_cnt = 0;
        cur = "";
        match = "";
        it = 0; 

        bool hit_end{false};

        do //first parse special tokens
        {
            cur.push_back(head);
            match_cnt = 0;
            for(auto& opts: matches)
            {
                bool open_match{false};
                for(auto& opt: opts)
                {
                    if(!opt.empty())
                    {
                        if(opt.size() == it) //Fully matched. Set as best match so far.
                        {
                            match = opt; //Override previous matches. Longest match is used.
                            opt = "";
                            continue;
                        }

                        if(opt[it] == head)
                        {
                            open_match = true;
                            if(opt.size() == it + 1) //Only used if we prematurely hit end of file
                                end_match = opt;
                        }
                        else
                        {
                            opt = ""; //Not a possibility
                        }
                    }
                }
                if(open_match)
                    ++match_cnt;
            }

            if(!fl.get(head))
            {
                hit_end = true;
                break;
            }

            ++it;
        }
        while(match_cnt > 0);

        if(hit_end)
            match = end_match;

        if(!match.empty()) //Found a match
        {
            //Find match in lookup
            unsigned match_idx = -1;
            for(unsigned i = 0; i < Token::Type::SPECIAL_COUNT; ++i)
            {
                for(auto& opt: Token::sp_token_lookup[i])
                {
                    if(match == opt)
                    {
                        match_idx = i;
                        goto end_label;
                    }
                }
            }
            end_label:
            CHECK(match_idx != (unsigned)-1);
            tokens.push_back(Token(match, (Token::Type)match_idx));

            //Hit end of file after special token
            if(hit_end)
                break; 
            
            //Rewind
            unsigned diff = cur.size() - match.size();
            for(unsigned i = 0; i < diff+1; ++i) fl.unget();

            continue; //Special token parsed
        }

        //No special tokens, now look for float, int, or string
        if(std::isdigit(cur[0]))
        {
            CHECK(cur.length() == 1); //No special tokens can start with a number

            for(; std::isdigit(head); ++it)
            {
                cur.push_back(head);
                fl.get(head);
            } 

            if(head != '.')
            {
                int int_dat{std::stoi(cur)};
                tokens.push_back(Token(int_dat));
                fl.unget();
                continue; //Int parsed
            }

            //Add '.' to cur
            cur.push_back(head);
            fl.get(head);

            for(; std::isdigit(head); ++it)
            {
                cur.push_back(head);
                fl.get(head);
            }

            float float_dat{std::stof(cur)};
            tokens.push_back(Token(float_dat)); 
            fl.unget();
            continue; //Float parsed
        }

        //Parse string 
        bool parse_string{(bool)std::isalpha(cur[0])};
        for(unsigned i = 1; i < cur.size(); ++i) 
        {
            if(!std::isalnum(cur[i]) && cur[i] != '_')
            {
                parse_string = false;
                break;
            }
        }

        if(parse_string)
        {
            for(; std::isalnum(head) || head == '_'; ++it)
            {
                cur.push_back(head);
                fl.get(head);
            }

            fl.unget();

            tokens.push_back(Token(cur));
            continue;
        } 

        std::cout<<"Tokenization error at string: "<<cur<<std::endl;
        return false;
    }
    tokens.push_back(Token("END OF FILE", Token::Type::EOL)); //End of file

    fl.close();

    return true;
}
