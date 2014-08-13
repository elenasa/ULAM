/* -*- c++ -*- */

#ifndef NODEBINARYOP_H
#define NODEBINARYOP_H

#include "Node.h"
#include "NodeCast.h"

namespace MFM{

  class NodeBinaryOp : public Node
  {
  public:
    
    NodeBinaryOp(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOp();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual void  eval();

  protected:

    Node * m_nodeLeft;
    Node * m_nodeRight;

    UlamType * calcNodeType(UlamType * lt, UlamType * rt);
    UlamType * calcNodeTypeBitwise(UlamType * lt, UlamType * rt);

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots) = 0;

  };

}

#endif //end NODEBINARYOP_H
