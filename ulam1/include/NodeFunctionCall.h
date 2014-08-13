/* -*- c++ -*- */

#ifndef NODEFUNCTIONCALL_H
#define NODEFUNCTIONCALL_H

#include <vector>
#include "Node.h"
#include "Token.h"
#include "SymbolFunction.h"

namespace MFM{

  class NodeFunctionCall : public Node
  {
  public:
    
    NodeFunctionCall(Token fname, SymbolFunction * fsym, CompilerState & state);
    ~NodeFunctionCall();

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual void eval();

    void addArgument(Node * n);

    u32 getNumberOfArguments();

    virtual const char * getName();

    virtual const std::string prettyNodeName();


  private:

    Token m_functionNameTok;
    SymbolFunction * m_funcSymbol;
    std::vector<Node *> m_argumentNodes;

  };

}

#endif //end NODEFUNCTIONCALL_H
