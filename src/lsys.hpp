#pragma once 

#include <stdint.h>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>
#include "lsys_config.hpp"
#include "defines.hpp"

union ReturnVal 
{
    bool BOOL;
    float FLOAT;
};

ReturnVal DefaultReturn () {ReturnVal val; val.BOOL = true; return val;}

struct SymInfo
{
    int id;
    uint8_t num_params;
};

struct Symbol 
{
    SymInfo info;
    std::vector<float> params;
};

struct Node 
{
    std::vector<Node*> children={};
    virtual std::string serialize () const = 0;
    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym) = 0;
};

struct ProdNode final : public Node
{
    std::vector<SymInfo> new_syms;
    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym)
    {
        unsigned params_total{0};
        for(auto& new_sym: new_syms)
        {
            *sym_it++ = new_sym;
            params_total += new_sym.num_params;
        }
        ASSERT(children.size() == params_total);
        for(auto& child: children)
        {
            child->eval(sym_it, param_it, sym); //Should set parameters for relevant symbol and increment param_it
        }
        return DefaultReturn();
    }

    virtual std::string serialize () const
    {
        return "ProdNode(" + std::to_string(new_syms.size()) + ")";
    }
};

typedef float (*ParamHookFunc)(Symbol const&);

//Used to create custom hooks to set a parameter in a production
struct ParamHookNode final : public Node
{
    ParamHookFunc hook;

    ParamHookNode (ParamHookFunc hook_) : hook{hook_} {}

    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym)
    {
        (void)sym_it; (void)param_it;
        ASSERT(children.empty());
        float flt_val{hook(sym)};
        ReturnVal val;
        val.FLOAT = flt_val;
        return val;
    }

    virtual std::string serialize () const
    {
        return "ParamHookNode";
    }
};

struct SetParamNode final : public Node 
{
    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym)
    {
        ASSERT(children.size() == 1);
        ReturnVal val{children[0]->eval(sym_it, param_it, sym)};
        *param_it++ = val.FLOAT; 
        return DefaultReturn();
    }
    virtual std::string serialize () const {return "SetParamNode";}
};

struct UnaryOpNode : public Node 
{
    virtual float op (float const& inp) = 0;

    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym) final
    {
        ASSERT(children.size() == 1);
        ReturnVal inp{children[0]->eval(sym_it, param_it, sym)};
        ReturnVal val;
        val.FLOAT = op(inp.FLOAT);
        return val;
    }
};

struct NegateNode final : public UnaryOpNode
{
    virtual float op (float const& inp) {return -inp;}
    virtual std::string serialize () const {return "NegateNode";}
};

struct LnNode final : public UnaryOpNode
{
    virtual float op (float const& inp) {return log(inp);}
    virtual std::string serialize () const {return "LnNode";}
};

struct BinaryOpNode : public Node 
{
    virtual float op (float const& lhs, float const& rhs) = 0;

    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym) final
    {
        ASSERT(children.size() == 2);
        ReturnVal lhs{children[0]->eval(sym_it, param_it, sym)};
        ReturnVal rhs{children[1]->eval(sym_it, param_it, sym)};
        ReturnVal val;
        val.FLOAT = op(lhs.FLOAT, rhs.FLOAT);
        return val;
    }
};

struct AddNode final : public BinaryOpNode
{
    virtual float op (float const& lhs, float const& rhs) {return lhs + rhs;}
    virtual std::string serialize () const {return "AddNode";}
};

struct SubNode final : public BinaryOpNode
{
    virtual float op (float const& lhs, float const& rhs) {return lhs - rhs;}
    virtual std::string serialize () const {return "SubNode";}
};

struct MulNode final : public BinaryOpNode
{
    virtual float op (float const& lhs, float const& rhs) {return lhs * rhs;}
    virtual std::string serialize () const {return "MulNode";}
};

struct DivNode final : public BinaryOpNode
{
    virtual float op (float const& lhs, float const& rhs) {return lhs / rhs;}
    virtual std::string serialize () const {return "DivNode";}
};

