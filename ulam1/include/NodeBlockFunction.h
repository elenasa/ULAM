/* -*- c++ -*- */

#ifndef NODEBLOCKFUNCTION_H
#define NODEBLOCKFUNCTION_H

#include "File.h"
#include "SymbolFunction.h"
#include "NodeBlock.h"

namespace MFM{

  class NodeBlockFunction : public NodeBlock
  {
  public:

    NodeBlockFunction(SymbolFunction * fsym, NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s = NULL);

    virtual ~NodeBlockFunction();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    void setDefinition();
    bool isDefinition();

    // for framestack
    void setMaxDepth(u32 depth);
    u32 getMaxDepth();

    SymbolFunction * getFuncSymbolPtr();

  protected:


  private:
    SymbolFunction * m_funcSymbol;
    bool m_isDefinition;
    u32 m_maxDepth;
  };

}

#endif //end NODEBLOCK_H
