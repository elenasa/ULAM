/* -*- c++ -*- */

#ifndef NODEBINARYOPMULTIPLY_H
#define NODEBINARYOPMULTIPLY_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpMultiply : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpMultiply(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpMultiply();

    virtual UlamType * checkAndLabelType();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPMULTIPLY_H
