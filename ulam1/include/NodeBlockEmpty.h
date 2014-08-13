/* -*- c++ -*- */

#ifndef NODEBLOCKEMPTY_H
#define NODEBLOCKEMPTY_H

#include "NodeBlock.h"


namespace MFM{

  class NodeBlockEmpty : public NodeBlock
  {
  public:

    NodeBlockEmpty(NodeBlock * prevBlockNode, CompilerState & state);

    virtual ~NodeBlockEmpty();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();


  protected:


  private:

  };

}

#endif //end NODEBLOCKEMPTY_H
