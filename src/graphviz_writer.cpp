#include "graphviz_writer.hpp"
#include <iomanip>
#include <iostream>

GraphvizWriter::GraphvizWriter (const char* fileName, std::ios_base::openmode const& mode) : 
    m_fileName{fileName}, m_fileStream(fileName, mode), m_clusterCnt{0}, m_indent{""},

    m_options{{"rankdir", "UD"}} 
{
    if(!m_fileStream.is_open())
        std::cerr<<"!!ERROR : Failed to open file"<<std::endl;
}

bool GraphvizWriter::checkWriteError() 
{
    if (m_fileStream.bad()) 
    {
        std::cerr<<"!!ERROR : Writing failed: deleting file"<<std::endl;
        m_fileStream.close();
        std::remove(m_fileName);
        return false;
    }
    return true;
}

bool GraphvizWriter::writeHeader ()
{
    m_fileStream<<"digraph G\n{\n";
    m_indent += "\t";
    return checkWriteError();
}

bool GraphvizWriter::writeOptions ()
{
    for(auto const& optionPair: m_options)
    {
        m_fileStream<<m_indent<<optionPair.name<<"=\""<<optionPair.val<<"\";\n"; 
    }
    return checkWriteError();
}

bool GraphvizWriter::writeClose (unsigned const& cnt)
{
    for(unsigned i = 0; i < cnt; ++i)
    {
        m_indent.pop_back();
        m_fileStream<<m_indent<<"}\n";
    }
    return checkWriteError();
}

bool GraphvizWriter::writeLabeledNode (std::string const& name, std::string const& label, unsigned long const& color, bool const& fill)
{
	if(color == 0) //no user inputted color? 
		m_fileStream<<m_indent<<"node_"<<name<<" [label=\""<<label<<"\"];\n";
	else

	{
		m_fileStream<<m_indent<<"node_"<<name<<" [label=\""<<label<<"\"; color=\"#"<<std::setw(6)<<std::setfill('0')<<std::hex<<color<<std::setfill(' ');
        if(!fill)
            m_fileStream<<"\"];\n";
        else 
            m_fileStream<<"\" style=filled;];\n";

	}

    return checkWriteError();
}

bool GraphvizWriter::writeBlankNode (std::string const& name, unsigned long const& color, bool const& fill)
{
	if(color == 0) //no user inputted color? 
    {
		m_fileStream<<m_indent<<"node_"<<name<<";\n";
    }
	else
    {
		m_fileStream<<m_indent<<"node_"<<name<<" [color=\"#"<<std::setw(6)<<std::setfill('0')<<std::hex<<color<<std::setfill(' ');
        if(!fill)
            m_fileStream<<"\"];\n";
        else 
            m_fileStream<<"\" style=filled;];\n";

    }
    m_fileStream<<m_indent<<"node_"<<name<<" [label=\"\"];\n";
    return checkWriteError();
}

bool GraphvizWriter::writeConnection (std::string const& name1, std::string const& name2)
{
    m_fileStream<<m_indent<<"node_"<<name1<<"->"<<"node_"<<name2<<";\n"; 
    return checkWriteError();
}

bool GraphvizWriter::writeBlankCluster (int const& index) 
{
    unsigned useIndex{index == -1 ? m_clusterCnt : (unsigned)index};    
    m_fileStream<<m_indent<<"subgraph cluster_"<<useIndex<<"\n"<<m_indent<<"{\n";
    ++m_clusterCnt;
    m_indent += "\t";
    return checkWriteError();
}

bool GraphvizWriter::writeLabeledCluster (std::string const& label, int const& index) 
{
    unsigned useIndex{index == -1 ? m_clusterCnt : (unsigned)index};    
    m_fileStream<<m_indent<<"subgraph cluster_"<<useIndex<<"\n"<<m_indent<<"{\n";
    ++m_clusterCnt;
    m_indent += "\t";
    m_fileStream<<m_indent<<"label=\""<<label<<"\";\n";
    return checkWriteError();
}


int GraphvizWriter::finishAndWrite2PDF (std::string const& outputLoc)
{
    m_fileStream.close();

    std::cout<<"Writing Graphviz file \""<<m_fileName<<"\" to PDF \""<<outputLoc<<"\""<<std::endl;
    //Send graphviz to pdf
    std::string sysCmd{"dot -Tpdf " + std::string(m_fileName) + " -o " + outputLoc};
    return system(sysCmd.c_str());
}
