#include <stdio.h>
#include "NodeConditionalHas.h"
#include "CompilerState.h"

namespace MFM {

  NodeConditionalHas::NodeConditionalHas(Node * leftNode, NodeTypeDescriptor * classType, CompilerState & state): NodeConditional(leftNode, classType, state) {}

  NodeConditionalHas::NodeConditionalHas(const NodeConditionalHas& ref) : NodeConditional(ref) {}

  NodeConditionalHas::~NodeConditionalHas() {}

  Node * NodeConditionalHas::instantiate()
  {
    return new NodeConditionalHas(*this);
  }

  const char * NodeConditionalHas::getName()
  {
    return "has";
  }

  const std::string NodeConditionalHas::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeConditionalHas::methodNameForCodeGen()
  {
    return  std::string(m_state.getHasMangledFunctionName(m_nodeLeft->getNodeType()));
  }

  UTI NodeConditionalHas::checkAndLabelType()
  {
    assert(m_nodeLeft);
    UTI newType = Bool;

    UTI luti = m_nodeLeft->checkAndLabelType(); //side-effect
    if(luti == Nav)
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '" << getName();
	msg << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
      }

    if(luti == Hzy)
      {
	std::ostringstream msg;
	msg << "Incomplete lefthand type of conditional operator '" << getName();
	msg << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	setNodeType(Hzy);
	m_state.setGoAgain();
	return Hzy; //short-circuit
      }

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClass();
    if(!((lut->getUlamTypeEnum() == UAtom || lclasstype == UC_ELEMENT || lclasstype == UC_QUARK) && lut->isScalar()))
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '" << getName();
	msg << "'; must be an atom, element or quark, not type: ";
	msg << lut->getUlamTypeNameBrief().c_str();
	if(lclasstype == UC_UNSEEN || luti == Hzy)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.setGoAgain();
	    newType = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
      }

    if(!strcmp(m_nodeLeft->getName(), "self"))
      {
	std::ostringstream msg;
	msg << "Invalid lefthand identifier of conditional operator '" << getName();
	msg << "'; Suggest using a variable of type Atom as 'self'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    if(!strcmp(m_nodeLeft->getName(), "atom"))
      {
	std::ostringstream msg;
	msg << "Invalid lefthand identifier of conditional operator '" << getName();
	msg << "'; Suggest using a variable of type Atom as 'atom'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType();
    if(m_state.okUTItoContinue(ruti))
      {
	UlamType * rut = m_state.getUlamTypeByIndex(ruti);
	ULAMCLASSTYPE rclasstype = rut->getUlamClass();
	if(!(rclasstype == UC_QUARK && rut->isScalar()))
	  {
	    std::ostringstream msg;
	    msg << "Invalid righthand type of conditional operator '" << getName();
	    msg << "'; must be a quark name, not type: ";
	    msg << rut->getUlamTypeNameBrief().c_str();
	    if(rclasstype == UC_UNSEEN || ruti == Hzy)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //goagain set
		m_state.setGoAgain();
		newType = Hzy;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		newType = Nav;
	      }
	  }
      }

    if(!m_state.isComplete(ruti))
      {
	std::ostringstream msg;
	msg << "Righthand type of conditional operator '" << getName() << "' ";
	msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
	msg << " is still incomplete while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	m_state.setGoAgain();
	newType = Hzy; //goagain set by nodetypedesc
      }
    setNodeType(newType);
    setStoreIntoAble(false);
    return getNodeType();
  } //checkAndLabelType

  EvalStatus  NodeConditionalHas::eval()
  {
    assert(m_nodeLeft);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);

    // DO 'HAS':
    UTI luti = pluv.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = pluv.getPtrTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    if(lut->getUlamTypeEnum() == UAtom)
      {
	//an atom can be element or quark in eval-land, so let's get specific!
	UlamValue luv = m_state.getPtrTarget(pluv);
	luti = luv.getUlamValueTypeIdx();
	lut = m_state.getUlamTypeByIndex(luti);
      }

    UTI ruti = getRightType();
    SymbolClass * csym = NULL;
    s32 posFound = -1;
    UTI insidecuti;

    if(m_state.alreadyDefinedSymbolClass(luti, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	posFound = classNode->findUlamTypeInTable(ruti, insidecuti);
      }
    else
      {
	//atom's don't work in eval, only genCode, let pass as not found.
	//if(luti != UAtom)
	if(m_state.getUlamTypeByIndex(pluv.getPtrTargetType())->getUlamTypeEnum() != UAtom)
	  {
	    std::ostringstream msg;
	    msg << "Invalid lefthand type of conditional operator '" << getName();
	    msg << "'; Type Not Found during eval: ";
	    msg << lut->getUlamTypeNameBrief().c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid lefthand type of conditional operator '" << getName();
	    msg <<  "', "  << lut->getUlamTypeNameBrief().c_str();
	    msg << "; Passing through as UNFOUND for eval";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }

    bool hasit = (posFound >= 0);
    if(hasit)
      {
	// insidecuti might be the same as ruti, or a sub-class of ruti!!!
	UlamValue ptr = UlamValue::makePtr(pluv.getPtrSlotIndex(), pluv.getPtrStorage(), insidecuti, m_state.determinePackable(ruti), m_state, pluv.getPtrPos() + posFound, pluv.getPtrNameId());
	m_state.m_currentAutoObjPtr = ptr;
	m_state.m_currentAutoStorageType = luti;
      }
    else
      {
	m_state.m_currentAutoObjPtr = UlamValue(); //wipeout
	m_state.m_currentAutoStorageType = Nouti; //clear
      }

    UlamValue rtnuv = UlamValue::makeImmediate(nuti, (u32) hasit, m_state);
    //also copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(rtnuv);

    evalNodeEpilog();
    return evs;
  } //eval

  // use to be quark rhs case of 'as'
  void NodeConditionalHas::genCode(File * fp, UlamValue& uvpass)
  {
    UTI nuti = getNodeType();
    UTI ruti = getRightType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    s32 tmpVarAs = m_state.getNextTmpVarNumber();

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //No need to load lhs into tmp (T); symbol's in COS vector
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType(); //replaces
    UlamType * lut = m_state.getUlamTypeByIndex(luti);

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    // atom is a special case since we have to learn its element type at runtime
    // before interrogating if it 'as' a particular QuarkName Type; return signed pos.
    if(lut->getUlamTypeEnum() == UAtom)
      {
	m_state.indent(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarAs).c_str());;
	fp->write(" = ");
	//UlamElement<EC> internal method, takes uc, u32 and const char*, returns s32
	fp->write(methodNameForCodeGen().c_str());
	fp->write("(");
	fp->write("uc, ");
	Node::genLocalMemberNameOfMethod(fp); //assume atom is a local var (neither dm nor ep)
	if(stgcos->isSelf())
	  fp->write("GetType(), "); //no read for self
	else
	  fp->write("read().GetType(), ");

	fp->write("\"");
	fp->write(rut->getUlamTypeMangledName().c_str());
	fp->write("\");\n"); //keeping pos in tmp
      }
    else  //not an atom
      {
	UlamType * lut = m_state.getUlamTypeByIndex(luti);
	ULAMCLASSTYPE lclasstype = lut->getUlamClass();

	m_state.indent(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarAs).c_str());
	fp->write(" = ");

	fp->write("(");
	fp->write(lut->getUlamTypeMangledName().c_str());
	if(lclasstype == UC_ELEMENT)
	  fp->write("<EC>::THE_INSTANCE.");
	else if(lclasstype == UC_QUARK)
	  fp->write("<EC,POS>::");
	else
	  assert(0);

	fp->write(methodNameForCodeGen().c_str()); //mangled-hAs
	fp->write("(\"");
	fp->write(rut->getUlamTypeMangledName().c_str());
	fp->write("\"));\n"); //keeping pos in tmp
      }

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();
    uvpass = UlamValue::makePtr(tmpVarAs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid); //POS 0 rightjustified (atom-based).

    //indicate to NodeControl that the value returned in uvpass, still needs to be tested >=0,
    //since its value represents the posoffset (+ FIRSTSTATEBIT) into T (in case of a quark).
    m_state.m_genCodingConditionalHas = true;
  } //genCode

} //end MFM
