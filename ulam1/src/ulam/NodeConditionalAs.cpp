#include <stdio.h>
#include "NodeConditionalAs.h"
#include "CompilerState.h"

namespace MFM {

  NodeConditionalAs::NodeConditionalAs(Node * leftNode, Token typeTok, CompilerState & state): NodeConditional(leftNode, typeTok, state) {}

  NodeConditionalAs::~NodeConditionalAs()
  {}

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
    return  std::string(m_state.getAsMangledFunctionName(m_nodeLeft->getNodeType(), m_utypeRight));
  }


  UTI NodeConditionalAs::checkAndLabelType()
  {
    assert(m_nodeLeft);
    UTI newType = Bool;

    UTI luti = m_nodeLeft->checkAndLabelType();  //side-effect
    assert(m_state.isScalar(luti));

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClass();
    if(!(luti == UAtom || lclasstype == UC_ELEMENT))
      {
	std::ostringstream msg;
	msg << "Invalid type for LHS of conditional operator '" << getName() << "'; must be an atom, or element, not type: " << m_state.getUlamTypeNameByIndex(luti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    UTI ruti = m_state.getUlamTypeFromToken(m_typeTok, 0, NONARRAYSIZE);  //name-based, sizes ignored
    assert(m_state.isScalar(ruti));

    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();
    if(!(rclasstype == UC_QUARK || rclasstype == UC_ELEMENT))
      {
	std::ostringstream msg;
	msg << "Invalid type for RHS of conditional operator '" << getName() << "'; must be a quark or element name, not type: " << m_state.getUlamTypeNameByIndex(ruti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    m_utypeRight = ruti;

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
    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(m_utypeRight)->getUlamClass();
    if(rclasstype == UC_QUARK)
      {
	UlamType * lut = m_state.getUlamTypeByIndex(luti);
	SymbolClass * csym = NULL;
	s32 posFound = -1;

	if(m_state.alreadyDefinedSymbolClass(lut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) csym)->getClassBlockNode();
	    assert(classNode);
	    posFound = classNode->findUlamTypeInTable(m_utypeRight);
	  }
	else
	  {
	    //atom's don't work in eval, only genCode, let pass as not found.
	    if(luti != UAtom)
	      {
		std::ostringstream msg;
		msg << "Invalid type for LHS of conditional operator '" << getName() << "'; Class Not Found: "  << m_state.getUlamTypeNameByIndex(luti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Invalid type for LHS of conditional operator '" << getName() <<  "', "  << m_state.getUlamTypeNameByIndex(luti).c_str() << "; Passing through as UNFOUND for eval";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	  }

	asit = (posFound >= 0);
      }
    else if(rclasstype == UC_ELEMENT)
      {
	// like 'is'
	// inclusive result for eval purposes (atoms and element types are orthogonal)
	asit = (luti == UAtom || luti == m_utypeRight);
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
    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(m_utypeRight)->getUlamClass();

    if(rclasstype == UC_QUARK)
      return genCodeAsQuark(fp, uvpass);
    else if(rclasstype == UC_ELEMENT)
      return genCodeAsElement(fp, uvpass);
    else
      assert(0);

    return;
  } //genCode


  void NodeConditionalAs::genCodeAsQuark(File * fp, UlamValue& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * rut = m_state.getUlamTypeByIndex(m_utypeRight);
    //ULAMCLASSTYPE rclasstype = rut->getUlamClass();

    s32 tmpVarAs = m_state.getNextTmpVarNumber();

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);  //No need to load lhs into tmp (T); symbol's in COS vector
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType();  //replaces

    // atom is a special case since we have to learn its element type at runtime
    // before interrogating if it 'as' a particular QuarkName Type; return signed pos.
    if(luti == UAtom)
      {
	m_state.indent(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarAs).c_str());;
	fp->write(" = ");
	//UlamElement<CC> internal method, takes uc, u32 and const char*, returns s32
	fp->write(methodNameForCodeGen().c_str());
	fp->write("(");
	fp->write("uc, ");
	Node::genLocalMemberNameOfMethod(fp);  //assume atom is a local var (neither dm nor ep)
	fp->write("read().GetType(), ");
	fp->write("\"");
	fp->write(rut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str());
	fp->write("\");\n");  //keeping pos in tmp
      }
    else  //not an atom
      {
	UlamType * lut = m_state.getUlamTypeByIndex(luti);

	m_state.indent(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarAs).c_str());
	fp->write(" = ");

	fp->write("(");
	fp->write(lut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("<CC>::");

	fp->write(methodNameForCodeGen().c_str());  //mangled-hAs
	fp->write("(\"");
	fp->write(rut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str());
	fp->write("\"));\n");  //keeping pos in tmp
      }

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();
    uvpass = UlamValue::makePtr(tmpVarAs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid);  //POS 0 rightjustified (atom-based).

    //indicate to NodeControl that the value returned in uvpass, still needs to be tested >=0,
    //since its value represents the posoffset (+ FIRSTSTATEBIT) into T (in case of a quark).
    m_state.m_genCodingConditionalAs = true;
    //m_state.m_currentObjSymbolsForCodeGen.clear();  //clear remnant of lhs ???
  } //genCodeAsQuark


  void NodeConditionalAs::genCodeAsElement(File * fp, UlamValue& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UlamType * rut = m_state.getUlamTypeByIndex(m_utypeRight);
    ULAMCLASSTYPE rclasstype = rut->getUlamClass();

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);  //needs to load lhs into tmp (T); symbol's in COS vector
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType();  //replaces

    //wiped out by reading lhs; needed later by auto NodeVarDecl
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_nodeLeft->genCodeReadIntoATmpVar(fp, luvpass);
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector;  //restore COS after read.

    s32 tmpVarNum = luvpass.getPtrSlotIndex();
    s32 tmpVarIs = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64, etc.
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs).c_str());
    fp->write(" = ");

    fp->write(rut->getUlamTypeMangledName(&m_state).c_str());
    if(rclasstype == UC_ELEMENT)
      fp->write("<CC>::THE_INSTANCE.");
    else if(rclasstype == UC_QUARK)
      fp->write("<CC,POS>::");
    else
      assert(0);

    fp->write(methodNameForCodeGen().c_str());  //mangled
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(luti, tmpVarNum).c_str());
    fp->write(");\n");

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();
    uvpass = UlamValue::makePtr(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid);
  } //genCodeAsElement

} //end MFM