struct PowNode final : public BinaryOpNode
{
    virtual float op (float const& lhs, float const& rhs) {return pow(lhs, rhs);}
    virtual std::string serialize () const {return "PowNode";}
};

struct LogNode final : public BinaryOpNode
{
    virtual float op (float const& lhs, float const& rhs) {return log(lhs) / log(rhs);}
    virtual std::string serialize () const {return "LogNode";}
};

struct FloatValNode final : public Node 
{
    float flt_val;
    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym)
    {
        (void)sym_it; (void)param_it; (void)sym;
        ASSERT(children.size() == 0);
        ReturnVal val;
        val.FLOAT = flt_val;
        return val;
    }
    virtual std::string serialize () const 
    {
        return "FloatValNode(" + std::to_string(flt_val) + ")";
    }
};

struct GetParamNode final : public Node 
{
    uint8_t idx;
    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym)
    {
        (void)sym_it; (void)param_it;
        ASSERT(children.size() == 0);
        ReturnVal val;
        val.FLOAT = sym.params.at(idx);
        return val;
    }
    virtual std::string serialize () const 
    {
        return "GetParamNode(" + std::to_string((int)idx) + ")";
    }
};

struct NotNode final : public Node 
{
    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym) final
    {
        ASSERT(children.size() == 1);
        ReturnVal inp{children[0]->eval(sym_it, param_it, sym)};
        ReturnVal val;
        val.BOOL = !inp.BOOL;
        return val;
    }
    virtual std::string serialize () const {return "NotNode";}
};

struct ConnectiveNode : public Node 
{
    virtual bool op (bool const& lhs, bool const& rhs) = 0;

    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym) final
    {
        ASSERT(children.size() == 2);
        ReturnVal lhs{children[0]->eval(sym_it, param_it, sym)};
        ReturnVal rhs{children[1]->eval(sym_it, param_it, sym)};
        ReturnVal val;
        val.BOOL = op(lhs.BOOL, rhs.BOOL);
        return val;
    }
};

struct OrNode final : public ConnectiveNode 
{
    virtual bool op (bool const& lhs, bool const& rhs) {return lhs || rhs;}
    virtual std::string serialize () const {return "OrNode";}
};

struct AndNode final : public ConnectiveNode 
{
    virtual bool op (bool const& lhs, bool const& rhs) {return lhs && rhs;}
    virtual std::string serialize () const {return "AndNode";}
};

struct CompareNode : public Node 
{
    virtual bool op (float const& lhs, float const& rhs) = 0;

    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym) final
    {
        ASSERT(children.size() == 2);
        ReturnVal lhs{children[0]->eval(sym_it, param_it, sym)};
        ReturnVal rhs{children[1]->eval(sym_it, param_it, sym)};
        ReturnVal val;
        val.BOOL = op(lhs.FLOAT, rhs.FLOAT);
        return val;
    }
};

struct LessThanNode final : public CompareNode 
{
    virtual bool op (float const& lhs, float const& rhs) {return lhs < rhs;}
    virtual std::string serialize () const {return "LessThanNode";}
};

struct LessThanEqNode final : public CompareNode 
{
    virtual bool op (float const& lhs, float const& rhs) {return lhs <= rhs;}
    virtual std::string serialize () const {return "LessThanEqNode";}
};

struct GreaterThanNode final : public CompareNode 
{
    virtual bool op (float const& lhs, float const& rhs) {return lhs > rhs;}
    virtual std::string serialize () const {return "GreaterThanNode";}
};

struct GreaterThanEqNode final : public CompareNode 
{
    virtual bool op (float const& lhs, float const& rhs) {return lhs >= rhs;}
    virtual std::string serialize () const {return "GreaterThanEqNode";}
};

struct EqualsNode final : public CompareNode 
{
    virtual bool op (float const& lhs, float const& rhs) {return lhs == rhs;}
    virtual std::string serialize () const {return "EqualsNode";}
};

