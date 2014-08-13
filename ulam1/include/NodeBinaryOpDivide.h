/* -*- c++ -*- */

#ifndef NODEBINARYOPDIVIDE_H
#define NODEBINARYOPDIVIDE_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpDivide : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpDivide(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpDivide();

    virtual UlamType * checkAndLabelType();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPDIVIDE_H
