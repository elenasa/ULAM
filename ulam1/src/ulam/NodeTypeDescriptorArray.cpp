#include <stdlib.h>
#include "NodeTypeDescriptorArray.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeDescriptorArray::NodeTypeDescriptorArray(Token tokarg, UTI auti, NodeTypeDescriptor * scalarnode, CompilerState & state) : NodeTypeDescriptor(tokarg, auti, state), m_nodeScalar(scalarnode), m_unknownArraysizeSubtree(NULL)
  {
    m_nodeScalar->updateLineage(getNodeNo()); //for unknown subtrees
  }

  NodeTypeDescriptorArray::NodeTypeDescriptorArray(const NodeTypeDescriptorArray& ref) : NodeTypeDescriptor(ref), m_nodeScalar(NULL), m_unknownArraysizeSubtree(NULL)
  {
    if(ref.m_nodeScalar)
      m_nodeScalar = (NodeTypeDescriptor *) ref.m_nodeScalar->instantiate();

    if(ref.m_unknownArraysizeSubtree)
      m_unknownArraysizeSubtree = new NodeSquareBracket(*ref.m_unknownArraysizeSubtree); //mappedUTI?
  }

  NodeTypeDescriptorArray::~NodeTypeDescriptorArray()
  {
    delete m_nodeScalar;
    m_nodeScalar = NULL;

    delete m_unknownArraysizeSubtree;
    m_unknownArraysizeSubtree = NULL;
  } //destructor

  Node * NodeTypeDescriptorArray::instantiate()
  {
    return new NodeTypeDescriptorArray(*this);
  }

  void NodeTypeDescriptorArray::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    assert(m_nodeScalar);
    assert(m_unknownArraysizeSubtree);
    m_nodeScalar->updateLineage(getNodeNo());
    m_unknownArraysizeSubtree->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeTypeDescriptorArray::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeScalar->findNodeNo(n, foundNode))
      return true;
    if(m_unknownArraysizeSubtree->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeDescriptorArray::printPostfix(File * fp)
  {
    fp->write(getName());
  }

  const char * NodeTypeDescriptorArray::getName()
  {
    std::ostringstream nstr;
    nstr << m_nodeScalar->getName();
    nstr << "[]";

    return nstr.str().c_str();
  } //getName

  const std::string NodeTypeDescriptorArray::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeTypeDescriptorArray::linkConstantExpressionArraysize(NodeSquareBracket * ceForArraySize)
  {
    //tfr owner, no dups
    assert(!m_unknownArraysizeSubtree);
    m_unknownArraysizeSubtree = ceForArraySize;
  } //linkConstantExpressionArraysize

  UTI NodeTypeDescriptorArray::checkAndLabelType()
  {
    UTI it = getNodeType();

    if(isReadyType())
      {
	assert(m_state.isComplete(it)); //could it be?
	return it;
      }

    if(resolveType(it))
      {
	UTI scalaruti = m_state.getUlamTypeAsScalar(it);
	UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
	ULAMCLASSTYPE sclasstype = scalarut->getUlamClass();
	if(scalarut->getUlamTypeEnum() == UAtom || sclasstype == UC_ELEMENT)
	  {
	    std::ostringstream msg;
	    msg << "Invalid non-scalar type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ". Requires a custom array"; //we have unpacked arrays now!
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //was ERR
	    //it = Nav;
	  }
	//else
	  {
	    m_ready = true; // set here
	    setNodeType(it);
	  }
      }
    else
      {
	setNodeType(it); //now that we have Hzy; could be Nav
	if(it == Hzy)
	  m_state.setGoAgain();
      }
    return getNodeType();
  } //checkAndLabelType

  bool NodeTypeDescriptorArray::resolveType(UTI& rtnuti)
  {
    if(isReadyType())
      {
	rtnuti = getNodeType();
	return true;
      }

    bool rtnb = false;
    // not node select, we are the array on top of the scalar leaf
    UTI nuti = givenUTI();

    // scalar type
    assert(m_nodeScalar);
    UTI scuti = m_nodeScalar->checkAndLabelType();

    if(m_nodeScalar->isReadyType())
      {
	//check scalar completeness
	if(!m_state.isComplete(scuti))
	  {
	    UTI mappedUTI = scuti;
	    UTI cuti = m_state.getCompileThisIdx();

	    // the symbol associated with this type, was mapped during instantiation
	    // since we're call AFTER that (not during), we can look up our
	    // new UTI and pass that on up the line of NodeType Selects, if any.
	    if(m_state.mappedIncompleteUTI(cuti, scuti, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "Substituting Mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		msg << " for incomplete descriptor for acalar of array: ";
		msg << m_state.getUlamTypeNameBriefByIndex(scuti).c_str();
		msg << "' UTI" << scuti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		scuti = mappedUTI;
	      }

	    if(!m_state.isComplete(scuti)) //reloads to recheck
	      {
		std::ostringstream msg;
		msg << "Incomplete descriptor for scalar of array: ";
		msg << m_state.getUlamTypeNameBriefByIndex(scuti).c_str();
		msg << " UTI" << scuti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	  }

	checkAndMatchBaseUlamTypes(nuti, scuti);

	if(resolveTypeArraysize(nuti, scuti))
	  {
	    rtnb = true;
	    rtnuti = nuti;
	  }
	else
	  rtnuti = nuti; //could be Nav or Hzy
      } //else select not ready, so neither are we!!
    else
      rtnuti = Hzy;
    return rtnb;
  } //resolveType

  bool NodeTypeDescriptorArray::resolveTypeArraysize(UTI& rtnuti, UTI scuti)
  {
    assert(m_unknownArraysizeSubtree);
    s32 as = UNKNOWNSIZE;

    UTI auti = Nouti;
    //array of primitives or classes
    bool rtnb = m_unknownArraysizeSubtree->getArraysizeInBracket(as, auti); //eval
    if(!rtnb)
      {
	rtnuti = Nav;
	return false; //error, e.g. possible divide by zero
      }
    else if(as != UNKNOWNSIZE)
      {
	// keep in case a template
	//delete m_unknownArraysizeSubtree;
	if(!m_state.setUTISizes(rtnuti, m_state.getBitSize(scuti), as)) //update UlamType
	  {
	    rtnuti = Nav;
	    return false;
	  }
      }

    attemptToResolveHolderArrayType(rtnuti, scuti);

    checkAndMatchClassTypes(rtnuti, scuti);

    return (m_state.isComplete(rtnuti)); //repeat if holder or bitsize is still unknown
  } //resolveTypeArraysize

  bool NodeTypeDescriptorArray::attemptToResolveHolderArrayType(UTI auti, UTI buti)
  {
    UlamType *aut = m_state.getUlamTypeByIndex(auti);
    UlamType *but = m_state.getUlamTypeByIndex(buti);
    bool aholder = aut->isHolder();
    bool bholder = but->isHolder();

    bool rtnstat = false; //e.g. both are holders

    if(aholder ^ bholder)
      {
	//if auti or buti is a holder, but not both, update a key
	UlamKeyTypeSignature akey = aut->getUlamKeyTypeSignature();
	UlamKeyTypeSignature bkey = but->getUlamKeyTypeSignature();
	if(aholder)
	  {
	    UlamKeyTypeSignature newkey(bkey.getUlamKeyTypeSignatureNameId(), bkey.getUlamKeyTypeSignatureBitSize(), akey.getUlamKeyTypeSignatureArraySize(), akey.getUlamKeyTypeSignatureClassInstanceIdx());
	    m_state.makeUlamTypeFromHolder(akey, newkey, but->getUlamTypeEnum(), auti);
	  }
	else
	  {
	    UlamKeyTypeSignature newkey(akey.getUlamKeyTypeSignatureNameId(), akey.getUlamKeyTypeSignatureBitSize(), bkey.getUlamKeyTypeSignatureArraySize(), bkey.getUlamKeyTypeSignatureClassInstanceIdx());
	    m_state.makeUlamTypeFromHolder(bkey, newkey, aut->getUlamTypeEnum(), buti);
	  }
	rtnstat = true;
      }
    else if(!aholder && !bholder)
      rtnstat = true; //neither are holders

    return rtnstat;
  } //attemptToResolveHolderArrayType

  void NodeTypeDescriptorArray::checkAndMatchClassTypes(UTI auti, UTI scuti)
  {
    //update class types to match, if necessary
    //e.g. t3436 (unpacked array of elements), t3621 (array of inherited quarks)
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    if(aut->getUlamTypeEnum() == Class)
      {
	ULAMCLASSTYPE aclasstype = aut->getUlamClass();
	UlamType * scut = m_state.getUlamTypeByIndex(scuti);
	ULAMCLASSTYPE sclasstype = scut->getUlamClass();
	if(aclasstype != sclasstype)
	  {
	    ((UlamTypeClass *) aut)->setUlamClass(sclasstype); //could have been unseen array
	    std::ostringstream msg;
	    msg << "Class type of array type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str();
	    msg << " (UTI" << auti << ") set to match its scalar type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(scuti).c_str();
	    msg << " (UTI" << scuti << ")";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }
  } //checkAndMatchClassTypes

  void NodeTypeDescriptorArray::checkAndMatchBaseUlamTypes(UTI auti, UTI scuti)
  {
    // of course, their keys' nameids should be the same (~ enum)!!
    // unless one is a "holder"
    // e.g. t3595 (typedef Unseen wasn't a class at all)
    if((m_state.getUlamTypeByIndex(auti)->getUlamTypeEnum() != m_state.getUlamTypeByIndex(scuti)->getUlamTypeEnum()) && !m_state.isHolder(auti))
      {
	UlamType * scut = m_state.getUlamTypeByIndex(scuti);
	assert(scut->isScalar());

	//create corresponding array type, keep givenUTI (=auti) just change the key
	UlamType * aut = m_state.getUlamTypeByIndex(auti);
	UlamKeyTypeSignature sckey = scut->getUlamKeyTypeSignature();
	UlamKeyTypeSignature newkey(sckey.getUlamKeyTypeSignatureNameId(), aut->getBitSize(), aut->getArraySize(), scuti, aut->getReferenceType());
	m_state.makeUlamTypeFromHolder(newkey, scut->getUlamTypeEnum(), auti);

	std::ostringstream msg;
	msg << "Type of array descriptor: ";
	msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str();
	msg << " (UTI" << auti << ") set to match its scalar type ";
	msg << m_state.getUlamTypeNameBriefByIndex(scuti).c_str();
	msg << " (UTI" << scuti << ")";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
  } //checkAndMatchBaseUlamTypes

  void NodeTypeDescriptorArray::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeTypeDescriptor::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_nodeScalar)
      m_nodeScalar->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);

    if(m_unknownArraysizeSubtree)
      m_unknownArraysizeSubtree->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

} //end MFM
