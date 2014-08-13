/* -*- c++ -*- */

#ifndef NODEBINARYOPBITWISEAND_H
#define NODEBINARYOPBITWISEAND_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpBitwiseAnd : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpBitwiseAnd(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpBitwiseAnd();

    virtual UlamType * checkAndLabelType();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPBITWISEAND_H
