/* -*- c++ -*- */

#ifndef NODEBLOCKFUNCTIONDEFINITION_H
#define NODEBLOCKFUNCTIONDEFINITION_H

#include "File.h"
#include "SymbolFunction.h"
#include "NodeBlock.h"

namespace MFM{

  class NodeBlockFunctionDefinition : public NodeBlock
  {
  public:

    NodeBlockFunctionDefinition(SymbolFunction * fsym, NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s = NULL);

    virtual ~NodeBlockFunctionDefinition();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

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

#endif //end NODEBLOCKFUNCTIONDEFINITION_H
