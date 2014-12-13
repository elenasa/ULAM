#include <stdio.h>
#include "NodeCast.h"
#include "CompilerState.h"

namespace MFM {

  NodeCast::NodeCast(Node * n, UTI typeToBe, CompilerState & state): NodeUnaryOp(n, state), m_explicit(false)
  {
    setNodeType(typeToBe);
  }

  NodeCast::~NodeCast()
  {
    delete m_node;
    m_node = NULL;
  }


  const char * NodeCast::getName()
  {
    return "cast";
  }


  const std::string NodeCast::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeCast::setExplicitCast()
  {
    m_explicit = true;
  }


  bool NodeCast::isExplicitCast()
  {
    return m_explicit;
  }


  UTI NodeCast::checkAndLabelType()
  {
    // unlike the other nodes, nodecast knows its type at construction time;
    // this is for checking for errors, before eval happens.
    u32 errorsFound = 0;
    UTI tobeType = getNodeType();
    UTI nodeType = m_node->checkAndLabelType(); //user cast
    //ULAMCLASSTYPE tobeClass = m_state.getUlamTypeByIndex(tobeType)->getUlamClass();
    //if(tobeType == Nav || tobeClass != UC_NOTACLASS || tobeType == UAtom)
    if(tobeType == Nav)
      {
	std::ostringstream msg;
	msg << "Cannot cast to type <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	errorsFound++;
      }
    else
      {
	if(!m_state.isScalar(tobeType))
	  {
	    MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts, elena", DEBUG);
	    errorsFound++;

	    if(m_state.isScalar(nodeType))
	      {
		MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Cannot cast scalar into array", ERR);
		errorsFound++;
	      }

	    if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	      {
		MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Array sizes differ", ERR);
		errorsFound++;
	      }
	  }
	else
	  {
	    if(!m_state.isScalar(nodeType))
	      {
		MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Cannot cast array into scalar", ERR);
		errorsFound++;
	      }
	  } // end not scalar errors

	// needs commandline arg..lots of non-explicit warning.
	// reserve for user requested casts; arithmetic operations
	// cast to Int32 all the time causing this to happend often.
	////if(isExplicitCast())
	//  Node::warnOfNarrowingCast(nodeType, tobeType);
      }

    // special case: user casting a quark to an Int;
    if(errorsFound == 0)
      {
	ULAMCLASSTYPE nodeClass = m_state.getUlamTypeByIndex(nodeType)->getUlamClass();
	if(nodeClass == UC_QUARK)
	  {
	    ULAMTYPE tobeTypEnum = m_state.getUlamTypeByIndex(tobeType)->getUlamTypeEnum();
	    if(tobeTypEnum != Int)
	      {
		std::ostringstream msg;
		msg << "Cannot cast quark type <" << m_state.getUlamTypeNameByIndex(nodeType).c_str() << "> to non-Int <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;		//error!
	      }
	    else
	      {
		// update to NodeMemberSelect + NodeFunctionCall
		m_node = makeCastingNode(m_node, tobeType);
	      }
	  }
	else
	  {
	    //if(nodeClass == UC_ELEMENT || nodeClass == UC_INCOMPLETE)
	    if(nodeClass == UC_INCOMPLETE)
	      {
		std::ostringstream msg;
		msg << "Cannot cast type <" << m_state.getUlamTypeNameByIndex(nodeType).c_str() << "> to <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;	    //error!
	      }
	  }
      }

    return getNodeType();
  }


