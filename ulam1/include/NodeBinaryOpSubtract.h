/* -*- c++ -*- */

#ifndef NODEBINARYOPSUBTRACT_H
#define NODEBINARYOPSUBTRACT_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpSubtract : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpSubtract(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpSubtract();

    virtual void printOp(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPSUBTRACT_H
