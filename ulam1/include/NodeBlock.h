/* -*- c++ -*- */

#ifndef NODEBLOCK_H
#define NODEBLOCK_H

#include "File.h"
#include "NodeStatements.h"
#include "SymbolTable.h"

namespace MFM{

  class NodeBlock : public NodeStatements
  {
  public:

    NodeBlock(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s = NULL);

    virtual ~NodeBlock();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();

    virtual bool isIdInScope(u32 id, Symbol * & symptrref);
    void addIdToScope(u32 id, Symbol * symptr);

    NodeBlock * getPreviousBlockPointer();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

     u32 getNumberOfSymbolsInTable();

     u32 getSizeOfSymbolsInTable();

  protected:
    SymbolTable m_ST;

  private:
    NodeBlock * m_prevBlockNode;

  };

}

#endif //end NODEBLOCK_H
