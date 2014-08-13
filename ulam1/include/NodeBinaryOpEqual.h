/* -*- c++ -*- */

#ifndef NODEBINARYOPEQUAL_H
#define NODEBINARYOPEQUAL_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpEqual : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpEqual(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpEqual();

    virtual UlamType * checkAndLabelType();

    virtual void eval();

    virtual void evalToStoreInto();

    virtual const char * getName();

    virtual const std::string prettyNodeName();


  protected:

    void assignUlamValue(UlamValue pluv, UlamValue ruv);

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //end NODEBINARYOPEQUAL_H
