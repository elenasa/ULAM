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

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClass();
    if(!((luti == UAtom || lclasstype == UC_ELEMENT) && m_state.isScalar(luti)))
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '" << getName();
	msg << "'; must be an atom or element, not ";
	msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	if(lclasstype == UC_UNSEEN || luti == Nav)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    if(!strcmp(m_nodeLeft->getName(), "self"))
      {
	std::ostringstream msg;
	msg << "Invalid lefthand identifier of conditional operator '" << getName();
	msg << "'; Suggest using a variable of type Atom as 'self'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType();

    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();
    if(!((rclasstype == UC_QUARK || rclasstype == UC_ELEMENT) && m_state.isScalar(ruti)))
      {
	std::ostringstream msg;
	msg << "Invalid righthand type of conditional operator '" << getName();
	msg << "'; must be a quark or element name, not ";
	msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
	if(rclasstype == UC_UNSEEN || ruti == Nav)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //goagain set
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    if(!m_state.getUlamTypeByIndex(ruti)->isComplete())
      {
	std::ostringstream msg;
	msg << "Righthand type of conditional operator '" << getName() << "' ";
	msg << m_state.getUlamTypeNameByIndex(ruti).c_str();
	msg << " is still incomplete while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	newType = Nav; //goagain set by nodetypedesc
      }

    setNodeType(newType);
    setStoreIntoAble(false);
    return getNodeType();
  } //checkAndLabelType

  EvalStatus  NodeConditionalAs::eval()
  {
    assert(m_nodeLeft);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    evalNodeProlog(0);   //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    // DO 'AS': rtype quark 'is' related (was 'has'); rtype element 'is'
    UTI luti = pluv.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = pluv.getPtrTargetType();

    bool asit = false;
    UTI ruti = getRightType();
    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();
    if(rclasstype == UC_QUARK)
      {
	if(m_state.isClassASuperclassOf(luti, ruti))
	  {
	    asit = true;
	  }
	else
	  {
	    //atom's don't work in eval, only genCode, let pass as not found.
	    if(luti != UAtom)
	      {
		std::ostringstream msg;
		msg << "Invalid lefthand type of conditional operator '" << getName();
		msg << "'; Class ";
		msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
		msg << " Not Found during eval";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Unsupported lefthand type of conditional operator '" << getName();
		msg <<  "', "  << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
		msg << "; Passing through as UNFOUND for eval";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	  }
      }
    else if(rclasstype == UC_ELEMENT)
      {
	// like 'is'
	// inclusive result for eval purposes (atoms and element types are orthogonal)
	asit = (luti == UAtom || luti == ruti);
      }

    UlamValue rtnuv = UlamValue::makeImmediate(nuti, (u32) asit, m_state);
    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(rtnuv);

    evalNodeEpilog();
    return evs;
  } //eval

  void NodeConditionalAs::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft);
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

    if(rclasstype == UC_ELEMENT)
      {
	fp->write(rut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::THE_INSTANCE.");
	fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(luti, tmpVarNum).c_str());
	fp->write(");\n");
      }
    // not possible!! we already know rhs is an element
    else if(rclasstype == UC_QUARK)
      {
	UlamType * lut = m_state.getUlamTypeByIndex(luti);
	ULAMCLASSTYPE lclasstype = lut->getUlamClass();
	if(lclasstype == UC_ELEMENT)
	  {
	    fp->write(lut->getUlamTypeMangledName().c_str());
	    fp->write("<EC>::THE_INSTANCE.");
	    // uses UlamElement isMethod to de-reference a lhs atom
	    fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	    fp->write("("); //one arg
	  }
	else
	  {
	    fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	    fp->write("(uc, ");
	    fp->write(m_state.getTmpVarAsString(luti, tmpVarNum).c_str());
	    fp->write(".GetType(), "); //from tmpvar T
	  }
	fp->write("\"");
	fp->write(rut->getUlamTypeMangledName().c_str());
	fp->write("\");\n");
      }
    else
      assert(0);

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();

    uvpass = UlamValue::makePtr(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid);
  } //genCode

} //end MFM
