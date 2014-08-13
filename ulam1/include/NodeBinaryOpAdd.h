/* -*- c++ -*- */

#ifndef NODEBINARYOPADD_H
#define NODEBINARYOPADD_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpAdd : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpAdd(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpAdd();

    virtual void printOp(File * f);

    virtual UlamType * checkAndLabelType();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPADD_H
