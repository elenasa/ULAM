/* -*- c++ -*- */

#ifndef NODEBINARYOPBITWISEXOR_H
#define NODEBINARYOPBITWISEXOR_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpBitwiseXor : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpBitwiseXor(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpBitwiseXor();

    virtual UlamType * checkAndLabelType();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPBITWISEXOR_H
