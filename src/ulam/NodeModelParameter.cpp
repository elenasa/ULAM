#include <stdlib.h>
#include "NodeModelParameter.h"
#include "CompilerState.h"

namespace MFM {

  NodeModelParameter::NodeModelParameter(const Token& tok, SymbolModelParameterValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state) : NodeConstant(tok, symptr, typedesc, state) { }

  NodeModelParameter::NodeModelParameter(const NodeModelParameter& ref) : NodeConstant(ref) {}

  NodeModelParameter::~NodeModelParameter(){}

  Node * NodeModelParameter::instantiate()
  {
    return new NodeModelParameter(*this);
  }

  void NodeModelParameter::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
    fp->write("(");
    fp->write(NodeConstant::getName());
    fp->write(")");
  }

  const char * NodeModelParameter::getName()
  {
    //unlike NodeConstant, model parameter name is used
    if(m_constSymbol)
      return m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
    return m_state.getTokenDataAsString(m_token).c_str();
  }

  const std::string NodeModelParameter::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeModelParameter::isAConstant()
  {
    return false; //seems like a contradiction, but no
  }

  FORECAST NodeModelParameter::safeToCastTo(UTI newType)
  {
    if(NodeConstant::isReadyConstant())
      {
	FORECAST scr = m_state.getUlamTypeByIndex(newType)->safeCast(getNodeType());
	if(scr == CAST_CLEAR)
	  return NodeTerminal::safeToCastTo(newType); //checks fits in bits
	else
	  return scr; //bad or hazy, fitting doesn't matter
      }
    return CAST_HAZY;
  } //safeToCastTo

  void NodeModelParameter::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin) && !hazyKin)
      {
	if(asymptr->isModelParameter())
	  {
	    m_constSymbol = (SymbolModelParameterValue *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) '" << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "' is not a model parameter, and cannot be used as one with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	}
    else
      {
	std::ostringstream msg;
	msg << "(2) Model Parameter '" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "' is not defined, and cannot be used with class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

  //class context set prior to calling us; purpose is to get
  // the value of this constant from the context before
  // constant folding happens.
  bool NodeModelParameter::assignClassArgValueInStubCopy()
  {
    return true; //nothing to do
  } //assignClassArgValueInStubCopy

  void NodeModelParameter::genCode(File * fp, UVPass& uvpass)
  {
    if(!NodeConstant::isReadyConstant())
      m_ready = NodeConstant::updateConstant(); //sets ready here
    assert(NodeConstant::isReadyConstant()); //must be

    //excerpt from makeUVPassForCodeGen in NodeIdent
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    s32 tmpnum = m_state.getNextTmpVarNumber();
    uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, 0, m_constSymbol->getId());

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol); //*******UPDATED GLOBAL;

    Node::genCodeReadIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeModelParameter::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    m_state.abortShouldntGetHere();
  } //genCodeToStoreInto

} //end MFM
