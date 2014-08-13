/* -*- c++ -*- */

#ifndef NODEUNARYOPMINUS_H
#define NODEUNARYOPMINUS_H

#include "NodeUnaryOp.h"

namespace MFM{

  class NodeUnaryOpMinus : public NodeUnaryOp
  {
  public:
    
    NodeUnaryOpMinus(Node * n, CompilerState & state);
    ~NodeUnaryOpMinus();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:
    virtual void doUnaryOperation(u32 slot, u32 nslots);    

  private:
   

  };

}

#endif //end NODEUNARYOP_H
