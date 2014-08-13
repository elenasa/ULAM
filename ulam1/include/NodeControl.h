/* -*- c++ -*- */

#ifndef NODECONTROL_H
#define NODECONTROL_H

#include "Node.h"
#include "NodeCast.h"

namespace MFM{

  class NodeControl : public Node
  {
  public:
    
    NodeControl(Node * condNode, Node * trueNode, CompilerState & state);
    ~NodeControl();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual UlamType * checkAndLabelType();

  protected:
    
    Node * m_nodeCondition;
    Node * m_nodeBody;

  };

}

#endif //end NODECONTROL_H
