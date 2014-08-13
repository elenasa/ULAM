/* -*- c++ -*- */

#ifndef NODEBINARYOPPLUSEQUAL_H
#define NODEBINARYOPPLUSEQUAL_H

#include "NodeBinaryOpEqual.h"

namespace MFM{

  class NodeBinaryOpPlusEqual : public NodeBinaryOpEqual
  {
  public:
    
    NodeBinaryOpPlusEqual(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpPlusEqual();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);
  };

}

#endif //end NODEBINARYOPPLUSEQUAL_H
