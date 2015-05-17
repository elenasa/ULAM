#include <stdlib.h>
#include "NodeConstantDef.h"
#include "NodeTerminal.h"
#include "CompilerState.h"


namespace MFM {

  NodeConstantDef::NodeConstantDef(SymbolConstantValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_constSymbol(symptr), m_nodeExpr(NULL), m_currBlockNo(m_state.getCurrentBlockNo()), m_nodeTypeDesc(nodetype)
  {
    if(symptr)
      {
	// node uses current block no, not the one saved in the symbol (e.g. pending class args)
	m_cid = symptr->getId();
      }
    else
      m_cid = 0; //error
  }

  NodeConstantDef::NodeConstantDef(const NodeConstantDef& ref) : Node(ref), m_constSymbol(NULL), m_nodeExpr(NULL), m_currBlockNo(ref.m_currBlockNo), m_cid(ref.m_cid), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeExpr)
      m_nodeExpr = ref.m_nodeExpr->instantiate();

    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeConstantDef::~NodeConstantDef()
  {
    delete m_nodeExpr;
    m_nodeExpr = NULL;

    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeConstantDef::instantiate()
  {
    return new NodeConstantDef(*this);
  }

  void NodeConstantDef::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    assert(m_state.getCurrentBlockNo() == m_currBlockNo);
    if(m_nodeExpr)
      m_nodeExpr->updateLineage(getNodeNo());
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

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
    if(m_nodeExpr && m_nodeExpr->findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeConstantDef::printPostfix(File * fp)
  {
    if(m_nodeExpr)
      {
	m_nodeExpr->printPostfix(fp);
	fp->write(" = ");
      }
    fp->write(getName());
    fp->write(" const");
  }

  const char * NodeConstantDef::getName()
  {
    if(m_constSymbol)
      return m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
    return "CONSTDEF?";
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
    UTI it = Nav; //expression type

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

    // NOASSIGN (e.g. for class parameters) doesn't have this!
    if(m_nodeExpr)
      {
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
      }

    assert(m_constSymbol);
    UTI suti = m_constSymbol->getUlamTypeIdx();
    UTI cuti = m_state.getCompileThisIdx();

    // type of the constant
    if(m_nodeTypeDesc)
      {
	UTI duti = m_nodeTypeDesc->checkAndLabelType(); //clobbers any expr it
	if(duti != Nav && suti != duti)
	  {
	    std::ostringstream msg;
	    msg << "REPLACING Symbol UTI" << suti;
	    msg << ", " << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << " used with named constant symbol name '" << getName();
	    msg << "' with node type descriptor type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(duti).c_str();
	    msg << " UTI" << duti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_constSymbol->resetUlamType(duti); //consistent!
	    m_state.mapTypesInCurrentClass(suti, duti);
	    suti = duti;
	  }
      }

    if(!m_state.isComplete(suti)) //reloads
      {
	std::ostringstream msg;
	msg << "Incomplete Named Constant for type: ";
	msg << m_state.getUlamTypeNameByIndex(suti).c_str();
	msg << " used with constant symbol name '" << getName();
	msg << "' UTI" << suti << " while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
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
	    msg << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    it = suti; //default it==Int for temp class args, maynot match after seeing the template
	  }
      }

    setNodeType(suti);

    if(!(m_constSymbol->isReady()))
      {
        foldConstantExpression();
        if(!(m_constSymbol->isReady()))
          setNodeType(Nav);
      }

    return getNodeType();
  } //checkAndLabelType

  void NodeConstantDef::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    if(m_nodeExpr)
      m_nodeExpr->countNavNodes(cnt);

    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavNodes(cnt);
  } //countNavNodes

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
    m_nodeExpr->updateLineage(getNodeNo()); //for unknown subtrees
  }

  // called during parsing rhs of named constant;
  // Requires a constant expression, else error;
  // (scope of eval is based on the block of const def.)
  bool NodeConstantDef::foldConstantExpression()
  {
    u32 newconst = 0;
    UTI uti = getNodeType();

    if(uti == Nav || !m_state.isComplete(uti))
      return false; //e.g. not a constant

    assert(m_constSymbol);
    if(m_constSymbol->isReady())
      return true;

    if(!m_nodeExpr)
      {
        return false;
      }

    // if here, must be a constant..
    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(uti); //offset a constant expression
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
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

    //insure constant value fits in its declared type
    if(!m_nodeExpr->fitsInBits(uti))
      {
	std::ostringstream msg;
	msg << "Attempting to fit named constant's value (";
	msg << getName() << " = " << newconst << ") into a smaller bit size type: ";
	msg<< m_state.getUlamTypeNameByIndex(uti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN); //output warning
	if(updateConstant(newconst))
	  {
	    NodeTerminal * newnode;
	    if(m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum() == Int)
	      newnode = new NodeTerminal((s32) newconst, uti, m_state);
	    else
	      newnode = new NodeTerminal(newconst, uti, m_state);

	    newnode->setNodeLocation(getNodeLocation());
	    delete m_nodeExpr;
	    m_nodeExpr = newnode;
	  }
	else
	  return false;
      }

    m_constSymbol->setValue(newconst); //isReady now!
    return true;
  } //foldConstantExpression

  bool NodeConstantDef::updateConstant(u32 & newconst)
  {
    u32 val = newconst;
    if(!m_constSymbol)
      return false;

    UTI nuti = getNodeType();
    if(!m_state.isComplete(nuti))
      return false;

    //store in UlamType format
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);
    u32 wordsize = nut->getTotalWordSize();
    assert(wordsize == MAXBITSPERINT);

    ULAMTYPE etype = nut->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	newconst = _Int32ToInt32((s32) val, wordsize, nbitsize);
	break;
      case Unsigned:
	newconst = _Unsigned32ToUnsigned32(val, wordsize, nbitsize);
	break;
      case Bool:
	newconst = _Unsigned32ToBool32(val, wordsize, nbitsize);
	break;
      case Unary:
	newconst =  _Unsigned32ToUnary32(val, wordsize, nbitsize);
	break;
      case Bits:
	newconst = _Unsigned32ToBits32(val, wordsize, nbitsize);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: " <<  m_state.getUlamTypeNameByIndex(nuti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };
    return true;
  } //updateConstant

  void NodeConstantDef::fixPendingArgumentNode()
  {
    assert(m_constSymbol);
    // for unseen classes that needed their args "fixed" to proper param name
    // this fixes the saved m_cid while clonePendingClassArgumentsForStubClassInstance
    // (the m_cid is used during full instantiation).
    if(m_constSymbol->getId() != getSymbolId())
      {
	m_cid = m_constSymbol->getId();
      }
  } //fixPendingArgumentNode

  bool NodeConstantDef::assignClassArgValueInStubCopy()
  {
    assert(m_nodeExpr);
    return m_nodeExpr->assignClassArgValueInStubCopy();
  }

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
