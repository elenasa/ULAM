#include <sstream>
#include <string.h>
#include "CompilerState.h"
#include "SymbolClass.h"

namespace MFM {

  SymbolClass::SymbolClass(u32 id, UlamType * utype, NodeBlockClass * classblock) : Symbol(id, utype), m_classBlock(classblock){}

  SymbolClass::~SymbolClass()
  {
    delete m_classBlock;
    m_classBlock = NULL;
  }


  void SymbolClass::setClassBlockNode(NodeBlockClass * node)
  {
    m_classBlock = node;
  }


  NodeBlockClass * SymbolClass::getClassBlockNode()
  {
    return m_classBlock;
  }
    

  bool SymbolClass::isClass()
  {
    return true;
  }


  const std::string SymbolClass::getMangledPrefix()
  {
    //    return ((UlamTypeClass *) getUlamType())->getUlamTypeAsStringForC();
    return getUlamType()->getUlamTypeUPrefix();
  }


  ULAMCLASSTYPE SymbolClass::getUlamClassType()
  {
    return  ((UlamTypeClass * ) getUlamType())->getUlamClassType();
  }


  void SymbolClass::setUlamClassType(ULAMCLASSTYPE type)
  {
    ((UlamTypeClass * ) getUlamType())->setUlamClassType(type);
  }


} //end MFM
