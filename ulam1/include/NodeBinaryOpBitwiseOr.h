/* -*- c++ -*- */

#ifndef NODEBINARYOPBITWISEOR_H
#define NODEBINARYOPBITWISEOR_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpBitwiseOr : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpBitwiseOr(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpBitwiseOr();

    virtual UlamType * checkAndLabelType();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPBITWISEOR_H
