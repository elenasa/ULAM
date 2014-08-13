/* -*- c++ -*- */

#ifndef NODECONTROLIF_H
#define NODECONTROLIF_H

#include "NodeControl.h"

namespace MFM{

  class NodeControlIf : public NodeControl
  {
  public:
    
    NodeControlIf(Node * condNode, Node * trueNode, Node * falseNode, CompilerState & state);
    ~NodeControlIf();

    virtual void printPostfix(File * fp);

    virtual void print(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();


  protected:
    
    Node * m_nodeElse;

  };

}

#endif //end NODECONTROLIF_H
