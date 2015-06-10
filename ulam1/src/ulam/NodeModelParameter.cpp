#include <stdlib.h>
#include "NodeModelParameter.h"
#include "CompilerState.h"

namespace MFM {

  NodeModelParameter::NodeModelParameter(Token tok, SymbolParameterValue * symptr, CompilerState & state) : NodeConstant(tok, symptr, state) { }

  NodeModelParameter::NodeModelParameter(const NodeModelParameter& ref) : NodeConstant(ref) {}

  //special constructor that replaces a var with a constant (see NodeIdent)
  NodeModelParameter::NodeModelParameter(const NodeIdent& iref) :  NodeConstant(iref) {}

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
    fp->write(NodeTerminal::getName());
    fp->write(")");
  }

  const char * NodeModelParameter::getName()
  {
    //unlike NodeConstant, model parameter name is used
    return m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
  }

  const std::string NodeModelParameter::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeModelParameter::isAConstant()
  {
    return false; //seems like a contradiction, but no
  }

  UTI NodeModelParameter::checkAndLabelType()
  {
    UTI it = Nav;
    //instantiate, look up in class block; skip if stub copy and already ready.
    if(m_constSymbol == NULL && !isReadyConstant())
      {
	//in case of a cloned unknown
	NodeBlock * currBlock = getBlock();
	m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	Symbol * asymptr = NULL;
	if(m_state.alreadyDefinedSymbol(m_token.m_dataindex,asymptr))
	  {
	    if(asymptr->isModelParameter())
	      {
		m_constSymbol = (SymbolParameterValue *) asymptr;
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "> is not a model parameter, and cannot be used as one with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) Model Parameter <" << m_state.getTokenDataAsString(&m_token).c_str();
	    msg << "> is not defined, and cannot be used with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	m_state.popClassContext(); //restore
      } //lookup symbol

    if(m_constSymbol)
      {
	it = m_constSymbol->getUlamTypeIdx();
      }
    else if(isReadyConstant())
      {
	//stub copy case: still wants uti mapping
	it = NodeTerminal::getNodeType();
      }

    // map incomplete UTI
    if(!m_state.isComplete(it)) //reloads to recheck
      {
	UTI cuti = m_state.getCompileThisIdx();
	std::ostringstream msg;
	msg << "Incomplete Model Parameter for type: ";
	msg << m_state.getUlamTypeNameByIndex(it).c_str();
	msg << " used with parameter symbol name '";
	msg << m_state.getTokenDataAsString(&m_token).c_str();
	msg << "' UTI" << it << " while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }

    setNodeType(it);

    //element parameters exist at the pleasure of the simulator;
    // ulam programmers cannot change them!
    setStoreIntoAble(false);

    //copy m_constant from Symbol into NodeTerminal parent.
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here

    return it;
  } //checkAndLabelType

  //class context set prior to calling us; purpose is to get
  // the value of this constant from the context before
  // constant folding happens.
  bool NodeModelParameter::assignClassArgValueInStubCopy()
  {
    return true; //nothing to do
  } //assignClassArgValueInStubCopy

  void NodeModelParameter::genCode(File * fp, UlamValue& uvpass)
  {
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here

    //excerpt from makeUlamValuePtrForCodeGen in NodeIdent
    UTI nuti = getNodeType();
    s32 tmpnum = m_state.getNextTmpVarNumber();
    uvpass = UlamValue::makePtr(tmpnum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, m_constSymbol->getId());

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol); //*******UPDATED GLOBAL;

    Node::genCodeReadIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeModelParameter::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    assert(0);
  } //genCodeToStoreInto

} //end MFM
