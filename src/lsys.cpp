#include "lsys.hpp"
#include "parser.hpp"
#include "serialize.hpp"

int main (int argc, char** argv)
{
    if(argc <= 1) 
    {
        std::cerr<<"No command line args"<<std::endl;
        return 1;
    }

    LSystem lsys;

    for(int i = 0; i < 2; ++i)
    {
        lsys.sym_bufs[i] = (SymInfo*)std::malloc(SYM_MAX * sizeof(SymInfo));
        lsys.param_bufs[i] = (float*)std::malloc(PARAM_MAX * sizeof(float));
    }

    Parser p(lsys);
    p.Parse(argv[1]);

    lsys.SortProds();

	GraphvizWriter gviz("../gviz.dot");
	Serialize::WriteProductions(gviz, lsys);
	gviz.finishAndWrite2PDF("../gviz.pdf");

   // lsys.PrintCur();

   // for(int i = 0; i < 20; ++i)
   // {
   //     lsys.Run();
   //     lsys.PrintCur();
   // }

    for(int i = 0; i < 2; ++i)
    {
        std::free(lsys.sym_bufs[i]);
        std::free(lsys.param_bufs[i]);
    }

    return 0;
}
