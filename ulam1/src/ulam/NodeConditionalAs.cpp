#include <stdio.h>
#include "NodeConditionalAs.h"
#include "CompilerState.h"

namespace MFM {

  NodeConditionalAs::NodeConditionalAs(Node * leftNode, NodeTypeDescriptor * classType, CompilerState & state): NodeConditional(leftNode, classType, state) {}

  NodeConditionalAs::NodeConditionalAs(const NodeConditionalAs& ref) : NodeConditional(ref) {}

  NodeConditionalAs::~NodeConditionalAs() {}

  Node * NodeConditionalAs::instantiate()
  {
    return new NodeConditionalAs(*this);
  }

  const char * NodeConditionalAs::getName()
  {
    return "as";
  }

  const std::string NodeConditionalAs::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeConditionalAs::methodNameForCodeGen()
  {
    return  std::string(m_state.getAsMangledFunctionName(m_nodeLeft->getNodeType(), getRightType()));
  }

  UTI NodeConditionalAs::checkAndLabelType()
  {
    assert(m_nodeLeft);
    UTI newType = Bool;

    UTI luti = m_nodeLeft->checkAndLabelType(); //side-effect
    assert(m_state.isScalar(luti));

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClass();
    if(!(luti == UAtom || lclasstype == UC_ELEMENT))
      {
	std::ostringstream msg;
	msg << "Invalid type for LHS of conditional operator '" << getName();
	msg << "'; must be an atom, or element, not type: ";
	msg << m_state.getUlamTypeNameByIndex(luti).c_str();
	if(lclasstype == UC_UNSEEN)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType();

    assert(m_state.isScalar(ruti));

    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();
    if(!(rclasstype == UC_QUARK || rclasstype == UC_ELEMENT))
      {
	std::ostringstream msg;
	msg << "Invalid type for RHS of conditional operator '" << getName();
	msg << "'; must be a quark or element name, not type: ";
	msg << m_state.getUlamTypeNameByIndex(ruti).c_str();
	if(rclasstype == UC_UNSEEN || ruti == Nav)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    if(!m_state.getUlamTypeByIndex(ruti)->isComplete())
      {
	std::ostringstream msg;
	msg << "RHS of conditional operator '" << getName() << "' type: ";
	msg << m_state.getUlamTypeNameByIndex(ruti).c_str();
	msg << "; is still incomplete while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	newType = Nav;
      }

    setNodeType(newType);
    setStoreIntoAble(false);
    return getNodeType();
  } //checkAndLabelType

  EvalStatus  NodeConditionalAs::eval()
  {
    assert(m_nodeLeft);

    evalNodeProlog(0);   //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    // DO 'AS': rtype quark 'has'; rtype element 'is'
    UTI luti = pluv.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = pluv.getPtrTargetType();

    bool asit;
    UTI ruti = getRightType();
    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();
    if(rclasstype == UC_QUARK)
      {
	SymbolClass * csym = NULL;
	s32 posFound = -1;

	if(m_state.alreadyDefinedSymbolClass(luti, csym))
	  {
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    posFound = classNode->findUlamTypeInTable(ruti);
	  }
	else
	  {
	    //atom's don't work in eval, only genCode, let pass as not found.
	    if(luti != UAtom)
	      {
		std::ostringstream msg;
		msg << "Invalid type for LHS of conditional operator '" << getName();
		msg << "'; Class Not Found: "  << m_state.getUlamTypeNameByIndex(luti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Invalid type for LHS of conditional operator '" << getName();
		msg <<  "', "  << m_state.getUlamTypeNameByIndex(luti).c_str();
		msg << "; Passing through as UNFOUND for eval";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	  }
	asit = (posFound >= 0);
      }
    else if(rclasstype == UC_ELEMENT)
      {
	// like 'is'
	// inclusive result for eval purposes (atoms and element types are orthogonal)
	asit = (luti == UAtom || luti == ruti);
      }

    UlamValue rtnuv = UlamValue::makeImmediate(getNodeType(), (u32) asit, m_state);
    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(rtnuv);

    evalNodeEpilog();
    return evs;
  } //eval

  void NodeConditionalAs::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft);
    UTI ruti = getRightType();
    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();

    if(rclasstype == UC_QUARK)
      return genCodeAsQuark(fp, uvpass);
    else if(rclasstype == UC_ELEMENT)
      return genCodeAsElement(fp, uvpass);
    else
      assert(0);
  } //genCode

  void NodeConditionalAs::genCodeAsQuark(File * fp, UlamValue& uvpass)
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

    // atom is a special case since we have to learn its element type at runtime
    // before interrogating if it 'as' a particular QuarkName Type; return signed pos.
    if(luti == UAtom)
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
	fp->write("read().GetType(), ");
	fp->write("\"");
	fp->write(rut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str());
	fp->write("\");\n"); //keeping pos in tmp
      }
    else  //not an atom
      {
	UlamType * lut = m_state.getUlamTypeByIndex(luti);

	m_state.indent(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarAs).c_str());
	fp->write(" = ");

	fp->write("(");
	fp->write(lut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::");

	fp->write(methodNameForCodeGen().c_str()); //mangled-hAs
	fp->write("(\"");
	fp->write(rut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str());
	fp->write("\"));\n"); //keeping pos in tmp
      }

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();
    uvpass = UlamValue::makePtr(tmpVarAs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid); //POS 0 rightjustified (atom-based).

    //indicate to NodeControl that the value returned in uvpass, still needs to be tested >=0,
    //since its value represents the posoffset (+ FIRSTSTATEBIT) into T (in case of a quark).
    m_state.m_genCodingConditionalAs = true;
    //m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs ???
  } //genCodeAsQuark

  void NodeConditionalAs::genCodeAsElement(File * fp, UlamValue& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UTI ruti = getRightType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    ULAMCLASSTYPE rclasstype = rut->getUlamClass();

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //needs to load lhs into tmp (T); symbol's in COS vector
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType(); //replaces

    //wiped out by reading lhs; needed later by auto NodeVarDecl
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_nodeLeft->genCodeReadIntoATmpVar(fp, luvpass);
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore COS after read.

    s32 tmpVarNum = luvpass.getPtrSlotIndex();
    s32 tmpVarIs = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64, etc.
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs).c_str());
    fp->write(" = ");

    fp->write(rut->getUlamTypeMangledName().c_str());
    if(rclasstype == UC_ELEMENT)
      fp->write("<EC>::THE_INSTANCE.");
    else if(rclasstype == UC_QUARK)
      fp->write("<EC,POS>::");
    else
      assert(0);

    fp->write(methodNameForCodeGen().c_str()); //mangled
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(luti, tmpVarNum).c_str());
    fp->write(");\n");

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();
    uvpass = UlamValue::makePtr(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid);
  } //genCodeAsElement

} //end MFM
