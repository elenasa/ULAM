/* -*- c++ -*- */

#ifndef NODEVARDECL_H
#define NODEVARDECL_H

#include "Node.h"
#include "SymbolVariable.h"

namespace MFM{
  
  class NodeVarDecl : public Node
  {
  public:
    
    NodeVarDecl(SymbolVariable * sym, CompilerState & state);
    ~NodeVarDecl();

    virtual void printPostfix(File * f);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(Symbol *& symptrref);

  private:
    SymbolVariable * m_varSymbol;
    //u32 m_stackFrameIndex;
  };

}

#endif //end NODEVARDECL_H
