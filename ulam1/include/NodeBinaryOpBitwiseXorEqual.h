/* -*- c++ -*- */

#ifndef NODEBINARYOPBITWISEXOREQUAL_H
#define NODEBINARYOPBITWISEXOREQUAL_H

#include "NodeBinaryOpEqual.h"

namespace MFM{

  class NodeBinaryOpBitwiseXorEqual : public NodeBinaryOpEqual
  {
  public:
    
    NodeBinaryOpBitwiseXorEqual(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpBitwiseXorEqual();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPBITWISEXOREQUAL_H
