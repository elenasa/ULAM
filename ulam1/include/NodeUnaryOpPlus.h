/* -*- c++ -*- */

#ifndef NODEUNARYOPPLUS_H
#define NODEUNARYOPPLUS_H

#include "NodeUnaryOp.h"

namespace MFM{

  class NodeUnaryOpPlus : public NodeUnaryOp
  {
  public:
    
    NodeUnaryOpPlus(Node * n, CompilerState & state);
    ~NodeUnaryOpPlus();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:
    virtual void doUnaryOperation(u32 slot, u32 nslots);    

  private:
   

  };

}

#endif //end NODEUNARYOP_H
