#include <stdio.h>
#include "NodeBlockContext.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockContext::NodeBlockContext(NodeBlock * prevBlockNode, CompilerState & state): NodeBlock(prevBlockNode, state, NULL), m_classConstantsReady(false), m_nameid(0) {}

  NodeBlockContext::NodeBlockContext(const NodeBlockContext& ref) : NodeBlock(ref), m_classConstantsReady(false), m_nameid(ref.m_nameid)
  {
    ref.copyUlamTypeKeys((NodeBlockContext *) this); //t3959
  }

  NodeBlockContext::~NodeBlockContext()
  {
    m_hasUlamTypes.clear();
  }

  const char * NodeBlockContext::getName()
  {
    return "context";
  }

  const std::string NodeBlockContext::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeBlockContext::setNodeType(UTI uti)
  {
    Node::setNodeType(uti);
    //avoid Hzy and Int (workaround for eval testing)
    if(m_state.okUTItoContinue(uti) && !m_state.isAPrimitiveType(uti))
      {
	m_nameid = m_state.getUlamTypeNameIdByIndex(uti);
      }
  }

  UTI NodeBlockContext::checkAndLabelType(Node * thisparentnode)
  {
    //dup of NodeBlockLocals for now
    UTI savnuti = getNodeType();
    NODE_ASSERT(savnuti != Nouti);

    //possibly empty
    if(m_nodeNext)
        m_nodeNext->checkAndLabelType(this);
    setNodeType(savnuti);
    return savnuti;
  }

  u32 NodeBlockContext::getAllRemainingCulamGeneratedTypedefSymbolsInContext(std::map<u32, Symbol*>& mapref)
  {
    return m_ST.getAllRemainingCulamGeneratedTypedefSymbolsInTable(mapref);
  }


  bool NodeBlockContext::hasStringDataMembers()
  {
    return m_ST.hasUlamTypeSymbolsInTable(String); //btw, does not check superclasses!!!
    //return m_ST.hasADataMemberStringInitValueInClass(getNodeType());
  }

  bool NodeBlockContext::classConstantsReady()
  {
    return m_classConstantsReady;
  }

  void NodeBlockContext::addUlamTypeKeyToSet(UlamKeyTypeSignature key)
  {
    m_hasUlamTypes.insert(key); //only derefs (t3224)
  }

  void NodeBlockContext::copyUlamTypeKeys(NodeBlockContext * toblock) const
  {
    //a step in generating a temporary class for locals filescope
    std::set<UlamKeyTypeSignature, less_than_key>::iterator it = m_hasUlamTypes.begin();
    while(it != m_hasUlamTypes.end())
      {
	UlamKeyTypeSignature key = *it;
	toblock->addUlamTypeKeyToSet(key);
	it++; //t3852
      }
  }

  bool NodeBlockContext::hasUlamTypeKey(UlamKeyTypeSignature key)
  {
    std::set<UlamKeyTypeSignature, less_than_key>::iterator it = m_hasUlamTypes.find(key);
    if(it != m_hasUlamTypes.end())
      {
	return true;
      }
    return false;
  }

  bool NodeBlockContext::hasUlamType(UTI uti)
  {
    UTI deref = m_state.getUlamTypeAsDeref(uti); //e.g. t3224, t3230
    UlamKeyTypeSignature key = m_state.getUlamKeyTypeSignatureByIndex(deref);
    return hasUlamTypeKey(key);
  } //hasUlamType

  bool NodeBlockContext::searchHasAnyUlamTypeASubclassOf(UTI suti)
  {
    bool rtnb = false;
    std::set<UlamKeyTypeSignature, less_than_key>::iterator it = m_hasUlamTypes.begin();
    while(it != m_hasUlamTypes.end())
      {
	UlamKeyTypeSignature key = *it;
	UlamType * ut = NULL;
	AssertBool isDef = m_state.isDefined(key, ut);
	NODE_ASSERT(isDef);
	if(ut->getUlamTypeEnum() == Class)
	  {
	    u32 cuti = key.getUlamKeyTypeSignatureClassInstanceIdx();
	    if(m_state.isClassASubclassOf(cuti, suti))
	      {
		rtnb = true;
		break;
	      }
	  }
	it++;
      } //end while
    return rtnb;
  } //searchHasAnyUlamTypeASubclassOf (unused)

  void NodeBlockContext::genUlamTypeImmediateDefinitions(File * fp)
  {
    //formerly in SymbolClass::genMangledTypesHeaderFile (Tue Mar 28 08:53:34 2017)
    // do we need UrSelf, Empty, or Classes that hold Strings we have seen as immediates?

    // do primitive types before classes so that immediate
    // Quarks/Elements can use them (e.g. immediate index for aref)
    std::set<UlamKeyTypeSignature, less_than_key>::iterator it = m_hasUlamTypes.begin();
    while(it != m_hasUlamTypes.end())
      {
	UlamKeyTypeSignature key = *it;
	UlamType * ut = NULL;
	AssertBool isDef = m_state.isDefined(key, ut);
	NODE_ASSERT(isDef);
	//e.g. skip constants, include atom, references done automatically
	if(ut->needsImmediateType() && (ut->getUlamClassType() == UC_NOTACLASS))
	  {
	    ut->genUlamTypeMangledAutoDefinitionForC(fp); //references
	    ut->genUlamTypeMangledDefinitionForC(fp);
	  }
	it++;
      }

    //same except now for user defined Class types:
    it = m_hasUlamTypes.begin();
    while(it != m_hasUlamTypes.end())
      {
	UlamKeyTypeSignature key = *it;
	UlamType * ut = NULL;
	AssertBool isDef = m_state.isDefined(key, ut);
	NODE_ASSERT(isDef);
	if(ut->needsImmediateType() && (ut->getUlamClassType() != UC_NOTACLASS))
	  {
	    ut->genUlamTypeMangledAutoDefinitionForC(fp); //references
	    ut->genUlamTypeMangledDefinitionForC(fp);
	  }
	it++;
      }
  } //genUlamTypeImmediateDefinitions

} //end MFM
