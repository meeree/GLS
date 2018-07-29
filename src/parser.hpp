#pragma once

#include "lsys.hpp"
#include "tokenizer.hpp"
#include <stack>

struct Parser 
{
    Tokenizer tokenizer;
    std::vector<Token>::iterator tok_it;

    LSystem& lsys;
    std::map<std::string, SymInfo> sym_id_map;

    SymInfo* sym_it;
    float* param_it;
    int cur_id;

    Parser (LSystem& lsys_) : lsys{lsys_} {}

    bool ParseAxiomSymbol () 
    {
        CHECK(tok_it->type == Token::Type::STRING);
        auto map_it{sym_id_map.find(tok_it->str_dat)};

        if(map_it == sym_id_map.end()) //Symbol not already identified. TODO: PARSE IT NOW INSTEAD
        {
        }

        *sym_it = map_it->second;
        ++tok_it;
        if(sym_it->num_params == 0) // No params. brackets are optional
        {
            if(tok_it->type == Token::Type::LBRACK) //Brackets optional
            {
                ++tok_it;
                CHECK(tok_it->type == Token::Type::RBRACK);
                ++tok_it;
            }
        }
        else //Parameters exist. Parse them.
        {
            CHECK(tok_it->type == Token::Type::LBRACK);
            ++tok_it;
            while(true)
            {
                if(tok_it->type == Token::Type::FLOAT)
                {
                    *param_it++ = tok_it->float_dat;
                }
                else
                {
                    CHECK(tok_it->type == Token::Type::INT);
                    *param_it++ = tok_it->int_dat;
                }
                ++tok_it;

                if(tok_it->type == Token::Type::RBRACK) //Done
                    break;

                CHECK(tok_it->type == Token::Type::COMMA); //Still going. Haven't hit RBRACK. 
                ++tok_it;
            }
            ++tok_it;
        }

        ++sym_it; //New symbol
        return true;
    }
    
    bool ParseBranches (Node*& root)
    {
        return ParseProductionOutput(root);
        if(tok_it->type == Token::Type::QMARK)
        {
            //TODO: FINISH
        }
    }

