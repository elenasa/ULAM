/* -*- c++ -*- */

#ifndef NODEPROGRAM_H
#define NODEPROGRAM_H

#include "File.h"
#include "NodeBlockClass.h"

namespace MFM{

  class NodeProgram : public Node
  {
  public:

    NodeProgram(CompilerState & state);

    virtual ~NodeProgram();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    void setRootNode(NodeBlockClass * root);


  protected:

  private:
    NodeBlockClass * m_root;

  };

}

#endif //end NODEPROGRAM_H
