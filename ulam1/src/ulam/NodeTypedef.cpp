#include <stdlib.h>
#include "NodeTypedef.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypedef::NodeTypedef(SymbolTypedef * sym, CompilerState & state) : Node(state), m_typedefSymbol(sym) {}

  NodeTypedef::~NodeTypedef(){}

  void NodeTypedef::printPostfix(File * fp)
  {
    fp->write(" typedef");

    fp->write(" ");
    fp->write(m_typedefSymbol->getUlamType()->getUlamTypeNameBrief(&m_state).c_str());
    fp->write(" ");
    fp->write(getName());

    u32 arraysize = m_typedefSymbol->getUlamType()->getArraySize();
    if(arraysize > 0)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }

    fp->write("; ");
  }


  const char * NodeTypedef::getName()
  {
    //same as m_typedefSymbol->getUlamType()->getUlamTypeNameBrief()); //short type name
    return m_state.m_pool.getDataAsString(m_typedefSymbol->getId()).c_str();
  }


  const std::string NodeTypedef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeTypedef::checkAndLabelType()
  {
    UlamType * it;
    if(!m_typedefSymbol)
      {
	MSG("","Typedef symbol is missing",ERR);
	it = m_state.getUlamTypeByIndex(Nav);
      }
    else
      {
	it = m_typedefSymbol->getUlamType();  
	//check for incomplete Classes
	if(it->getUlamClassType() == UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Typedef for type: <" << m_state.getUlamTypeNameByIndex(m_state.getUlamTypeIndex(it)).c_str() << "> used with variable symbol name <" << getName() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

	    m_state.completeIncompleteClassSymbol(it);
	  }
      }
    setNodeType(it);
    return getNodeType();
  }


  EvalStatus NodeTypedef::eval()
  {
    assert(m_typedefSymbol);
    return NORMAL;
  }


  bool NodeTypedef::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_typedefSymbol;
    return true;
  }


  void NodeTypedef::packBitsInOrderOfDeclaration(u32& offset)
  {
    //do nothing ???
  }


  void NodeTypedef::genCode(File * fp)
  {
#if 0
    m_state.indent(fp);
    fp->write("typedef ");

    fp->write(m_typedefSymbol->getUlamType()->getUlamTypeMangledName().c_str()); //for C++
    fp->write(" ");
    fp->write(getName());

    u32 arraysize = m_typedefSymbol->getUlamType()->getArraySize();
    if(arraysize > 0)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    fp->write(";\n");
#endif
  }


} //end MFM


