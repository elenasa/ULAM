/* -*- c++ -*- */

#ifndef NODEBLOCKCLASS_H
#define NODEBLOCKCLASS_H

#include "NodeBlock.h"
#include "NodeBlockFunction.h"
#include "Symbol.h"

namespace MFM{

  class NodeBlockClass : public NodeBlock
  {
  public:

    NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s = NULL);

    virtual ~NodeBlockClass();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    //checks both function and variable symbol names
    virtual bool isIdInScope(u32 id, Symbol * & symptrref);

    bool isFuncIdInScope(u32 id, Symbol * & symptrref);
    void addFuncIdToScope(u32 id, Symbol * symptr);

     u32 getNumberOfFuncSymbolsInTable();
     u32 getSizeOfFuncSymbolsInTable();

  protected:
    SymbolTable m_functionST;

  private:
    NodeBlockFunction * findMainFunctionNode();

  };

}

#endif //end NODEBLOCKCLASS_H
