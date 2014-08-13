/* -*- c++ -*- */

#ifndef NODE_H
#define NODE_H

#include <sstream>
#include <stdio.h>
#include <assert.h>
#include "File.h"
#include "UlamType.h"
#include "UlamValue.h"
#include "Locator.h"
#include "Symbol.h"


namespace MFM{

  enum EVALS { EVAL_RHS, EVAL_LHS, EVAL_SIDEEFFECTS};
  enum EvalStatus {ERROR, NORMAL, RETURN, BREAK, CONTINUE};

  struct CompilerState; //forward

  class Node
  {
  public:

    Node(CompilerState & state);
    virtual ~Node() {}

    virtual void print(File * fp);
    virtual void printPostfix(File * fp) = 0;

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval() = 0;
    virtual EvalStatus evalToStoreInto();

    UlamType * getNodeType();
    void setNodeType(UlamType * ut);

    bool isStoreIntoAble();
    void setStoreIntoAble(bool s);

    Locator getNodeLocation();
    void setNodeLocation(Locator loc);
    void printNodeLocation(File * fp);

    virtual bool getSymbolPtr(Symbol *& symptrref);
    virtual bool installSymbolTypedef(Token atok, u32 bitsize, u32 arraysize, Symbol *& asymptr);
    virtual bool installSymbol(Token atok, u32 arraysize, Symbol *& asymptr);

    virtual const char * getName() = 0;

    virtual const std::string prettyNodeName() = 0;
    const std::string nodeName(const std::string& prettyFunction);

    void evalNodeProlog(u32 depth);
    void evalNodeEpilog();
    u32 makeRoomForNodeType(UlamType * type, STORAGE where = EVALRETURN);
    u32 makeRoomForSlots(u32 slots);
    void assignReturnValueToStack(UlamValue rtnUV, STORAGE where = EVALRETURN);
    void assignReturnValuePtrToStack(UlamValue rtnUVptr);

  protected:

    std::string getNodeLocationAsString();
    CompilerState & m_state;  //for printing error messages with path

  private:
    bool m_storeIntoAble;
    UlamType * m_nodeUType;
    Locator m_nodeLoc;

  };

}

#endif //end NODE_H
