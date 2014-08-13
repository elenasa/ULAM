/* -*- c++ -*- */

#ifndef NODEVARDECLLIST_H
#define NODEVARDECLLIST_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeVarDeclList : public NodeBinaryOp
  {
  public:
    
    NodeVarDeclList(Node * left, Node * right, CompilerState & state);
    ~NodeVarDeclList();

    virtual void printOp(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();
    virtual void  evalToStoreInto();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots){}
  };

}

#endif //end NODEVARDECLLIST_H