    bool ParseProductionOutput (Node*& prod_root) 
    {
        while(tok_it->type != Token::Type::EOL && tok_it->type != Token::Type::COLON)
        {
            CHECK(tok_it->type == Token::Type::STRING);
            SymInfo const& prod_sym{sym_id_map[tok_it->str_dat]};

            static_cast<ProdNode*>(prod_root)->new_syms.push_back(prod_sym);
            ++tok_it;

            //TODO: CHECK PARAM COUNTS ARE CORRECT HERE
            if(tok_it->type == Token::Type::LBRACK)
            {
                ++tok_it; 
                while(true)
                {
                    CHECK(tok_it->type == Token::Type::STRING);

                    Node* expr_node{nullptr};
                    CHECK(ParseMath(expr_node));
                    
                    SetParamNode* set_param_node{new SetParamNode};
                    set_param_node->children.push_back(expr_node);
                    
                    prod_root->children.push_back(set_param_node);

                    ++tok_it;
                    if(tok_it->type == Token::Type::RBRACK) //Done
                        break;

                    std::cout<<(tok_it->type_str()<<','<<tok_it->type_str()<<std::endl;
                    CHECK(tok_it->type == Token::Type::COMMA); //Still going. Haven't hit RBRACK. 
                    ++tok_it;
                }
                ++tok_it;
            }
        }

        return true;
    }

    bool ParseProduction (bool ident_pass)
    {
        CHECK(tok_it->type == Token::Type::STRING);
        std::vector<std::string> names;
        names.push_back(tok_it->str_dat);
        ++tok_it;

        if(tok_it->type == Token::Type::LBRACK) //Start brackets. Optional
        {
            ++tok_it;
            while(true)
            {
                CHECK(tok_it->type == Token::Type::STRING);
                names.push_back(tok_it->str_dat);
                ++tok_it;

                if(tok_it->type == Token::Type::RBRACK) //Done
                    break;

                CHECK(tok_it->type == Token::Type::COMMA); //Still going. Haven't hit RBRACK. 
                ++tok_it;
            }
            ++tok_it;
        }

        if(ident_pass) //Just identifying symbols and giving them ids. Done for now. 
        {
            lsys.sym_names[cur_id] = names;
            SymInfo info;
            info.id = cur_id;
            info.num_params = names.size() - 1;
            sym_id_map[names[0]] = info;
            ++cur_id; //Increment symbol id

            while(tok_it->type != Token::Type::EOL) {++tok_it;} //Pass till EOL character
            while(tok_it->type == Token::Type::EOL) {++tok_it;} //Pass new lines
            return true;
        }

        CHECK(tok_it->type == Token::Type::MAPS_TO);
        ++tok_it;

        SymInfo const& cur_info{sym_id_map[names[0]]};
        cur_id = cur_info.id;

        lsys.prods.push_back(Production());
        Production& prod{lsys.prods.back()};
        prod.inp_id = cur_info.id;
        prod.prod_root = new ProdNode;

        ParseBranches(prod.prod_root);

        CHECK(tok_it->type == Token::Type::EOL);
        ++tok_it;
        while(tok_it->type == Token::Type::EOL) {++tok_it;} //Pass new lines

        return true;
    }

    bool ParseAxioms () 
    {
        CHECK(tok_it->type == Token::Type::AXIOM_LABEL);
        ++tok_it; //Pass axioms label

        while(tok_it->type == Token::Type::EOL) {++tok_it;} //Pass new lines

        do
        {
            CHECK(ParseAxiomSymbol());
        }
        while(tok_it->type != Token::Type::EOL);

        return true;
    }

    bool ParseProductions ()
    {
        CHECK(tok_it->type == Token::Type::PRODUCTIONS_LABEL);
        ++tok_it; //Pass productions label

        while(tok_it->type == Token::Type::EOL) {++tok_it;} //Pass new lines

        if(tok_it->type == Token::Type::AXIOM_LABEL)
        {
            std::cout<<"Warning: no productions"<<std::endl;
            return true;
        }

        //Parse over productions to identify all symbols
        auto start_tok_it{tok_it};
        do
        {
            CHECK(ParseProduction(true));
        }
        while(tok_it->type != Token::Type::AXIOM_LABEL);

        //Rewind and do productions parsing
        tok_it = start_tok_it; 
        do
        {
            CHECK(ParseProduction(false));
        }
        while(tok_it->type != Token::Type::AXIOM_LABEL);

        return true;
    }

    bool Parse (std::string const& fl_name)
    {
        std::vector<Token> tokens;
        if(!tokenizer.Tokenize(fl_name, tokens))
        {
            std::cout<<"Tokenization failed"<<std::endl;
            return false;
        }
        for(auto& tok: tokens)
        {
            std::cout<<"("<<tok.type_str()<<": ";
            switch(tok.type)
            {
                case Token::Type::FLOAT: std::cout<<tok.float_dat;                                      break;
                case Token::Type::INT  : std::cout<<tok.int_dat  ;                                      break;
                case Token::Type::EOL  : std::cout<<"'"<<(tok.str_dat.back() == '\n'? tok.str_dat.substr(0,tok.str_dat.size()-1) + "*NL*" : tok.str_dat)<<"'"; break; 
                default                : std::cout<<"'"<<tok.str_dat<<"'" ;                                      break;
            }
            std::cout<<")";
        }
        std::cout<<std::endl;

        cur_id = 0;
        tok_it = tokens.begin();

        param_it = lsys.param_bufs[0];
        sym_it = lsys.sym_bufs[0];

        CHECK(ParseProductions());
        CHECK(ParseAxioms());

        lsys.num_syms = sym_it - lsys.sym_bufs[0];

        while(tok_it->type == Token::Type::EOL) {++tok_it;} //Pass new lines

        return true;
    }

    bool ParseMath (Node*& expr_node)
    {
        static const std::vector<std::vector<std::string>> prec_table
        {
            {"+", "-"},
            {"*", "/"},
            {"^"     } 
        };

        unsigned prec{0};
        std::stack<Node*> stk, prec_stk; 
        int sub_expr_idx{0};
        int brack_cnt{0};

        while(true)
        {
            switch(sub_expr_idx)
            {
                case 0:
                {
                    if(brack_cnt > 0 && tok_it->type == Token::Type::RBRACK)
                    {
                        stk.push(prec_stk.top());
                        prec_stk.pop();
                        --brack_cnt;
                        break;
                    }

                case 2:
                    if(tok_it->type == Token::Type::LBRACK) //Check LBRACK
                    {
                        ++brack_cnt;
                        sub_expr_idx = 0;

                        if(!stk.empty())
                        {
                            prec_stk.push(stk.top());
                            stk.pop();
                        }
                        break;
                    }
               
                    Node* var_node{nullptr};
                    CHECK(ParseVariableOrConstant(var_node));
                    CHECK(var_node);

                    if(sub_expr_idx == 0)
                    {
                        stk.push(var_node);
                    }
                    else if(sub_expr_idx == 2)
                    {
                        Node* top{stk.top()};
                        stk.pop();
                        stk.push(var_node);
                        stk.push(top);
                    }

                    ++sub_expr_idx;
                    break;
                }

                case 1: 
                {
                    std::cout<<tok_it->type_str()<<std::endl;
                    if(tok_it->type != Token::Type::ARITH_OP)
                        goto end_label;

                    Node* op_node{nullptr};
                    unsigned new_prec{0};
                    for(; new_prec < prec_table.size(); ++new_prec)
                    {
                        auto const& lvl{prec_table[new_prec]};
                        for(auto const& op: lvl)
                        {
                            if(op == tok_it->str_dat)
                                break;
                        }
                    }

                    switch(tok_it->str_dat[0])
                    {
                        case '+': op_node = new AddNode; break;
                        case '-': op_node = new SubNode; break;
                        case '*': op_node = new MulNode; break;
                        case '/': op_node = new DivNode; break;
                        case '^': op_node = new PowNode; break;
                    }
                    CHECK(op_node);

                    if(prec < new_prec)
                    {
                        stk.push(op_node);
                    }
                    else
                    {
                        prec_stk.push(stk.top());
                        stk.pop();
                        stk.push(op_node);
                    }

                    prec = new_prec;
                    break;
                }

                case 3:
                {
                    while(!prec_stk.empty())
                    {
                        stk.push(prec_stk.top());
                        prec_stk.pop();
                    }
                    sub_expr_idx = 2; 
                    break;
                }
            }

            std::stack<Node*> cpy{stk};
            while(!cpy.empty())
            {
                std::cout<<cpy.top()->serialize()<<',';
                cpy.pop();
            }
            std::cout<<std::endl;
        }
        end_label:

        return true;
    }

    bool ParseAddExpression (Node*& expr_node) 
    {
        Node* left_node{nullptr};
        CHECK(ParseMulExpression(left_node));
        expr_node = left_node;
        if((tok_it+1)->type == Token::Type::ARITH_OP)
        {
            if((tok_it+1)->str_dat == "+")
            {
                tok_it += 2;
                Node* right_node{nullptr};
                CHECK(ParseAddExpression(right_node));
                expr_node = new AddNode;
                expr_node->children.push_back(left_node);
                expr_node->children.push_back(right_node);
            }
            else if((tok_it+1)->str_dat == "-")
            {
                tok_it += 2;
                Node* right_node{nullptr};
                CHECK(ParseAddExpression(right_node));
                expr_node = new SubNode;
                expr_node->children.push_back(left_node);
                expr_node->children.push_back(right_node);
            }
        }
        return true;
    }

    bool ParseMulExpression (Node*& expr_node) 
    {
        Node* left_node{nullptr};
        CHECK(ParsePowExpression(left_node));
        expr_node = left_node;
        if((tok_it+1)->type == Token::Type::ARITH_OP)
        {
            if((tok_it+1)->str_dat == "*")
            {
                tok_it += 2;
                Node* right_node{nullptr};
                CHECK(ParseAddExpression(right_node));
                expr_node = new MulNode;
                expr_node->children.push_back(left_node);
                expr_node->children.push_back(right_node);
            }
            else if((tok_it+1)->str_dat == "/")
            {
                tok_it += 2;
                Node* right_node{nullptr};
                CHECK(ParseAddExpression(right_node));
                expr_node = new DivNode;
                expr_node->children.push_back(left_node);
                expr_node->children.push_back(right_node);
            }
        }
        return true;
    }

    bool ParsePowExpression (Node*& expr_node) 
    {
        Node* left_node{nullptr};
        CHECK(ParseBrackExpression(left_node));
        expr_node = left_node;
        if((tok_it+1)->type == Token::Type::ARITH_OP)
        {
            if((tok_it+1)->str_dat == "^")
            {
                tok_it += 2;
                Node* right_node{nullptr};
                CHECK(ParseAddExpression(right_node));
                expr_node = new PowNode;
                expr_node->children.push_back(left_node);
                expr_node->children.push_back(right_node);
            }
            else
            {
                expr_node = left_node;
            }
        }
        return true;
    }

    bool ParseBrackExpression (Node*& expr_node) 
    {
        if(tok_it->type == Token::Type::LBRACK)
        {
            ++tok_it;
            CHECK(ParseAddExpression(expr_node)); //Start at top again
            CHECK(tok_it->type == Token::Type::RBRACK);
            ++tok_it;
            return true;
        }
        return ParseVariableOrConstant (expr_node);
    }

    bool ParseVariableOrConstant (Node*& expr_node)
    {
        if(tok_it->type == Token::Type::FLOAT)
        {
            expr_node = new FloatValNode;
            static_cast<FloatValNode*>(expr_node)->flt_val = tok_it->float_dat;
        }
        else if(tok_it->type == Token::Type::INT)
        {
            expr_node = new FloatValNode;
            static_cast<FloatValNode*>(expr_node)->flt_val = tok_it->int_dat;
        }
        else 
        {
            CHECK(tok_it->type == Token::Type::STRING);
            std::vector<std::string> const& names{lsys.sym_names[cur_id]};
            auto it{std::find(names.begin()+1, names.end(), tok_it->str_dat)};
            CHECK(it != names.end());

            uint8_t idx = it - (names.begin()+1);
            expr_node = new GetParamNode;
            static_cast<GetParamNode*>(expr_node)->idx = idx;
        }
        ++tok_it;

        return true;
    }
};