  EvalStatus NodeCast::eval()
  {
    assert(m_node); //has to be

    evalNodeProlog(0); //new current frame pointer
    UTI tobeType = getNodeType();
    UTI nodeType = m_node->getNodeType(); //uv.getUlamValueType()

    makeRoomForNodeType(nodeType);
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //do we believe these to be scalars, only?
    //possibly an array that needs to be casted, per elenemt
    if(m_state.isScalar(tobeType))
      assert(m_state.isScalar(nodeType));
    else
      {
	//both arrays the same dimensions
	//assert(!nodeType->isScalar());
	if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	  {
	    MSG(getNodeLocationAsString().c_str(), "Considering implementing array casts!!!", ERR);
	    assert(0);
	  }
      }

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    if(nodeType != tobeType)
      {
	if(!(m_state.getUlamTypeByIndex(tobeType)->cast(uv, m_state)))
	  {
	    std::ostringstream msg;
	    msg << "Cast problem! Value type <" << m_state.getUlamTypeNameByIndex(uv.getUlamValueTypeIdx()).c_str() << "> failed to be cast as type: <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  }


  UlamValue NodeCast::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    assert(0); // n/a
    return UlamValue();
  }


  void NodeCast::genCode(File * fp, UlamValue& uvpass)
  {
    m_node->genCode(fp, uvpass);
    if(needsACast())
      {
	genCodeReadIntoATmpVar(fp, uvpass);     // cast.
      }
  } //genCode


 void NodeCast::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    m_node->genCodeToStoreInto(fp, uvpass);
    if(needsACast())
      {
	m_node->genCodeReadIntoATmpVar(fp, uvpass);
	genCodeReadIntoATmpVar(fp, uvpass);     // cast.
      }
  }


  void NodeCast::genCodeReadIntoATmpVar(File * fp, UlamValue& uvpass)
  {
    //assert(needsACast());
    // e.g. called by NodeFunctionCall on a NodeTerminal..
    if(!needsACast())
      {
	return m_node->genCodeReadIntoATmpVar(fp, uvpass);
      }

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    UTI vuti = uvpass.getUlamValueTypeIdx();
    bool isTerminal = false;
    s32 tmpVarNum = 0;

   if(vuti == Ptr)
      {
	tmpVarNum = uvpass.getPtrSlotIndex();
	vuti = uvpass.getPtrTargetType();  //replace
      }
    else
      {
	// an immediate terminal value
	isTerminal = true;
      }

   //handle element-atom and atom-element casting differently
    if(nuti == UAtom || vuti == UAtom)
      {
	return genCodeCastAtomAndElement(fp, uvpass);
      }

   UlamType * vut = m_state.getUlamTypeByIndex(vuti);
   //ULAMCLASSTYPE vclasstype = vut->getUlamClass();
   s32 tmpVarCastNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64, etc.
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarCastNum).c_str());
    fp->write(" = ");

    // write the cast method (e.g. _Unsigned32ToInt32, _Int32ToUnary32, etc..)
    fp->write(nut->castMethodForCodeGen(vuti, m_state).c_str());
    fp->write("(");

    if(isTerminal)
      {
	u32 data = uvpass.getImmediateData(m_state);
	char dstr[40];
	vut->getDataAsString(data, dstr, 'z', m_state);
	fp->write(dstr);
      }
    else
      {
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum).c_str());
      }

    fp->write(", ");

    assert(!(nuti == UAtom || vuti == UAtom));
    //LENGTH of node being casted (Uh_AP_mi::LENGTH ?)
    //fp->write(m_state.getBitVectorLengthAsStringForCodeGen(nodetype).c_str());
    fp->write_decimal(m_state.getTotalBitSize(vuti)); //src length

    fp->write(", ");
    fp->write_decimal(m_state.getTotalBitSize(nuti)); //tobe length

    fp->write(")");
    fp->write(";\n");

    //PROBLEM is that funccall checks for 0 nameid to use the tmp var!
    // but then if we don't pass it along Node::genMemberNameForMethod fails..
    if(isTerminal)
      uvpass = UlamValue::makePtr(tmpVarCastNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0); //POS 0 rightjustified.
    else
      uvpass = UlamValue::makePtr(tmpVarCastNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, uvpass.getPtrNameId()); //POS 0 rightjustified; pass along name id
  } //genCodeReadIntoTmp


  void NodeCast::genCodeWriteFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    assert(0);
    genCodeWriteFromATmpVar(fp, luvpass, ruvpass);
  }


  void NodeCast::genCodeCastAtomAndElement(File * fp, UlamValue & uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    UTI vuti = uvpass.getUlamValueTypeIdx();
    s32 tmpVarNum = 0;

    if(vuti == Ptr)
      {
	tmpVarNum = uvpass.getPtrSlotIndex();
	vuti = uvpass.getPtrTargetType();  //replace
      }

    // "downcast" might not be true; compare to be sure the atom is an element "Foo"
    if(vuti == UAtom)
      {
	m_state.indent(fp);
	fp->write("if(!");
	fp->write(nut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("<CC>::");
	fp->write(m_state.getIsMangledFunctionName());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum).c_str());
	fp->write("))\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write("FAIL(BAD_CAST);\n");
	m_state.m_currentIndentLevel--;
      }
    return;  //no change to uvpass, as if no casting
  } //genCodeCastAtomAndElement


  bool NodeCast::needsACast()
  {
    //return true;  //debug

    UTI tobeType = getNodeType();
    UTI nodeType = m_node->getNodeType();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(tobeType)->getUlamTypeEnum();
    ULAMTYPE nodetypEnum = m_state.getUlamTypeByIndex(nodeType)->getUlamTypeEnum();

    // consider user requested first, then size independent;
    // even constant may need casting (e.g. narrowing for saturation)
    // Bool constants require casts to generate "full" true UlamValue.

    return(isExplicitCast() || typEnum != nodetypEnum  || (m_state.getBitSize(tobeType) != m_state.getBitSize(nodeType)) || (nodetypEnum == Bool && m_state.isConstant(nodeType)) );
  } //needsACast

} //end MFM
