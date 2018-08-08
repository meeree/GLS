#pragma once

#include "lsys.hpp"
#include "tokenizer.hpp"
#include <stack>

//TODO: ADD NEGATIVE NUMBERS

struct Parser 
{
    Tokenizer tokenizer;
    std::vector<Token>::iterator tok_it;

    LSystem& lsys;
    std::map<std::string, SymInfo> sym_id_map;

    SymInfo* sym_it;
    float* param_it;
    int cur_id;

    std::vector<ParamHookFunc> param_hooks;

    Parser (LSystem& lsys_, std::vector<ParamHookFunc> const& param_hooks_={}) : lsys{lsys_}, param_hooks{param_hooks_} {}

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
                    Node* expr_node{nullptr};
                    CHECK(ParseMath(expr_node));
                    CHECK(expr_node);
                    
                    SetParamNode* set_param_node{new SetParamNode};
                    set_param_node->children.push_back(expr_node);
                    
                    prod_root->children.push_back(set_param_node);

                    if(tok_it->type == Token::Type::RBRACK) //Done
                        break;

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

    bool ShuntOp (std::stack<std::pair<Node*, bool>>& stk, std::stack<std::pair<Token, unsigned>>& op_stk)
    {
        Node* op_node{nullptr};
        std::string const& str{op_stk.top().first.str_dat};
        if(str == "+") op_node = new AddNode;
        else if(str == "-") op_node = new SubNode;
        else if(str == "*") op_node = new MulNode;
        else if(str == "/") op_node = new DivNode;
        else if(str == "^") op_node = new PowNode;
        CHECK(op_node);
        stk.push({op_node, false});
        op_stk.pop();
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
        static const std::vector<std::string> r2l_ops{ "^" }; //right-to-left operators

        std::stack<std::pair<Node*, bool>> stk; //Second value indicates if this is a value (true) or an operator (false)
        std::stack<std::pair<Token, unsigned>> op_stk; //Second value is precedence
        Node* val_node{nullptr};
        
        unsigned lbrack_cnt{0}; //Used to terminate for math expressions encased in brackets

        //Shunting-yard algorithm to remove recursion and for more adaptability
        //TODO: ADD ERROR HANDLING
        for(;; ++tok_it)
        {
            if(tok_it->type == Token::Type::LBRACK)
            {
                op_stk.push({*tok_it, 0});
                ++lbrack_cnt;
            }
            else if(tok_it->type == Token::Type::ARITH_OP)
            {
                unsigned prec{0};
                for(; prec < prec_table.size(); ++prec)
                {    
                    std::vector<std::string> const& layer{prec_table[prec]};
                    if(std::find(layer.begin(), layer.end(), tok_it->str_dat) != layer.end())
                        break;
                }

                if(!op_stk.empty() && op_stk.top().first.type == Token::Type::LBRACK)
                {
                    op_stk.push({*tok_it, prec});
                }     
                else
                {
                    while(!op_stk.empty() && op_stk.top().second > prec)
                    {
                        std::cout<<op_stk.top().first.str_dat<<std::endl;
                        CHECK(ShuntOp(stk, op_stk));
                    }            

                    if(op_stk.empty() || op_stk.top().second < prec)
                    {
                        op_stk.push({*tok_it, prec});
                    }
                    else //Equal precedence
                    {
                        bool r2l{std::find(r2l_ops.begin(), r2l_ops.end(), tok_it->str_dat) != r2l_ops.end()};
                        if(!r2l)
                            CHECK(ShuntOp(stk, op_stk));

                        op_stk.push({*tok_it, prec});
                    }
                }
            }
            else if(ParseVarConstOrHook(val_node))
            {
                stk.push({val_node, true});
                --tok_it;
            }
            else if(tok_it->type == Token::Type::RBRACK)
            {
                if(lbrack_cnt == 0) //Done
                {
                    while(!op_stk.empty())
                    {
                        CHECK(ShuntOp(stk, op_stk));
                    }
                    break;
                }

                while(!op_stk.empty() && op_stk.top().first.type != Token::Type::LBRACK)
                {
                    CHECK(ShuntOp(stk, op_stk));
                }
                CHECK(!op_stk.empty()); //Should have LBRACK
                op_stk.pop();
                --lbrack_cnt;
            }
            else  //Done
            {
                while(!op_stk.empty())
                {
                    CHECK(ShuntOp(stk, op_stk));
                }
                break;
            }
        }

        CHECK(op_stk.empty());
        CHECK(!stk.empty());

        std::stack<std::pair<Node*, bool>> cpy{stk};
        std::vector<std::pair<Node*, bool>> cpy_vec;
        while(!cpy.empty())
        {
            cpy_vec.push_back(cpy.top());
            cpy.pop();
        }
        for(auto it = cpy_vec.rbegin(); it != cpy_vec.rend(); ++it)
        {
            std::cout<<it->first->serialize()<<','<<it->second<<';';
        }
        std::cout<<std::endl;

        //Read RPN notation
        unsigned child_idx{0}; //0, 1 or 2   
        std::stack<Node*> rpn_op_stk;
        expr_node = stk.top().first; //Set root

        while(!stk.empty())
        {
            std::pair<Node*, bool> const& top{stk.top()};
            if(child_idx == 2)
            {
                CHECK(!rpn_op_stk.empty() && rpn_op_stk.top()->children.size() == 2);
                rpn_op_stk.top()->children[0] = top.first; 
                rpn_op_stk.pop();
            }
            else if(child_idx == 1)
            {
                CHECK(!rpn_op_stk.empty() && rpn_op_stk.top()->children.size() == 2);
                rpn_op_stk.top()->children[1] = top.first; 
                child_idx = 2;    
            }
            else if(child_idx == 0)
            {
                child_idx = 1;
            }

            if(!top.second) //operator
            {
                top.first->children.resize(2);
                rpn_op_stk.push(top.first);
                child_idx = 1; //Ready to parse first child (right side child)
            }

            stk.pop();
        }
        CHECK(rpn_op_stk.empty());

        return true;
    }

    bool ParseVarConstOrHook (Node*& expr_node)
    {
        if(tok_it->type == Token::Type::FLOAT) //Float
        {
            expr_node = new FloatValNode;
            static_cast<FloatValNode*>(expr_node)->flt_val = tok_it->float_dat;
        }
        else if(tok_it->type == Token::Type::INT) //Int
        {
            expr_node = new FloatValNode;
            static_cast<FloatValNode*>(expr_node)->flt_val = tok_it->int_dat;
        }
        else if(tok_it->type == Token::Type::STRING) //Variable
        {
            std::vector<std::string> const& names{lsys.sym_names[cur_id]};
            auto it{std::find(names.begin()+1, names.end(), tok_it->str_dat)};
            std::cout<<tok_it->str_dat<<std::endl;
            CHECK(it != names.end());

            uint8_t idx = it - (names.begin()+1);
            expr_node = new GetParamNode;
            static_cast<GetParamNode*>(expr_node)->idx = idx;
        }
        else if(tok_it->type == Token::Type::LCURLY) //Param hook
        {
            ++tok_it;
            CHECK(tok_it->type == Token::Type::INT); 

            int hook_idx{tok_it->int_dat};
            CHECK(hook_idx < param_hooks.size());
            ParamHookFunc const& func{param_hooks[hook_idx]};

            expr_node = new ParamHookNode(func);

            ++tok_it;
            CHECK(tok_it->type == Token::Type::RCURLY);
        }
        else
        {
            return false;
        }
        ++tok_it;

        return true;
    }
};
