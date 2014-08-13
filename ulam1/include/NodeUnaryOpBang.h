/* -*- c++ -*- */

#ifndef NODEUNARYOPBANG_H
#define NODEUNARYOPBANG_H

#include "NodeUnaryOp.h"

namespace MFM{

  class NodeUnaryOpBang : public NodeUnaryOp
  {
  public:
    
    NodeUnaryOpBang(Node * n, CompilerState & state);
    ~NodeUnaryOpBang();

    virtual UlamType * checkAndLabelType();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:
    virtual void doUnaryOperation(u32 slot, u32 nslots);    
    
  private:
   

  };

}

#endif //end NODEUNARYOP_H
