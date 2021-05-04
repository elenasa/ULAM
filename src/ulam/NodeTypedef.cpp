#include <stdlib.h>
#include "NodeTypedef.h"
#include "CompilerState.h"
#include "MapTypedefDesc.h"

namespace MFM {

  NodeTypedef::NodeTypedef(SymbolTypedef * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_typedefSymbol(sym), m_tdid(0), m_currBlockNo(0), m_currBlockPtr(NULL), m_nodeTypeDesc(nodetype)
  {
    if(sym)
      {
	m_tdid = sym->getId();
	m_currBlockNo = sym->getBlockNoOfST();
      }
  }

  NodeTypedef::NodeTypedef(const NodeTypedef& ref) : Node(ref), m_typedefSymbol(NULL), m_tdid(ref.m_tdid), m_currBlockNo(ref.m_currBlockNo), m_currBlockPtr(NULL), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeTypedef::~NodeTypedef()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeTypedef::instantiate()
  {
    return new NodeTypedef(*this);
  }

  void NodeTypedef::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeTypedef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypedef::printPostfix(File * fp)
  {
    //assert(m_typedefSymbol);
    if(m_tdid == m_state.m_pool.getIndexForDataString("Self")) return;
    if(m_tdid == m_state.m_pool.getIndexForDataString("Super")) return;

    UTI tuti = getNodeType();
    if(m_typedefSymbol)
      tuti = m_typedefSymbol->getUlamTypeIdx();
    else if(m_nodeTypeDesc)
      tuti = m_nodeTypeDesc->givenUTI();
   else
     m_state.abortNotImplementedYet();

    UlamKeyTypeSignature tkey = m_state.getUlamKeyTypeSignatureByIndex(tuti);
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);
    ULAMTYPE tetyp = tut->getUlamTypeEnum();

    fp->write(" typedef");

    fp->write(" ");

    fp->write(m_state.getUlamTypeNameBriefByIndex(tuti).c_str());

    if((tetyp != Class) && tut->isAltRefType())
      fp->write(" &"); //an array of refs as written, should be ref to an array.

    fp->write(" ");
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(tuti);
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }
    fp->write("; ");
  } //printPostfix

  void NodeTypedef::noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize)
  {
    return; //bypass
  }

  void NodeTypedef::genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize)
  {
    return; //bypass
  }

  const char * NodeTypedef::getName()
  {
    return m_state.m_pool.getDataAsString(m_tdid).c_str(); //safer
  }

  const std::string NodeTypedef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeTypedef::clearSymbolPtr()
  {
    //if symbol is in a stub, there's no guarantee the stub
    // won't be replace by another duplicate class once its
    // pending args have been resolved.
    m_typedefSymbol = NULL;
    setBlock(NULL);
  }

  bool NodeTypedef::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_typedefSymbol;
    return true;
  } //getSymbolPtr

  UTI NodeTypedef::checkAndLabelType(Node * thisparentnode)
  {
    UTI it = Nav;

    // instantiate, look up in current block
    if(m_typedefSymbol == NULL)
      checkForSymbol();

    if(m_typedefSymbol)
      {
	//m_typedefSymbol is the (rhs) type alias
	it = m_typedefSymbol->getUlamTypeIdx();

	//check for UNSEEN Class' ClassType (e.g. array of UC_QUARK)
	UlamType * tdut = m_state.getUlamTypeByIndex(it);
	if((tdut->getUlamTypeEnum() == Class) && (tdut->getUlamClassType() == UC_UNSEEN))
	  {
	    if(!m_state.completeIncompleteClassSymbolForTypedef(it))
	      {
		std::ostringstream msg;
		msg << "Incomplete Typedef for class type: ";
		msg << m_state.getUlamTypeNameByIndex(it).c_str();
		msg << ", used with variable symbol name '" << getName() << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	      }
	    else
	      m_state.addCompleteUlamTypeToThisContextSet(it); //t41147
	  }
	else if(tdut->isHolder() && !m_state.isThisLocalsFileScope())
	  {
	    m_state.statusUnknownTypeInThisClassResolver(it);
	  }
    assert(m_nodeTypeDesc);
	UTI cuti = m_state.getCompileThisIdx();
	if(m_nodeTypeDesc)
	  {
	    UTI duti = m_nodeTypeDesc->checkAndLabelType(this); //sets goagain if nav
	    if(m_state.okUTItoContinue(duti) && (duti != it))
	      {
		std::ostringstream msg;
		msg << "REPLACING Symbol UTI" << it;
		msg << ", " << m_state.getUlamTypeNameByIndex(it).c_str();
		msg << " used with typedef symbol name '" << getName();
		msg << "' with node type descriptor type: ";
		msg << m_state.getUlamTypeNameByIndex(duti).c_str();
		msg << " UTI" << duti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		//m_state.mapTypesInCurrentClass(it, duti);
		m_typedefSymbol->resetUlamType(duti); //consistent! (must be same ref type)
		//m_state.updateUTIAliasForced(it, duti); //t3379, t3668, t41431
		it = duti;
	      }
	  }

	if(!m_state.okUTItoContinue(it) || !m_state.isComplete(it)) //reloads
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Typedef for type: ";
	    msg << m_state.getUlamTypeNameByIndex(it).c_str();
	    msg << " (UTI " << it << ")";
	    msg << ", used with typedef symbol name '" << getName() << "'";
	    if(m_state.okUTItoContinue(it) || m_state.isStillHazy(it)) //t41288
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		it = Hzy; //t3862
	      }
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      } // got typedef symbol

    setNodeType(it);
    if(it == Hzy)
      {
	clearSymbolPtr();
	m_state.setGoAgain(); //since not error; unlike vardecl
      }
    return getNodeType();
  } //checkAndLabelType

  void NodeTypedef::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    setBlock(currBlock);

    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_tdid, asymptr, hazyKin)) // && !hazyKin) t41446?
      {
	if(asymptr->isTypedef())
	  {
	    m_typedefSymbol = (SymbolTypedef *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) '" << m_state.m_pool.getDataAsString(m_tdid).c_str();
	    msg << "' is not a typedef, and cannot be used as one";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) Typedef '" << m_state.m_pool.getDataAsString(m_tdid).c_str();
	msg << "' is not defined, and cannot be used";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //t41408
      }
    m_state.popClassContext(); //restore
  } //toinstantiate

  NNO NodeTypedef::getBlockNo()
  {
    return m_currBlockNo;
  }

  void NodeTypedef::setupBlockNo()
  {
    //taken from NodeIdent (t41446)?
    //define before used, start search with current block
    if(m_currBlockNo == 0)
      {
	if(m_state.useMemberBlock())
	  {
	    NodeBlockClass * memberclass = m_state.getCurrentMemberClassBlock();
	    assert(memberclass);
	    setBlockNo(memberclass->getNodeNo());
	  }
	else
	  setBlockNo(m_state.getCurrentBlockNo());
      }
  } //setupBlockNo

  void NodeTypedef::setBlockNo(NNO n)
  {
    //assert(n > 0);
    m_currBlockNo = n;
    m_currBlockPtr = NULL; //not owned, just clear
  }

  void NodeTypedef::setBlock(NodeBlock * ptr)
  {
    m_currBlockPtr = ptr;
  }

  NodeBlock * NodeTypedef::getBlock()
  {
    assert(m_currBlockNo);
    if(m_currBlockPtr)
      return m_currBlockPtr;

    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClassOrLocalsScope(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  } //getBlock

  TBOOL NodeTypedef::packBitsInOrderOfDeclaration(u32& offset)
  {
    //do nothing, but override
    return TBOOL_TRUE;
  }

  void NodeTypedef::printUnresolvedVariableDataMembers()
  {
    assert(m_typedefSymbol);
    UTI it = m_typedefSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(it))
      {
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameByIndex(it).c_str() << "> (UTI ";
	msg << it << ") used with typedef symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
      } //not complete
  } //printUnresolvedVariableDataMembers

  void NodeTypedef::printUnresolvedLocalVariables(u32 fid)
  {
    assert(m_typedefSymbol);
    UTI it = m_typedefSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(it))
      {
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameByIndex(it).c_str() << "> (UTI ";
	msg << it << ") used with typedef symbol name '" << getName() << "'";
	msg << " in function: " << m_state.m_pool.getDataAsString(fid);
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
      } //not complete
  } //printUnresolvedLocalVariables

  void NodeTypedef::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  bool NodeTypedef::buildDefaultValue(u32 wlen, BV8K& dvref)
  {
    return true; //pass on
  }

  bool NodeTypedef::buildDefaultValueForClassConstantDefs()
  {
    return true; //pass on
  }

  EvalStatus NodeTypedef::eval()
  {
    assert(m_typedefSymbol);
    return NORMAL;
  } //eval

  void NodeTypedef::genCode(File * fp, UVPass& uvpass)
  {
#if 0
    m_state.indentUlamCode(fp);
    fp->write("//typedef ");

    fp->write(m_state.getUlamTypeByIndex(m_typedefSymbol->getUlamTypeIdx())->getUlamTypeMangledName().c_str()); //for C++
    fp->write(" ");
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(m_typedefSymbol->getUlamTypeIdx());
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }

    fp->write(";"); GCNL;
#endif
  } //genCode

  void NodeTypedef::genCodeConstantArrayInitialization(File * fp)
  {}

  void NodeTypedef::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {}

  void NodeTypedef::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    //for comment purposes (e.g. t3883)
    NodeTypedef * cloneofme = (NodeTypedef *) this->instantiate();
    assert(cloneofme);
    cloneVec.push_back(cloneofme);
  }

  void NodeTypedef::generateTestInstance(File * fp, bool runtest)
  { /* noop */ }

  void NodeTypedef::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {}

  void NodeTypedef::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers)
  {
    assert(m_typedefSymbol);
    TypedefDesc * descptr = new TypedefDesc(m_typedefSymbol, classType, m_state);
    assert(descptr);

    //concat mangled class and parameter names to avoid duplicate keys into map
    std::ostringstream fullMangledName;
    fullMangledName << descptr->m_mangledClassName << "_" << descptr->m_mangledMemberName;
    classmembers.insert(std::pair<std::string, ClassMemberDescHolder>(fullMangledName.str(), ClassMemberDescHolder(descptr)));
  } //addMemberDescriptionToInfoMap

} //end MFM
