/* -*- c++ -*- */

#ifndef NODESTATEMENTS_H
#define NODESTATEMENTS_H

#include "File.h"
#include "Node.h"

namespace MFM{

  class NodeStatements : public Node
  {
  public:

    NodeStatements(Node * s, CompilerState & state);
    virtual ~NodeStatements();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();

    virtual void setNextNode(NodeStatements * s);

    virtual const char * getName();


    virtual const std::string prettyNodeName();

  protected:
    Node * m_node;
    NodeStatements * m_nextNode;

  private:


  };

}

#endif //end NODESTATEMENTS_H
