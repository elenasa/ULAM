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
    ULAMTYPE UT = m_typedefSymbol->getUlamType()->getUlamTypeEnum();
    fp->write(UlamType::getUlamTypeEnumAsString(UT));
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
      it = m_typedefSymbol->getUlamType();  

    setNodeType(it);
    return getNodeType();
  }


  void NodeTypedef::eval()
  {
    assert(m_typedefSymbol);
  }


  bool NodeTypedef::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_typedefSymbol;
    return true;
  }


} //end MFM


