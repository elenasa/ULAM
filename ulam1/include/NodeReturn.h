/* -*- c++ -*- */

#ifndef NODERETURN_H
#define NODERETURN_H

#include "File.h"
#include "Node.h"

namespace MFM{

  class NodeReturn : public Node
  {
  public:

    NodeReturn(Node * s, CompilerState & state);
    virtual ~NodeReturn();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

  private:
    Node * m_node;


  };

}

#endif //end NODESTATEMENTS_H
