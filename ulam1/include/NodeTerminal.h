/* -*- c++ -*- */

#ifndef NODETERMINAL_H
#define NODETERMINAL_H

#include "Node.h"
#include "Token.h"


namespace MFM{
  
  class NodeTerminal : public Node
  {
  public:
    
    NodeTerminal(Token tok, CompilerState & state);
    ~NodeTerminal();

    virtual void printPostfix(File * f);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:
    Token m_token;

  private:
  };

}

#endif //end NODETERMINAL_H
