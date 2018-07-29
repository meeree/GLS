#pragma once
#include <fstream>
#include <cmath>

enum GraphvizOption : char
{
    eRankDir=0,
    /*...*/ 
    /*...*/ 
    /*...*/ 
    /*...*/ 
    eOptionCount
};

class GraphvizWriter 
{
private:
    std::ofstream m_fileStream;
    const char* m_fileName;
    unsigned m_clusterCnt;
    std::string m_indent;
    bool checkWriteError();

    struct OptPair 
    {
        std::string const name;
        std::string val;
    };
    OptPair m_options[GraphvizOption::eOptionCount];

public:
    GraphvizWriter (const char* fileName, std::ios_base::openmode const& mode = (std::ios::out | std::ios::trunc));
    ~GraphvizWriter () {if(m_fileStream) {m_fileStream.close();}}

    bool writeHeader ();
    bool writeOptions ();
    bool writeClose (unsigned const& cnt=1);

    bool writeLabeledNode (std::string const& name, std::string const& label, unsigned long const& color=0, bool const& fill=false);
    bool writeBlankNode (std::string const& name, unsigned long const& color=0, bool const& fill=false);
    bool writeConnection (std::string const& name1, std::string const& name2);
    bool writeBlankCluster (int const& index=-1);
    bool writeLabeledCluster (std::string const& label, int const& index=-1);
    int finishAndWrite2PDF (std::string const& outputLoc);

    inline void setOption(GraphvizOption optionIdx, std::string const& val) {m_options[optionIdx].val = val;}
};
