/* -*- c++ -*- */

#ifndef NODEBINARYOPMULTIPLYEQUAL_H
#define NODEBINARYOPMULTIPLYEQUAL_H

#include "NodeBinaryOpEqual.h"

namespace MFM{

  class NodeBinaryOpMultiplyEqual : public NodeBinaryOpEqual
  {
  public:
    
    NodeBinaryOpMultiplyEqual(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpMultiplyEqual();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

    void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPMULTIPLYEQUAL_H
