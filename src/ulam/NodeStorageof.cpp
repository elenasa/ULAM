#include <stdio.h>
#include "NodeStorageof.h"
#include "NodeMemberSelect.h"
#include "CompilerState.h"
#include "SymbolVariableStack.h"

namespace MFM {

  NodeStorageof::NodeStorageof(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_nodeOf(ofnode), m_oftype(Nouti), m_nodeTypeDesc(nodetype)
  {
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeStorageof::NodeStorageof(const NodeStorageof& ref) : Node(ref), m_nodeOf(NULL), m_oftype(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_oftype,ref.getNodeLocation())), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
    if(ref.m_nodeOf)
      m_nodeOf = ref.m_nodeOf->instantiate();
  }

  NodeStorageof::~NodeStorageof()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
    delete m_nodeOf;
    m_nodeOf = NULL;
  }

  void NodeStorageof::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
    if(m_nodeOf)
      m_nodeOf->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeStorageof::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeOf == oldnptr)
      {
	m_nodeOf = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeStorageof::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    if(m_nodeOf && m_nodeOf->findNodeNo(n, foundNode))
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
    if(m_nodeOf)
      m_nodeOf->printPostfix(fp);
    else
      {
	fp->write(" ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(getOfType()).c_str());
      }
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

  bool NodeStorageof::hasASymbol()
  {
    return m_nodeOf && m_nodeOf->hasASymbol();
  }

  u32 NodeStorageof::getSymbolId()
  {
    assert(m_nodeOf);
    return m_nodeOf->getSymbolId();
  }

  UTI NodeStorageof::getOfType()
  {
    return m_oftype;
  }

  void NodeStorageof::setOfType(UTI oftyp)
  {
    m_oftype = oftyp;
  }

  UTI NodeStorageof::checkAndLabelType(Node * thisparentnode)
  {
  UTI nuti = Nouti;

  if(!m_state.okUTItoContinue(getOfType()))
    {
      //m_nodeOf if variable subject; o.w. nodeTypeDescriptor if Type subject
      if(m_nodeOf)
	{
	  UTI ofuti = m_nodeOf->checkAndLabelType(this);
	  if(m_state.okUTItoContinue(ofuti))
	    {
	      std::ostringstream msg;
	      msg << "Determined type for member '";
	      msg << m_nodeOf->getName();
	      msg << getName() << "', as type: ";
	      msg << m_state.getUlamTypeNameBriefByIndex(ofuti).c_str();
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      nuti = ofuti;
	    }
	  else
	    {
	      std::ostringstream msg;
	      msg << "Undetermined type for missing member '";
	      msg << m_nodeOf->getName();
	      msg << getName() << "'";
	      if(ofuti == Nav)
		{
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  setNodeType(Nav); //missing
		  return Nav;
		}
	      else
		{
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		  setNodeType(Hzy); //missing
		  m_state.setGoAgain(); //too
		  return Hzy;
		}
	    }
	}
      else if(m_nodeTypeDesc)
	{
	  //Of a Type (lhs)
	  nuti = m_nodeTypeDesc->checkAndLabelType(this); //sets goagain if hzy
	}
      else
	m_state.abortShouldntGetHere();
    }
  else
    nuti = getOfType();

  if(m_state.okUTItoContinue(nuti))
    {
      if(m_state.isReference(nuti))
	nuti = m_state.getUlamTypeAsDeref(nuti); //e.g. selftyperef

      if(!m_state.isComplete(nuti)) //reloads
	{
	  std::ostringstream msg;
	  msg << "Incomplete Type for '";
	  if(m_nodeOf)
	    msg << m_nodeOf->getName();
	  else
	    msg << m_state.getUlamTypeNameBriefByIndex(getOfType()).c_str();
	  msg << getName() << "'";
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	  nuti = Hzy;
	}
      else
	{
	  UlamType * nut = m_state.getUlamTypeByIndex(nuti);
	  if(!((nut->getUlamTypeEnum() == Class) || m_state.isAtom(nuti)))
	    {
	      std::ostringstream msg;
	      msg << "Invalid non-class type provided: '";
	      if(m_nodeOf)
		{
		  if(m_nodeOf->isAMemberSelect()) //t3699
		    msg << ((NodeMemberSelect *) m_nodeOf)->getFullName().c_str();
		  else
		    msg << m_nodeOf->getName();
		}
	      else
		msg << m_state.getUlamTypeNameBriefByIndex(getOfType()).c_str();
	      msg << getName() << "'";
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      nuti = Nav;
	    }

	  if(!nut->isScalar())
	    {
	      std::ostringstream msg;
	      msg << "Invalid non-scalar type provided: '";
	      if(m_nodeOf)
		msg << m_nodeOf->getName();
	      else
		msg << m_state.getUlamTypeNameBriefByIndex(getOfType()).c_str();
	      msg << getName() << "'";
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      nuti = Nav;
	    }
	} //complete
    } //ok

  if(m_state.okUTItoContinue(nuti))
    {
      setOfType(nuti); //set here!!
      if(m_nodeOf) //m_token.m_type == TOK_IDENTIFIER)
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
    if(nuti == Hzy) m_state.setGoAgain(); //since not error
    return nuti;
  } //checkAndLabelType

  EvalStatus NodeStorageof::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    // quark or nonclass data member;
    evalNodeProlog(0); //new current node eval frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr (t41507 vs t41033)

    UlamValue uvp = makeUlamValuePtr(); //virtual
    if(!uvp.isPtr()) return evalStatusReturn(ERROR);

    UlamValue uv = m_state.getPtrTarget(uvp);

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeStorageof::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr (t41507 vs t41033)

    // return ptr to this local var (from NodeIdent's makeUlamValuePtr)
    UlamValue rtnUVPtr = makeUlamValuePtr(); //virtual

    if(!rtnUVPtr.isPtr()) return evalStatusReturn(ERROR);

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeStorageof::evalAtomOfExpr()
  {
    UTI nuti = getNodeType();

    assert(m_nodeOf);
    assert(m_state.isAtom(nuti));

    UlamValue ofuv = UlamValue::makeAtom();
    ofuv.setUlamValueEffSelfTypeIdx(m_state.getEmptyElementUTI());

    // need effective self of atom (t3286)
    evalNodeProlog(0); //new current node eval frame pointer

    u32 slots = makeRoomForSlots(1); //always 1 slot for ptr

    EvalStatus evs = m_nodeOf->eval();
    if(evs == NORMAL)
      ofuv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slots);
    evalNodeEpilog();

    return ofuv;
  } //evalAtomOfExpr

  void NodeStorageof::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    depth += m_state.slotsNeeded(getNodeType());

    if(m_nodeOf)
      m_nodeOf->calcMaxDepth(depth, maxdepth, base);
  } //calcMaxDepth

} //end MFM
