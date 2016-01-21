#include <stdlib.h>
#include "NodeConstant.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstant::NodeConstant(Token tok, SymbolWithValue * symptr, CompilerState & state) : NodeTerminal(state), m_token(tok), m_constSymbol(symptr), m_ready(false), m_currBlockNo(0)
  {
    assert(symptr);
    m_currBlockNo = symptr->getBlockNoOfST();
    m_ready = updateConstant(); //sets ready here
  }

  NodeConstant::NodeConstant(const NodeConstant& ref) : NodeTerminal(ref), m_token(ref.m_token), m_constSymbol(NULL), m_ready(false), m_currBlockNo(ref.m_currBlockNo) {}

  //special constructor that replaces a var with a constant (see NodeIdent)
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
    if(isReadyConstant())
      return NodeTerminal::getName();
    return m_state.getTokenDataAsString(&m_token).c_str();
  }

  const std::string NodeConstant::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstant::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  void NodeConstant::constantFoldAToken(Token tok)
  {
    //not same meaning as NodeTerminal; bypass.
  }

  bool NodeConstant::isReadyConstant()
  {
    return m_ready;
  }

  FORECAST NodeConstant::safeToCastTo(UTI newType)
  {
    if(isReadyConstant())
      return NodeTerminal::safeToCastTo(newType);
    return CAST_HAZY;
  } //safeToCastTo

  UTI NodeConstant::checkAndLabelType()
  {
    UTI it = Nav;

    bool stubcopy = m_state.isClassAStub(m_state.getCompileThisIdx());

    //instantiate, look up in class block; skip if stub copy and already ready.
    if(!stubcopy && m_constSymbol == NULL)
	checkForSymbol();
    else
      {
	stubcopy = m_state.hasClassAStub(m_state.getCompileThisIdx()); //includes ancestors
      }

    if(m_constSymbol)
      {
	it = m_constSymbol->getUlamTypeIdx();
      }
    else if(isReadyConstant() && stubcopy)
      {
	//stub copy case: still wants uti mapping
	it = NodeTerminal::checkAndLabelType();
      }
    else if(stubcopy)
      {
	// still need its symbol for a value
	// use the member class (unlike checkForSymbol)
      }

    // map incomplete UTI
    if(!m_state.isComplete(it)) //reloads to recheck
      {
	UTI cuti = m_state.getCompileThisIdx();
	std::ostringstream msg;
	msg << "Incomplete " << prettyNodeName().c_str() << " for type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << " used with constant symbol name '";
	msg << m_state.getTokenDataAsString(&m_token).c_str();
	msg << "' UTI" << it << " while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	// m_state.setGoAgain(); wait until updateConstant tried.
      }

    setNodeType(it);
    setStoreIntoAble(false);

    //copy m_constant from Symbol into NodeTerminal parent.
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here
    if(!isReadyConstant())
      {
	it = Hzy;
	if(!stubcopy)
	  m_constSymbol = NULL; //lookup again too! (e.g. inherited template instances)
	m_state.setGoAgain();
      }
    return it;
  } //checkAndLabelType

  void NodeConstant::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
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
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

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

  //class context set prior to calling us; purpose is to get
  // the value of this constant from the context before
  // constant folding happens.
  bool NodeConstant::assignClassArgValueInStubCopy()
  {
    // insure current block NNOs match
    if(m_currBlockNo != m_state.getCurrentBlockNo())
      {
	std::ostringstream msg;
	msg << "Block NNO " << m_currBlockNo << " for <";
	msg << m_state.getTokenDataAsString(&m_token).c_str();
	msg << "> does not match the current block no ";
	msg << m_state.getCurrentBlockNo();
	msg << "; its value cannot be used in stub copy, with class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

    if(m_ready)
      return true; //nothing to do

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
      {
	assert(hazyKin); //always hazy, right?
	if(asymptr->isConstant() && ((SymbolConstantValue *) asymptr)->isReady()) //???
	  {
	    u64 val = 0;
	    ((SymbolConstantValue *) asymptr)->getValue(val);
	    m_constant.uval = val;
	    m_ready = true;
	    //note: m_constSymbol may be NULL; ok in this circumstance (i.e. stub copy).
	  }
      }
    return m_ready;
  } //assignClassArgValueInStubCopy

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
    assert(isReadyConstant()); //must be
    NodeTerminal::genCode(fp, uvpass);
  } //genCode

  bool NodeConstant::updateConstant()
  {
    u64 val;
    if(!m_constSymbol)
      return false;

    if(m_constSymbol->getValue(val))
      m_constant.uval = val; //value fits type per its constantdef
    //else don't want default value here

    return m_constSymbol->isReady();
  } //updateConstant

} //end MFM
