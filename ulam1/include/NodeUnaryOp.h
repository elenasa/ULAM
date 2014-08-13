/* -*- c++ -*- */

#ifndef NODEUNARYOP_H
#define NODEUNARYOP_H

#include "Node.h"

namespace MFM{

  class NodeUnaryOp : public Node
  {
  public:
    
    NodeUnaryOp(Node * n, CompilerState & state);
    ~NodeUnaryOp();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();

  protected:
    
    Node * m_node;
    virtual void doUnaryOperation(u32 slot, u32 nslots) = 0;    
  };

}

#endif //end NODEUNARYOP_H
