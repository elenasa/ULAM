/* -*- c++ -*- */

#ifndef NODETYPEDEF_H
#define NODETYPEDEF_H

#include "Node.h"
#include "SymbolTypedef.h"

namespace MFM{
  
  class NodeTypedef : public Node
  {
  public:
    
    NodeTypedef(SymbolTypedef * sym, CompilerState & state);
    ~NodeTypedef();

    virtual void printPostfix(File * f);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(Symbol *& symptrref);

  private:
    SymbolTypedef * m_typedefSymbol;
  };

}

#endif //end NODETYPEDEF_H
