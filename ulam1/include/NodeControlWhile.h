/* -*- c++ -*- */

#ifndef NODECONTROLWHILE_H
#define NODECONTROLWHILE_H

#include "NodeControl.h"

namespace MFM{

  class NodeControlWhile : public NodeControl
  {
  public:
    
    NodeControlWhile(Node * condNode, Node * trueNode, CompilerState & state);
    ~NodeControlWhile();

    virtual void print(File * fp);

    virtual void eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:
 

  };

}

#endif //end NODECONTROLWHILE_H
