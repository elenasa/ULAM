/* -*- c++ -*- */

#ifndef NODESTATEMENTEMPTY_H
#define NODESTATEMENTEMPTY_H

#include "Node.h"

namespace MFM{

  class NodeStatementEmpty : public Node
  {
  public:
    
    NodeStatementEmpty(CompilerState & state);
    ~NodeStatementEmpty();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();
    
  protected:    

  };

}

#endif //end NODESTATEMENTEMPTY_H
