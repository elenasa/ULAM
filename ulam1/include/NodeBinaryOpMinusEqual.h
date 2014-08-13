/* -*- c++ -*- */

#ifndef NODEBINARYOPMINUSEQUAL_H
#define NODEBINARYOPMINUSEQUAL_H

#include "NodeBinaryOpEqual.h"

namespace MFM{

  class NodeBinaryOpMinusEqual : public NodeBinaryOpEqual
  {
  public:
    
    NodeBinaryOpMinusEqual(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpMinusEqual();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPMINUSEQUAL_H
