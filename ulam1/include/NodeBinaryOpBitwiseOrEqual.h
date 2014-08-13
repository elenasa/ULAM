/* -*- c++ -*- */

#ifndef NODEBINARYOPBITWISEOREQUAL_H
#define NODEBINARYOPBITWISEOREQUAL_H

#include "NodeBinaryOpEqual.h"

namespace MFM{

  class NodeBinaryOpBitwiseOrEqual : public NodeBinaryOpEqual
  {
  public:
    
    NodeBinaryOpBitwiseOrEqual(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpBitwiseOrEqual();


    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPBITWISEOREQUAL_H
