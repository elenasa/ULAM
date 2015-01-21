#include <stdlib.h>
#include "NodeConstant.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstant::NodeConstant(Token tok, SymbolConstantValue * symptr, CompilerState & state) : NodeTerminal(state), m_token(tok), m_constSymbol(symptr), m_ready(false)
  {
    assert(symptr);
    updateConstant();
  }

  NodeConstant::~NodeConstant(){}


  void NodeConstant::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }


  const char * NodeConstant::getName()
  {
    return NodeTerminal::getName();
  }


  const std::string NodeConstant::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeConstant::constantFold(Token tok)
  {
    //not same meaning as NodeTerminal; bypass.
  }


  bool NodeConstant::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return true;
  }


  UTI NodeConstant::checkAndLabelType()
  {
    //impossible to use before def; o.w. it would be a NodeIdent
    assert(m_constSymbol);

    UTI it = m_constSymbol->getUlamTypeIdx();
    setNodeType(it);
    setStoreIntoAble(false);

    //copy m_constant from Symbol into NodeTerminal parent.
    if(!m_ready)
      m_ready = updateConstant();

    return it;
  } //checkAndLabelType


  EvalStatus NodeConstant::eval()
  {
    if(!m_ready)
      m_ready = updateConstant();
    return NodeTerminal::eval();
  } //eval


  void NodeConstant::genCode(File * fp, UlamValue& uvpass)
  {
    if(!m_ready)
      m_ready = updateConstant();
    NodeTerminal::genCode(fp, uvpass);
  } //genCode


  bool NodeConstant::updateConstant()
  {
    u32 val;
    m_constSymbol->getValue(val);
    m_constant.uval = val;
    return m_constSymbol->isReady();
  } //updateConstant

} //end MFM