struct NotEqualsNode final : public CompareNode 
{
    virtual bool op (float const& lhs, float const& rhs) {return lhs != rhs;}
    virtual std::string serialize () const {return "NotEqualsNode";}
};

struct ProdUnaryBranch final : public Node
{
    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym)
    {
        ASSERT(children.size() == 2);
        ReturnVal val{children[0]->eval(sym_it, param_it, sym)};
        if(val.BOOL)
            children[1]->eval(sym_it, param_it, sym);

        return DefaultReturn();
    }
    virtual std::string serialize () const {return "ProdUnaryBranch";}
};

struct ProdBinaryBranch final : public Node
{
    virtual ReturnVal eval (SymInfo*& sym_it, float*& param_it, Symbol const& sym)
    {
        ASSERT(children.size() == 3);
        ReturnVal val{children[0]->eval(sym_it, param_it, sym)};
        if(val.BOOL)
            children[1]->eval(sym_it, param_it, sym);
        else
            children[2]->eval(sym_it, param_it, sym);

        return DefaultReturn();
    }
    virtual std::string serialize () const {return "ProdBinaryBranch";}
};

struct Production 
{
    int inp_id;
    Node* prod_root;

    void Apply (SymInfo*& sym_it, float*& param_it, Symbol sym) 
    {
        ASSERT(sym.info.id == inp_id);
        prod_root->eval(sym_it, param_it, sym); 
    }
};

struct LSystem 
{
    std::vector<Production> prods;
    SymInfo* sym_bufs[2]; 
    float* param_bufs[2];
    bool buf=true;
    
    size_t num_syms;

    std::map<int,std::vector<std::string>> sym_names;

    void PrintCur () const
    {
        std::cout<<num_syms<<" ; ";

        SymInfo* sym_it{buf ? sym_bufs[0] : sym_bufs[1]};
        float* param_it{buf ? param_bufs[0] : param_bufs[1]};
        for(size_t i = 0; i < num_syms; ++i)
        {
            std::vector<std::string> const& vec{sym_names.at((sym_it++)->id)};
            std::cout<<vec[0];
            if(vec.size() > 1)
            {
                std::cout<<'(';
                unsigned i = 1;
                for(; i < vec.size() - 1; ++i)
                {
                    std::cout<<vec[i]<<'='<<(*param_it++)<<','; 
                }
                std::cout<<vec[i]<<'='<<(*param_it++)<<')';
            }
        }
        std::cout<<std::endl;
    }

    void SortProds ()
    {
        std::sort(prods.begin(), prods.end(), [](Production const& lhs, Production const& rhs)
                {return lhs.inp_id < rhs.inp_id;} );
    }

    void Run ()
    {
        SymInfo* sym_in_buf{buf ? sym_bufs[0] : sym_bufs[1]};
        float* param_in_buf{buf ? param_bufs[0] : param_bufs[1]};
        SymInfo* sym_out_buf{buf ? sym_bufs[1] : sym_bufs[0]};
        float* param_out_buf{buf ? param_bufs[1] : param_bufs[0]};

        SymInfo* sym_in_it = sym_in_buf;
        float* param_in_it = param_in_buf;
        SymInfo* sym_out_it = sym_out_buf;
        float* param_out_it = param_out_buf;

        for(size_t i = 0; i < num_syms; ++i)
        {
            SymInfo const& sym{*sym_in_it};
            auto prod_it{std::find_if(prods.begin(), prods.end(), [&](Production const& prod)
                    {return prod.inp_id == sym.id;} )}; //Should use a binary search here because we sort prods

            Symbol sym_in;
            sym_in.info = sym;
            sym_in.params.resize(sym.num_params);
            for(auto& param: sym_in.params)
            {
                param = *param_in_it++;
            }

            if(prod_it != prods.end())
                prod_it->Apply(sym_out_it, param_out_it, sym_in);
            else 
                *sym_out_it++ = sym;

            ++sym_in_it;
        }

        num_syms = (size_t)(sym_out_it - sym_out_buf);
        buf = !buf;
    }
};
