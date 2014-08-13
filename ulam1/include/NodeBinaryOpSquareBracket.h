/* -*- c++ -*- */

#ifndef NODEBINARYOPSQUAREBRACKET_H
#define NODEBINARYOPSQUAREBRACKET_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeBinaryOpSquareBracket : public NodeBinaryOp
  {
  public:
    
    NodeBinaryOpSquareBracket(Node * left, Node * right, CompilerState & state);
    ~NodeBinaryOpSquareBracket();

    virtual void printOp(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();
    virtual EvalStatus evalToStoreInto();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool installSymbolTypedef(Token atok, u32 bitsize, u32 arraysize, Symbol *& asymptr);
    virtual bool installSymbol(Token atok, u32 arraysize, Symbol *& asymptr);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  protected:

  private:
    //helper method to install symbol
    bool getArraysizeInBracket(u32 & rtnArraySize);

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots){}

  };

}

#endif //end NODEBINARYOPSQUAREBRACKET_H
