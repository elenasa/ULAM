#include <stdlib.h>
#include "NodeConstant.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstant::NodeConstant(Token tok, SymbolConstantValue * symptr, CompilerState & state) : NodeTerminal(state), m_token(tok), m_constSymbol(symptr), m_ready(false), m_currBlockNo(0)
  {
    assert(symptr);
    m_currBlockNo = symptr->getBlockNoOfST();
    m_ready = updateConstant(); //sets ready here
  }

  NodeConstant::NodeConstant(const NodeConstant& ref) : NodeTerminal(ref), m_token(ref.m_token), m_constSymbol(NULL), m_ready(false), m_currBlockNo(ref.m_currBlockNo) {}

  NodeConstant::NodeConstant(const NodeIdent& iref) :  NodeTerminal(iref), m_token(iref.getToken()), m_constSymbol(NULL), m_ready(false), m_currBlockNo(iref.getBlockNo()) {}

  NodeConstant::~NodeConstant(){}

  Node * NodeConstant::instantiate()
  {
    return new NodeConstant(*this);
  }

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

  bool NodeConstant::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return true;
  }

  void NodeConstant::constantFoldAToken(Token tok)
  {
    //not same meaning as NodeTerminal; bypass.
  }

  bool NodeConstant::isReadyConstant()
  {
    return m_ready;
  }

  UTI NodeConstant::checkAndLabelType()
  {
    UTI it = Nav;
    //instantiate, look up in class block
    if(m_constSymbol == NULL)
      {
	//in case of a cloned unknown
	NodeBlock * currBlock = getBlock();
	m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	Symbol * asymptr = NULL;
	if(m_state.alreadyDefinedSymbol(m_token.m_dataindex,asymptr))
	  {
	    if(asymptr->isConstant())
	      {
		m_constSymbol = (SymbolConstantValue *) asymptr;
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "> is not a constant, and cannot be used as one with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) Named Constant <" << m_state.getTokenDataAsString(&m_token).c_str();
	    msg << "> is not defined, and cannot be used with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	m_state.popClassContext(); //restore
      } //lookup symbol

    if(m_constSymbol)
      {
	it = m_constSymbol->getUlamTypeIdx();
	if(!m_state.isComplete(it))
	  {
	    UTI cuti = m_state.getCompileThisIdx();
	    UTI mappedUTI = Nav;
	    if(m_state.mappedIncompleteUTI(cuti, it, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "Substituting Mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameByIndex(mappedUTI).c_str();
		msg << " for incomplete Named Constant type: ";
		msg << m_state.getUlamTypeNameByIndex(it).c_str();
		msg << " used with constant symbol name '" << getName();
		msg << "' UTI" << it << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		it = mappedUTI;
		m_constSymbol->resetUlamType(mappedUTI); //consistent!
	      }
	  }

	if(!m_state.isComplete(it)) //reloads
	  {
	    UTI cuti = m_state.getCompileThisIdx();
	    std::ostringstream msg;
	    msg << "Incomplete Named Constant for type: ";
	    msg << m_state.getUlamTypeNameByIndex(it).c_str();
	    msg << " used with constant symbol name '" << getName();
	    msg << "' UTI" << it << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
      } //got const symbol

    setNodeType(it);
    setStoreIntoAble(false);

    //copy m_constant from Symbol into NodeTerminal parent.
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here

    return it;
  } //checkAndLabelType

  NNO NodeConstant::getBlockNo()
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeConstant::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  EvalStatus NodeConstant::eval()
  {
    if(!isReadyConstant())
      m_ready = updateConstant();
    if(!isReadyConstant())
      return ERROR;
    if(!m_state.isComplete(getNodeType()))
      return ERROR;
    return NodeTerminal::eval();
  } //eval

  void NodeConstant::genCode(File * fp, UlamValue& uvpass)
  {
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here
    NodeTerminal::genCode(fp, uvpass);
  } //genCode

  bool NodeConstant::updateConstant()
  {
    u32 val;
    if(!m_constSymbol)
      return false;
    m_constSymbol->getValue(val);
    m_constant.uval = val;
    return m_constSymbol->isReady();
  } //updateConstant

} //end MFM
