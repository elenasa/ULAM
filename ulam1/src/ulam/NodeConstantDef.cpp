#include <stdlib.h>
#include "NodeConstantDef.h"
#include "CompilerState.h"


namespace MFM {

  NodeConstantDef::NodeConstantDef(SymbolConstantValue * symptr, CompilerState & state) : Node(state), m_constSymbol(symptr), m_nodeExpr(NULL), m_currBlockNo(m_state.getCurrentBlockNo())
  {
    if(symptr)
      {
	// node uses current block no, not the one saved in the symbol (e.g. pending class args)
	m_cid = symptr->getId();
      }
    else
      m_cid = 0; //error
  }

  NodeConstantDef::NodeConstantDef(const NodeConstantDef& ref) : Node(ref), m_constSymbol(NULL), m_currBlockNo(ref.m_currBlockNo), m_cid(ref.m_cid)
  {
    if(ref.m_nodeExpr)
      m_nodeExpr = ref.m_nodeExpr->instantiate();
    else
      m_nodeExpr = NULL;
  }

  NodeConstantDef::~NodeConstantDef()
  {
    delete m_nodeExpr;
    m_nodeExpr = NULL;
  }

  Node * NodeConstantDef::instantiate()
  {
    return new NodeConstantDef(*this);
  }

  void NodeConstantDef::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    assert(m_state.getCurrentBlockNo() == m_currBlockNo);
    m_nodeExpr->updateLineage(getNodeNo());
  }//updateLineage

  bool NodeConstantDef::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeExpr == oldnptr)
      {
	m_nodeExpr = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeConstantDef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    return m_nodeExpr->findNodeNo(n, foundNode);
  } //findNodeNo

  void NodeConstantDef::printPostfix(File * fp)
  {
    m_nodeExpr->printPostfix(fp);
    fp->write(" = ");
    fp->write(getName());
    fp->write(" const");
  }

  const char * NodeConstantDef::getName()
  {
    assert(m_constSymbol);
    return m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
  }

  const std::string NodeConstantDef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstantDef::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return true;
  }

  void NodeConstantDef::setSymbolPtr(SymbolConstantValue * cvsymptr)
  {
    assert(cvsymptr);
    m_constSymbol = cvsymptr;
    m_currBlockNo = cvsymptr->getBlockNoOfST();
    assert(m_currBlockNo);
  } //setSymbolPtr

  u32 NodeConstantDef::getSymbolId()
  {
    return m_cid;
  }

  UTI NodeConstantDef::checkAndLabelType()
  {
    UTI it = Nav;
    // instantiate, look up in current block
    if(m_constSymbol == NULL)
      {
	//in case of a cloned unknown
	NodeBlock * currBlock = getBlock();
	m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	Symbol * asymptr = NULL;
	if(m_state.alreadyDefinedSymbol(m_cid, asymptr))
	  {
	    if(asymptr->isConstant())
	      {
		m_constSymbol = (SymbolConstantValue *) asymptr;
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.m_pool.getDataAsString(m_cid).c_str();
		msg << "> is not a constant, and cannot be used as one";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) Named Constant <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "> is not defined, and cannot be used";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	m_state.popClassContext(); //restore
      } //toinstantiate

    assert(m_nodeExpr);
    it = m_nodeExpr->checkAndLabelType();
    if(!m_nodeExpr->isAConstant())
      {
	std::ostringstream msg;
	msg << "Constant value expression for: ";
	msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << ", has an invalid type: <" << m_state.getUlamTypeNameByIndex(it) << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	it = Nav;
      }

    assert(m_constSymbol);
    UTI suti = m_constSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(suti))
      {
	UTI cuti = m_state.getCompileThisIdx();
	UTI mappedUTI = Nav;
	if(m_state.mappedIncompleteUTI(cuti, suti, mappedUTI))
	  {
	    std::ostringstream msg;
	    msg << "Substituting Mapped UTI" << mappedUTI;
	    msg << ", " << m_state.getUlamTypeNameByIndex(mappedUTI).c_str();
	    msg << " for incomplete Named Constant type: ";
	    msg << m_state.getUlamTypeNameByIndex(suti).c_str();
	    msg << " used with constant symbol name '" << getName();
	    msg << "' UTI" << suti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    suti = mappedUTI;
	    m_constSymbol->resetUlamType(mappedUTI); //consistent!
	  }
      }

    if(!m_state.isComplete(suti)) //reload
      {
	UTI cuti = m_state.getCompileThisIdx();
	std::ostringstream msg;
	msg << "Incomplete Named Constant for type: ";
	msg << m_state.getUlamTypeNameByIndex(suti).c_str();
	msg << " used with constant symbol name '" << getName();
	msg << "' UTI" << suti << " while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	it = suti;
      }
    else
      {
	ULAMTYPE eit = m_state.getUlamTypeByIndex(it)->getUlamTypeEnum();
	ULAMTYPE esuti = m_state.getUlamTypeByIndex(suti)->getUlamTypeEnum();
	if(eit != esuti)
	  {
	    std::ostringstream msg;
	    msg << "Named Constant '" << getName();
	    msg << "' type: <" << m_state.getUlamTypeByIndex(suti)->getUlamTypeNameOnly().c_str();
	    msg << "> does not match its value type: <";
	    msg << m_state.getUlamTypeByIndex(it)->getUlamTypeNameOnly().c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    it = suti;
	  }
      }
    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  void NodeConstantDef::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_nodeExpr->countNavNodes(cnt);
  }

  NNO NodeConstantDef::getBlockNo()
  {
    return m_currBlockNo;
  }

  void NodeConstantDef::setBlockNo(NNO n)
  {
    m_currBlockNo = n;
  }

  NodeBlock * NodeConstantDef::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  void NodeConstantDef::setConstantExpr(Node * node)
  {
    m_nodeExpr = node;
  }

  // called during parsing rhs of named constant;
  // Requires a constant expression, else error;
  // (scope of eval is based on the block of const def.)
  bool NodeConstantDef::foldConstantExpression()
  {
    s32 newconst = 0;
    UTI uti = checkAndLabelType(); //find any missing symbol

    if(uti == Nav)
      return false; //e.g. not a constant

    assert(m_constSymbol);
    if(m_constSymbol->isReady())
      return true;

    // if here, must be a constant..
    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(getNodeType()); //offset a constant expression
    EvalStatus evs = m_nodeExpr->eval();
    if( evs == NORMAL)
      {
	UlamValue cnstUV = m_state.m_nodeEvalStack.popArg();
	newconst = cnstUV.getImmediateData(m_state);
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
	msg << "' is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	return false;
      }

    m_constSymbol->setValue(newconst); //isReady now
    return true;
  } //foldConstantExpression

  EvalStatus NodeConstantDef::eval()
  {
    if(m_constSymbol->isReady())
      return NORMAL;
    return ERROR;
  } //eval

  void NodeConstantDef::packBitsInOrderOfDeclaration(u32& offset)
  {
    //do nothing, but override
  }

  void NodeConstantDef::genCode(File * fp, UlamValue& uvpass)
  {}

} //end MFM
