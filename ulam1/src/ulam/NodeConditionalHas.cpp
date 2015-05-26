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
    //assert(m_state.isScalar(luti));

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClass();
    if(!(luti == UAtom || lclasstype == UC_ELEMENT || lclasstype == UC_QUARK || m_state.isScalar(luti)))
      {
	std::ostringstream msg;
	msg << "Invalid type for LHS of conditional operator '" << getName();
	msg << "'; must be an atom, element or quark, not type: ";
	msg << m_state.getUlamTypeNameByIndex(luti).c_str();
	if(lclasstype == UC_UNSEEN)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType();

    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();
    if(!(rclasstype == UC_QUARK || m_state.isScalar(ruti)))
      {
	std::ostringstream msg;
	msg << "Invalid type for RHS of conditional operator '" << getName();
	msg << "'; must be a quark name, not type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
	msg << " (UTI" << ruti << ")";
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

  EvalStatus  NodeConditionalHas::eval()
  {
    assert(m_nodeLeft);
    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    // DO 'HAS':
    UTI luti = pluv.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = pluv.getPtrTargetType();

    UTI ruti = getRightType();
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
	    msg << "'; Type Not Found: "  << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid type for LHS of conditional operator '" << getName();
	    msg <<  "', "  << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	    msg << "; Passing through as UNFOUND for eval";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }

    bool hasit = (posFound >= 0);
    UlamValue rtnuv = UlamValue::makeImmediate(getNodeType(), (u32) hasit, m_state);

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(rtnuv);

    evalNodeEpilog();
    return evs;
  } //eval

  void NodeConditionalHas::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft);
    UTI nuti = getNodeType();
    UTI ruti = getRightType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //NO need to load lhs into tmp (T)
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType(); //replace

    s32 tmpVarHas = m_state.getNextTmpVarNumber();

    // atom is a special case since we have to learn its element type at runtime
    // before interrogating if it 'has' a particular QuarkName Type.
    if(luti == UAtom)
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32
	fp->write(" "); //e.g. u32
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarHas).c_str());;
	fp->write(" = (");
	//UlamElement<EC> internal method, takes UC, u32 and const char*, returns s32
	fp->write(methodNameForCodeGen().c_str());
	fp->write("(");
	fp->write("uc, ");
	Node::genLocalMemberNameOfMethod(fp); //assume atom is a local var (neither dm nor ep)
	fp->write("read().GetType(), ");
	fp->write("\"");
	fp->write(rut->getUlamTypeMangledName().c_str());
	fp->write("\") >= 0);\n"); //bool as u32
      }
    else //not atom
      {
	UlamType * lut = m_state.getUlamTypeByIndex(luti);
	ULAMCLASSTYPE lclasstype = lut->getUlamClass();

	m_state.indent(fp);
	fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64, etc.
	fp->write(" "); //e.g. u32, s32, u64, etc.
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarHas).c_str());
	fp->write(" = ");

	fp->write("(");
	fp->write(lut->getUlamTypeMangledName().c_str());
	if(lclasstype == UC_ELEMENT)
	  fp->write("<EC>::THE_INSTANCE.");
	else if(lclasstype == UC_QUARK)
	  fp->write("<EC,POS>::");
	else
	  assert(0);

	fp->write(methodNameForCodeGen().c_str()); //mangled
	fp->write("(\"");
	fp->write(rut->getUlamTypeMangledName().c_str());
	fp->write("\") >= 0);\n");
      }
    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarHas, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0); //POS 0 rightjustified (atom-based).
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs
  } //genCode

} //end MFM
