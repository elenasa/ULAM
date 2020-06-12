#include <stdlib.h>
#include "NodeModelParameterDef.h"
#include "NodeTerminal.h"
#include "CompilerState.h"
#include "MapParameterDesc.h"

namespace MFM {

  NodeModelParameterDef::NodeModelParameterDef(SymbolModelParameterValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeConstantDef(symptr, nodetype, state) {}

  NodeModelParameterDef::NodeModelParameterDef(const NodeModelParameterDef& ref) : NodeConstantDef(ref) {}

  NodeModelParameterDef::~NodeModelParameterDef() {}

  Node * NodeModelParameterDef::instantiate()
  {
    return new NodeModelParameterDef(*this);
  }

  void NodeModelParameterDef::printPostfix(File * fp)
  {
    //in case the node belongs to the template, use the symbol uti, o.w. 0Nav.
    UTI suti = m_constSymbol ? m_constSymbol->getUlamTypeIdx() : getNodeType();
    fp->write("parameter");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(suti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(m_cid).c_str());

    if(m_nodeExpr)
      {
	fp->write(" =");
	m_nodeExpr->printPostfix(fp);
      }
    fp->write("; ");
  }

  const std::string NodeModelParameterDef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeModelParameterDef::fixPendingArgumentNode()
  {
    m_state.abortShouldntGetHere();
  }

  UTI NodeModelParameterDef::checkAndLabelType()
  {
    UTI nodeType = NodeConstantDef::checkAndLabelType();
    if(m_state.okUTItoContinue(nodeType))
      {
	UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
	u32 wordsize = nut->getTotalWordSize();
	if(wordsize > MAXBITSPERINT)
	  {
	    std::ostringstream msg;
	    msg << "Model Parameter '" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "' must fit in " << MAXBITSPERINT << " bits";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    nodeType = Nav;
	    setNodeType(Nav);
	  }
      }
    return nodeType;
  } //checkAndLabelType

  bool NodeModelParameterDef::buildDefaultValue(u32 wlen, BV8K& dvref)
  {
    return true;
  }

  bool NodeModelParameterDef::buildDefaultValueForClassConstantDefs()
  {
    return true;
  }

  void NodeModelParameterDef::checkForSymbol()
  {
    assert(!m_constSymbol);

    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_cid, asymptr, hazyKin) && !hazyKin)
      {
	if(asymptr->isModelParameter())
	  {
	    m_constSymbol = (SymbolModelParameterValue *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) '" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "' is not a model parameter, and cannot be used as one";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) Model Parameter '" << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "' is not defined, and cannot be used";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

  // called during parsing rhs of named model parameter constant;
  // Requires a constant expression, else error;
  // (scope of eval is based on the block of const def.)
  // ulam-4 refactored for 32-bit platforms.
  UTI NodeModelParameterDef::foldConstantExpression()
  {
    UTI uti = getNodeType();

    if(uti == Nav)
      return Nav;

    if(!m_state.isComplete(uti)) //not complete includes Hzy
      return Hzy; //e.g. not a constant; total word size (below) requires completeness

    assert(m_constSymbol);
    if(isReadyConstant())
      return uti;

    assert(!m_state.isConstantRefType(uti));
    assert(m_state.isScalar(uti));
    assert(m_nodeExpr);
    assert(!m_state.isAClass(uti));

    // MP must be a primitive constant..
    u64 newconst = 0; //UlamType format (not sign extended)

    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(uti); //offset a constant expression

    EvalStatus evs = m_nodeExpr->eval();
    UlamValue cnstUV;
    if( evs == NORMAL)
      cnstUV = m_state.m_nodeEvalStack.popArg();

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
	msg << "', is erronous while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
	msg << "', is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	return Hzy;
      }

    //t3403, t3490
    //insure constant value fits in its declared type
    // no saturation without an explicit cast.
    FORECAST scr = m_nodeExpr->safeToCastTo(uti);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for model parameter '";
	msg << getName() << "' is not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	if(scr == CAST_BAD)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return Nav;
	  }
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	return Hzy; //necessary if not just a warning.
      }

    //cast first, also does safeCast
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    if(ut->cast(cnstUV, uti))
      {
	u32 wordsize = m_state.getTotalWordSize(uti);
	if(wordsize == MAXBITSPERINT)
	  newconst = cnstUV.getImmediateData(m_state);
	else if(wordsize == MAXBITSPERLONG)
	  newconst = cnstUV.getImmediateDataLong(m_state);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }
    else
      {
	std::ostringstream msg;
	msg << "Constant value expression for model parameter '";
	msg << getName() << "', was not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    // BUT WHY when Symbol is all we need/want? because it indicates
    // there's a default value before c&l (see SCNT::getTotalParametersWithDefaultValues) (t3526)
    //then do the surgery
    NodeTerminal * newnode;
    ULAMTYPE etyp = m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum();
    if(etyp == Int)
      newnode = new NodeTerminal((s64) newconst, uti, m_state);
    else
      newnode = new NodeTerminal(newconst, uti, m_state);
    newnode->setNodeLocation(getNodeLocation());
    delete m_nodeExpr;
    m_nodeExpr = newnode;

    BV8K bvtmp;
    u32 len = m_state.getTotalBitSize(uti);
    bvtmp.WriteLong(0u, len, newconst); //is newconst packed?
    m_constSymbol->setValue(bvtmp); //isReady now! (e.g. ClassArgument, ModelParameter)

    return uti; //ok
  } //foldConstantExpression

  void NodeModelParameterDef::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_constSymbol->isModelParameter());

    UTI vuti = m_constSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indentUlamCode(fp);

    fp->write(vut->getImmediateModelParameterStorageTypeAsString().c_str());
    fp->write("<EC>");
    fp->write(" ");
    fp->write(m_constSymbol->getMangledName().c_str());

    fp->write("; //model parameter"); GCNL;
  } //genCode

  void NodeModelParameterDef::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers)
  {
    assert(m_constSymbol);
    assert(m_constSymbol->isReady());

    ParameterDesc * descptr = new ParameterDesc((SymbolModelParameterValue *) m_constSymbol, classType, m_state);
    assert(descptr);

    //replace m_memberName with Ulam Type and Name
    std::ostringstream mnstr;
    if(m_nodeTypeDesc)
      mnstr << m_state.m_pool.getDataAsString(m_nodeTypeDesc->getTypeNameId()).c_str();
    else
      mnstr << m_state.getUlamTypeNameBriefByIndex(getNodeType()).c_str();
    mnstr << " " << descptr->m_memberName;

    descptr->m_memberName = ""; //clear base init
    descptr->m_memberName = mnstr.str();

    //concat mangled class and parameter names to avoid duplicate keys into map
    std::ostringstream fullMangledName;
    fullMangledName << descptr->m_mangledClassName << "_" << descptr->m_mangledMemberName;
    classmembers.insert(std::pair<std::string, ClassMemberDescHolder>(fullMangledName.str(), ClassMemberDescHolder(descptr)));
  } //addMemberDescriptionToInfoMap

} //end MFM
