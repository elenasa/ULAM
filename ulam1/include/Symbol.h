/* -*- c++ -*- */

#ifndef SYMBOL_H
#define SYMBOL_H

#include "itype.h"
#include "Token.h"
#include "UlamType.h"

namespace MFM{

  class Symbol
  {
  public:
    Symbol(u32 id, UlamType * utype);
    virtual ~Symbol();    //abstract

    UlamType * getUlamType();
    u32 getId();
    
    virtual bool isFunction();
    virtual bool isTypedef();
    
    void setDataMember();
    bool isDataMember();

  protected:

  private:
    u32 m_id;            // id to its name (string) in lexer; also in ST
    UlamType * m_utype;  // may seem redundant, but not; from NodeVarDecl, before m_value known.
                         // base type, not array type, used here (e.g. NodeBinaryOp::calcNodeType)
    bool m_dataMember;
  };

}

#endif //end SYMBOL_H
