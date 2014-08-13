/* -*- c++ -*- */

#ifndef NODEBINARYOPBITWISEANDEQUAL_H
#define NODEBINARYOPBITWISEANDEQUAL_H

#include "NodeBinaryOpEqual.h"

namespace MFM{

  class NodeBinaryOpBitwiseAndEqual : public NodeBinaryOpEqual
  {
  public:
    
    NodeBinaryOpBitwiseAndEqual(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpBitwiseAndEqual();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);
  };

}

#endif //end NODEBINARYOPBITWISEANDEQUAL_H
