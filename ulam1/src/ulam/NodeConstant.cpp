#include <stdlib.h>
#include "NodeConstant.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstant::NodeConstant(Token tok, SymbolConstantValue * symptr, CompilerState & state) : NodeTerminal(state), m_token(tok), m_constSymbol(symptr), m_ready(false), m_currBlock(NULL), m_currBlockNo(0)
  {
    assert(symptr);
    m_currBlockNo = symptr->getBlockNoOfST();
    m_ready = updateConstant();
  }

  NodeConstant::NodeConstant(const NodeConstant& ref) : NodeTerminal(ref), m_token(ref.m_token), m_constSymbol(NULL) /* shallow */, m_ready(false), m_currBlock(NULL), m_currBlockNo(ref.m_currBlockNo) {}

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
    UTI it = Nav;
    //impossible to use before def; o.w. it would be a NodeIdent
    //assert(m_constSymbol);

    //instantiate, look up in class block
    if(m_constSymbol == NULL)
      {
	NodeBlock * savecurrentblock = m_state.m_currentBlock; //**********
	//in case of a cloned unknown
	if(m_currBlock == NULL)
	  setBlock();

	NodeBlockClass * savememberclassblock = m_state.m_currentMemberClassBlock;
	bool saveUseMemberBlock = m_state.m_useMemberBlock;
	m_state.m_useMemberBlock = false;

	m_state.m_currentBlock = m_currBlock; //before lookup

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
		msg << "(1) <" << m_state.getTokenDataAsString(&m_token).c_str() << "> is not a constnat, and cannot be used as one with class: " << m_state.getUlamTypeNameByIndex(m_state.m_compileThisIdx).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) Named Constant <" << m_state.getTokenDataAsString(&m_token).c_str() << "> is not defined, and cannot be used with class: " << m_state.getUlamTypeNameByIndex(m_state.m_compileThisIdx).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	m_state.m_currentBlock = savecurrentblock; //restore
	m_state.m_useMemberBlock = saveUseMemberBlock;
	m_state.m_currentMemberClassBlock = savememberclassblock;
      } //lookup symbol

    if(m_constSymbol)
      it = m_constSymbol->getUlamTypeIdx();
    setNodeType(it);
    setStoreIntoAble(false);

    //copy m_constant from Symbol into NodeTerminal parent.
    if(!m_ready)
      m_ready = updateConstant();

    return it;
  } //checkAndLabelType

  NNO NodeConstant::getBlockNo()
  {
    return m_currBlockNo;
  }

  void NodeConstant::setBlock()
  {
    assert(m_currBlockNo);
    m_currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(m_currBlock);
  }

  EvalStatus NodeConstant::eval()
  {
    if(!m_ready)
      m_ready = updateConstant();
    if(!m_ready)
      return ERROR;
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
    if(!m_constSymbol)
      return false;
    m_constSymbol->getValue(val);
    m_constant.uval = val;
    return m_constSymbol->isReady();
  } //updateConstant

} //end MFM
