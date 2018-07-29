#pragma once
#include "lsys.hpp"
#include "graphviz_writer.hpp"

namespace Serialize 
{
    bool WriteTree (GraphvizWriter& gviz, Node* const& node)
    {
        CHECK(node);
        std::string name{"n" + std::to_string((size_t)&node)};
        std::string label{node->serialize()};
        CHECK(gviz.writeLabeledNode(name, label)); 

        for(auto const& child: node->children)
        {
            std::string child_name{"n" + std::to_string((size_t)&child)};
            std::string child_label{child->serialize()};
            CHECK(WriteTree(gviz, child));
            CHECK(gviz.writeConnection(name, child_name));
        }
        return true;
    }

    bool WriteProductions (GraphvizWriter& gviz, LSystem const& lsys)
    {
        CHECK(gviz.writeHeader());
        CHECK(gviz.writeOptions());
        
        for(Production const& prod: lsys.prods)
        {
            CHECK(WriteTree(gviz, prod.prod_root));
        }

        CHECK(gviz.writeClose());

        return true;
    }
}
