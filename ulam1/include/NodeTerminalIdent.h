/* -*- c++ -*- */

#ifndef NODETERMINALIDENT_H
#define NODETERMINALIDENT_H

#include "NodeTerminal.h"
#include "Token.h"
#include "SymbolVariable.h"
#include "UlamType.h"

namespace MFM{
  
  class NodeTerminalIdent : public NodeTerminal
  {
  public:
    
    NodeTerminalIdent(Token tok, SymbolVariable * symptr, CompilerState & state);
    ~NodeTerminalIdent();


    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool installSymbolTypedef(Token atok, u32 bitsize, u32 arraysize, Symbol *& asymptr);
    virtual bool installSymbol(Token atok, u32 arraysize, Symbol *& asymptr);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

  private:
    SymbolVariable * m_varSymbol;
    SymbolVariable *  makeSymbol(UlamType * aut);

  };

}

#endif //end NODETERMINALIDENT_H
