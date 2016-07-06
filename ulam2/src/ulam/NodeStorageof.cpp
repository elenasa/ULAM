#include <stdio.h>
#include "NodeStorageof.h"
#include "CompilerState.h"
#include "SymbolVariableStack.h"

namespace MFM {

  NodeStorageof::NodeStorageof(Token tokof, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_token(tokof), m_varSymbol(NULL), m_oftype(Nouti), m_nodeTypeDesc(nodetype), m_currBlockNo(0)
  {
    Node::setNodeLocation(tokof.m_locator);
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeStorageof::NodeStorageof(const NodeStorageof& ref) : Node(ref), m_token(ref.m_token), m_varSymbol(NULL), m_oftype(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_oftype)), m_nodeTypeDesc(NULL), m_currBlockNo(ref.m_currBlockNo)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeStorageof::~NodeStorageof()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  void NodeStorageof::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeStorageof::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeStorageof::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);
  }

  void NodeStorageof::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(m_state.getTokenDataAsString(&m_token).c_str());
    fp->write(getName());
  }

  const char * NodeStorageof::getName()
  {
    return ".storageof";
  }

  const std::string NodeStorageof::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeStorageof::safeToCastTo(UTI newType)
  {
    return (m_state.isAtom(newType) ? CAST_CLEAR : CAST_BAD);
  } //safeToCastTo

  UTI NodeStorageof::getOfType()
  {
    return m_oftype;
  }

  void NodeStorageof::setOfType(UTI oftyp)
  {
    m_oftype = oftyp;
  }

  UTI NodeStorageof::checkAndLabelType()
  {
    UTI nuti = Nouti;
    if(m_token.m_type == TOK_TYPE_IDENTIFIER)
      {
	assert(m_nodeTypeDesc);
	nuti = m_nodeTypeDesc->checkAndLabelType(); //sets goagain if hzy
      } // got type
    else if(m_token.m_type == TOK_IDENTIFIER)
      {
	if(m_varSymbol == NULL)
	  {
	    // like NodeIdent, in case of template instantiations
	    //used before defined, start search with current block
	    if(m_currBlockNo == 0)
	      {
		if(m_state.useMemberBlock())
		  {
		    NodeBlockClass * memberclass = m_state.getCurrentMemberClassBlock();
		    assert(memberclass);
		    m_currBlockNo = memberclass->getNodeNo();
		  }
		else
		  m_currBlockNo = m_state.getCurrentBlock()->getNodeNo();
	      }

	    NodeBlock * currBlock = getBlock();
	    if(m_state.useMemberBlock())
	      m_state.pushCurrentBlock(currBlock);
	    else
	      m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	    Symbol * asymptr = NULL;
	    bool hazyKin = false; //useful?

	    //searched back through all block's ST for idx
	    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
	      {
		if(hazyKin)
		  nuti = Hzy;
		else
		  {
		    if(!asymptr->isFunction() && !asymptr->isTypedef() && !asymptr->isConstant() && !asymptr->isModelParameter())
		      {
			nuti = asymptr->getUlamTypeIdx();
			m_varSymbol = (SymbolVariable *) asymptr;
			m_currBlockNo = asymptr->getBlockNoOfST(); //refined
		      }
		    else
		      {
			std::ostringstream msg;
			msg << "(1) <" << m_state.getTokenDataAsString(&m_token).c_str();
			msg << "> is not a variable, and cannot be used as one with ";
			msg << getName();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			nuti = Nav;
		      }
		  }
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Unfound symbol variable for " << getName() << " '";
		msg << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		nuti = Nav;
	      }
	    m_state.popClassContext(); //restore
	  }
	else
	    nuti = m_varSymbol->getUlamTypeIdx();
      }
    else
      assert(0); //shouldn't happen

    if(m_state.okUTItoContinue(nuti))
      {
	if(m_state.isReference(nuti))
	  nuti = m_state.getUlamTypeAsDeref(nuti); //e.g. selftyperef

	if(!m_state.isComplete(nuti)) //reloads
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Type for '";
	    msg << m_state.getTokenDataAsString(&m_token).c_str();
	    msg << getName();
	    msg << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    nuti = Hzy;
	    m_state.setGoAgain(); //since not error
	  }
	else
	  {
	    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
	    if(!((nut->getUlamTypeEnum() == Class) || m_state.isAtom(nuti)))
	      {
		std::ostringstream msg;
		msg << "Invalid non-class type provided: '";
		msg << m_state.getTokenDataAsString(&m_token).c_str();
		msg << getName();
		msg << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		nuti = Nav;
	      }

	    if(!nut->isScalar())
	      {
		std::ostringstream msg;
		msg << "Invalid non-scalar type provided: '";
		msg << m_state.getTokenDataAsString(&m_token).c_str();
		msg << getName();
		msg << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		nuti = Nav;
	      }
	  } //complete
      } //ok

    if(m_state.okUTItoContinue(nuti))
      {
	setOfType(nuti); //set here!!
	if(m_token.m_type == TOK_IDENTIFIER)
	  {
	    Node::setStoreIntoAble(TBOOL_TRUE);
	    nuti = UAtomRef;
	  }
	else
	  {
	    Node::setStoreIntoAble(TBOOL_FALSE);
	    nuti = UAtom;
	  }
      }

    setNodeType(nuti);
    return nuti;
  } //checkAndLabelType

  NNO NodeStorageof::getBlockNo() const
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeStorageof::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  EvalStatus NodeStorageof::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    // quark or nonclass data member;
    evalNodeProlog(0); //new current node eval frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    UlamValue uvp = makeUlamValuePtr(); //virtual
    if(!uvp.isPtr())
      {
	evalNodeEpilog();
	return ERROR;
      }

    UlamValue uv = m_state.getPtrTarget(uvp);

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeStorageof::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    // return ptr to this local var (from NodeIdent's makeUlamValuePtr)
    UlamValue rtnUVPtr = makeUlamValuePtr(); //virtual

    if(!rtnUVPtr.isPtr())
      {
	evalNodeEpilog();
	return ERROR;
      }

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

} //end MFM
