#include <stdlib.h>
#include "NodePositionofRef.h"
#include "CompilerState.h"

namespace MFM {

  NodePositionofRef::NodePositionofRef(Node * membernode, CompilerState & state) : Node(state), m_nodeOf(membernode)
  { }

  NodePositionofRef::NodePositionofRef(const NodePositionofRef& ref) : Node(ref), m_nodeOf(NULL)
  {
    if(ref.m_nodeOf)
      m_nodeOf = ref.m_nodeOf->instantiate();
  }

  NodePositionofRef::~NodePositionofRef()
  {
    delete m_nodeOf;
    m_nodeOf = NULL;
  }

  Node * NodePositionofRef::instantiate()
  {
    return new NodePositionofRef(*this);
  }

  void NodePositionofRef::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeOf)
      m_nodeOf->updateLineage(getNodeNo());
  } //updateLineage

  bool NodePositionofRef::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeOf == oldnptr)
      {
	m_nodeOf = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodePositionofRef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeOf && m_nodeOf->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodePositionofRef::printPostfix(File * fp)
  {
    fp->write(" ");
    if(m_nodeOf)
      fp->write(m_nodeOf->getName());
    else
      fp->write("REF");
    fp->write(" positionof");
  } //printPostfix

  const char * NodePositionofRef::getName()
  {
    u32 sidx = getNameId();
    return m_state.m_pool.getDataAsString(sidx).c_str();
  }

  u32 NodePositionofRef::getNameId()
  {
    std::ostringstream fullnm;
    if(m_nodeOf)
      fullnm << m_nodeOf->getName();
    else
      fullnm << "REF";
    fullnm << ".positionof";

    u32 sidx = m_state.m_pool.getIndexForDataString(fullnm.str());
    return sidx;
  }

  const std::string NodePositionofRef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodePositionofRef::isAConstant()
  {
    return false;
  }

  FORECAST NodePositionofRef::safeToCastTo(UTI newType)
  {
    return m_state.getUlamTypeByIndex(newType)->safeCast(Node::getNodeType());
  } //safeToCastTo

  UTI NodePositionofRef::checkAndLabelType(Node * thisparentnode)
  {
    UTI ofuti = m_nodeOf->checkAndLabelType(this);
    if(m_state.okUTItoContinue(ofuti))
      {
	std::ostringstream msg;
	msg << "Determined type for member '";
	msg << m_nodeOf->getName();
	msg << "' as type: ";
	msg << m_state.getUlamTypeNameByIndex(ofuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    else
      {
	std::ostringstream msg;
	msg << "Undetermined type for missing member '";
	msg << m_nodeOf->getName();
	msg << "'";
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

    if(!m_state.isComplete(ofuti)) //reloads to recheck
      {
	std::ostringstream msg;
	msg << "Incomplete member for type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(ofuti).c_str();
	if(m_nodeOf)
	  {
	    msg << ", of member '";
	    msg << m_nodeOf->getName() << "'";
	  }
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	setNodeType(Hzy); //missing
	m_state.setGoAgain(); //too
	return Hzy;
      }

    setNodeType(Unsigned);
    return getNodeType(); //updated to Unsigned, hopefully
  } //checkandLabelType

  void NodePositionofRef::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavNodes

  EvalStatus NodePositionofRef::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    EvalStatus evs = UNEVALUABLE; //punt for now
    return evs;
  } //eval

  void NodePositionofRef::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeOf);
    UTI nuti = getNodeType();
    UVPass ofpass;
    m_nodeOf->genCodeToStoreInto(fp, ofpass);

    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    assert(cosSize > 0);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    assert(stgcosut->isReference()); //non-refs handled in NodeTerminalProxy


    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(m_state.getUlamTypeByIndex(nuti)->getTmpStorageTypeAsString().c_str()); //u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(ASCII, tmpVarNum, TMPREGISTER).c_str());
    fp->write(" = ");

    if(cosSize == 1)
      {
	//case: ref.positionof
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".GetPos();"); GCNL;
      }
    else
      {
	//case: ref.?.datamember
	if(cos->isDataMember())
	  {
	    UTI dmclassuti = cos->getDataMemberClass();

	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetEffectiveSelf()->");
	    fp->write(m_state.getGetRelPosMangledFunctionName(dmclassuti));
	    fp->write("(");
	    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(dmclassuti)); //efficiency
	    fp->write("u ");
	    fp->write("/*");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(dmclassuti).c_str());
	    fp->write("*/");
	    fp->write(") + ");
	    fp->write_decimal(cos->getPosOffset());
	    fp->write("/*");
	    fp->write(m_state.m_pool.getDataAsString(cos->getId()).c_str());
	    fp->write("*/ + ");
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetEffectiveSelfPos();");
	    GCNL;
	  }
	else
	  {
	    s32 BTidx = Node::isCurrentObjectsContainingABaseTypeTmpSymbol();

	    if(BTidx > 0)
	      {
		//case: ref.BASETYPE.positionof
		SymbolTmpVar * tmpsym = (SymbolTmpVar *) m_state.m_currentObjSymbolsForCodeGen[BTidx];
		UTI BTuti = tmpsym->getUlamTypeIdx();

		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetEffectiveSelf()->");
		fp->write(m_state.getGetRelPosMangledFunctionName(BTuti));
		fp->write("(");
		fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(BTuti)); //efficiency
		fp->write("u ");
		fp->write("/* ");
		fp->write(m_state.getUlamTypeNameBriefByIndex(BTuti).c_str());
		fp->write(" */");
		fp->write(") + ");
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetEffectiveSelfPos();"); GCNL;
	      }
	    else
	      m_state.abortNotImplementedYet(); //t41616
	  }
      }

    uvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).
    m_state.clearCurrentObjSymbolsForCodeGen();
  }

  void NodePositionofRef::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    return;
  }

} //end MFM
