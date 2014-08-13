/* -*- c++ -*- */

#ifndef NODECAST_H
#define NODECAST_H

#include "NodeUnaryOp.h"

namespace MFM{

  class NodeCast : public NodeUnaryOp
  {
  public:
    
    NodeCast(Node * n, UlamType * typeToBe, CompilerState & state);
    ~NodeCast();

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:    
    virtual void doUnaryOperation(u32 slot, u32 nslots){}

  };

}

#endif //end NODECAST_H
