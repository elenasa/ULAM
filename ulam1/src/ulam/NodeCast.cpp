#include <stdio.h>
#include "NodeCast.h"
#include "CompilerState.h"

namespace MFM {

  NodeCast::NodeCast(Node * n, UTI typeToBe, NodeTypeDescriptor * nodetype, CompilerState & state): NodeUnaryOp(n, state), m_explicit(false), m_nodeTypeDesc(nodetype)
  {
    setNodeType(typeToBe);
  }

  NodeCast::NodeCast(const NodeCast& ref) : NodeUnaryOp(ref), m_explicit(ref.m_explicit), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeCast::~NodeCast()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeCast::instantiate()
  {
    return new NodeCast(*this);
  }

  void NodeCast::updateLineage(NNO pno)
  {
    NodeUnaryOp::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeCast::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeUnaryOp::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  const char * NodeCast::getName()
  {
    return "cast";
    //return m_node->getName();
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

  bool NodeCast::isReadyConstant()
  {
    return m_node->isReadyConstant(); //needs constant folding
  }

  UTI NodeCast::checkAndLabelType()
  {
    // unlike the other nodes, nodecast knows its type at construction time;
    // this is for checking for errors, before eval happens.
    u32 errorsFound = 0;
    UTI tobeType = getNodeType();
    UTI nodeType = m_node->checkAndLabelType(); //user cast

    if(m_nodeTypeDesc)
      {
	//might be a mapped uti for instantiated template class
	tobeType = m_nodeTypeDesc->checkAndLabelType();
	setNodeType(tobeType); //overrides type set at parse time
	if(!m_nodeTypeDesc->isReadyType())
	  {
	    std::ostringstream msg;
	    msg << "Cannot cast to nonready type: " ;
	    msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    msg << " (UTI" << tobeType << ")";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    errorsFound++;
	  }
      }

    if(!m_state.isComplete(tobeType))
      {
	std::ostringstream msg;
	msg << "Cannot cast to incomplete type: " ;
	msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	msg << " (UTI" << tobeType << ")";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	errorsFound++;
      }
    else if(tobeType == Nav)
      {
	std::ostringstream msg;
	msg << "Cannot cast to type: " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	errorsFound++;
      }
    else if(m_state.getUlamTypeByIndex(tobeType)->getUlamClass() != UC_NOTACLASS && m_state.getUlamTypeByIndex(nodeType)->getUlamClass() == UC_NOTACLASS)
      {
	if(nodeType != UAtom)
	  {
	    std::ostringstream msg;
	    msg << "Cannot cast ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
	    msg << " (UTI" << nodeType << ")";
	    msg << " to type: " << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    msg << " (UTI" << tobeType << ")";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    errorsFound++;
	  }
      }

    if(errorsFound == 0) //else
      {
	if(!m_state.isScalar(tobeType))
	  {
	    MSG(getNodeLocationAsString().c_str(),
		"Array casts currently not supported", ERR);
	    errorsFound++;

	    if(m_state.isScalar(nodeType))
	      {
		MSG(getNodeLocationAsString().c_str(),
		    "Consider implementing array casts: Cannot cast scalar into array", ERR);
		errorsFound++;
	      }
	    else if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	      {
		MSG(getNodeLocationAsString().c_str(),
		    "Consider implementing array casts: Array sizes differ", ERR);
		errorsFound++;
	      }
	  }
	else
	  {
	    //to be scalar type
	    if(!m_state.isScalar(nodeType))
	      {
		MSG(getNodeLocationAsString().c_str(),
		    "Consider implementing array casts: Cannot cast array into scalar", ERR);
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
		msg << "Cannot cast quark type: " << m_state.getUlamTypeNameByIndex(nodeType).c_str();
		msg << " to non-Int type: " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;
	      }
	    else
	      {
		// update to NodeMemberSelect + NodeFunctionCall
		//m_node = makeCastingNode(m_node, tobeType);
		if(!makeCastingNode(m_node, tobeType, m_node))
		  errorsFound++;
	      }
	  }
	else
	  {
	    //classes are not surprisingly unknown bit sizes at this point
	    if(nodeClass == UC_UNSEEN)
	      {
		std::ostringstream msg;
		msg << "Cannot cast type: " << m_state.getUlamTypeNameByIndex(nodeType).c_str();
		msg << " to: " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;
	      }
	  }
      }

    if(errorsFound)
      return Nav; //inconsistent! but keeps cast type..makeCastingNode returns error

    return getNodeType();
  } //checkAndLabelType

  void NodeCast::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_node->countNavNodes(cnt);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavNodes(cnt);
  } //countNavNodes

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
      {
	if(!m_state.isScalar(nodeType))
	  {
	    std::ostringstream msg;
	    msg << "Cannot cast an array: ";
	    msg << m_state.getUlamTypeNameByIndex(nodeType).c_str();
	    msg << " to a scalar type: " ;
	    msg << m_state.getUlamTypeNameByIndex(tobeType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	//both arrays the same dimensions
	if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	  {
	    MSG(getNodeLocationAsString().c_str(), "Considering implementing array casts!!!", ERR);
	  }
      }

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    if(nodeType != tobeType)
      {
	if(!(m_state.getUlamTypeByIndex(tobeType)->cast(uv, tobeType)))
	  {
	    std::ostringstream msg;
	    msg << "Cast problem! Value type: ";
	    msg << m_state.getUlamTypeNameByIndex(uv.getUlamValueTypeIdx()).c_str();
	    msg << " failed to be cast as type: " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    evalNodeEpilog();
	    return ERROR;
	  }
      }

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

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
	genCodeReadIntoATmpVar(fp, uvpass); // cast.
      }
  } //genCode

 void NodeCast::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    m_node->genCodeToStoreInto(fp, uvpass);
    if(needsACast())
      {
	m_node->genCodeReadIntoATmpVar(fp, uvpass);
	genCodeReadIntoATmpVar(fp, uvpass); // cast.
      }
  }

  void NodeCast::genCodeReadIntoATmpVar(File * fp, UlamValue& uvpass)
  {
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
	vuti = uvpass.getPtrTargetType(); //replace
      }
    else
      {
	// an immediate terminal value
	isTerminal = true;
      }

   if(nuti == vuti)
     return; //nothing to do!

   UlamType * vut = m_state.getUlamTypeByIndex(vuti); //after vuti replacement
   ULAMCLASSTYPE nclasstype = nut->getUlamClass();
   ULAMCLASSTYPE vclasstype = vut->getUlamClass();

   //handle element-atom and atom-element casting differently
   // handle element->quark, atom->quark, not quark->element or quark->atom
   //    if(nuti == UAtom || vuti == UAtom)
   if(nuti == UAtom || vuti == UAtom || vclasstype == UC_ELEMENT)
     {
       //only to be nclasstype quark makes sense!!! check first, one might be element
       if(nclasstype == UC_QUARK || vclasstype == UC_QUARK)
	 return genCodeCastAtomAndQuark(fp, uvpass);

       if(nclasstype == UC_ELEMENT || vclasstype == UC_ELEMENT)
	 return genCodeCastAtomAndElement(fp, uvpass);

       {
	 std::ostringstream msg;
	 msg << "Casting 'incomplete' types: " << m_state.getUlamTypeNameByIndex(nuti).c_str();
	 msg << "(UTI" << nuti << ") to be " << m_state.getUlamTypeNameByIndex(vuti).c_str();
	 msg << "(UTI" << vuti << ")";
	 MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	 return;
       }
     }

   s32 tmpVarCastNum = m_state.getNextTmpVarNumber();

   m_state.indent(fp);
   fp->write("const ");
   fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64, etc.
   fp->write(" ");
   fp->write(m_state.getTmpVarAsString(nuti, tmpVarCastNum).c_str());
   fp->write(" = ");

   // write the cast method (e.g. _Unsigned32ToInt32, _Int32ToUnary32, etc..)
   fp->write(nut->castMethodForCodeGen(vuti).c_str());
   fp->write("(");

   if(isTerminal)
     {
       u32 data = uvpass.getImmediateData(m_state);
       char dstr[40];
       vut->getDataAsString(data, dstr, 'z');
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
	fp->write(nut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::THE_INSTANCE.");
	fp->write(m_state.getIsMangledFunctionName());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum).c_str());
	fp->write("))\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write("FAIL(BAD_CAST);\n");
	m_state.m_currentIndentLevel--;
      }

    //update the uvpass to have the casted type
    uvpass = UlamValue::makePtr(tmpVarNum, uvpass.getPtrStorage(), nuti, m_state.determinePackable(nuti), m_state, 0, uvpass.getPtrNameId()); //POS 0 rightjustified; pass along name id

    return;
  } //genCodeCastAtomAndElement

  void NodeCast::genCodeCastAtomAndQuark(File * fp, UlamValue & uvpass)
  {
    UTI nuti = getNodeType(); //quark tobe
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    UTI vuti = uvpass.getUlamValueTypeIdx();
    if(vuti == Ptr)
      vuti = uvpass.getPtrTargetType(); //replace

    m_node->genCodeToStoreInto(fp, uvpass); //No need to load lhs into tmp (T); symbol's in COS vector

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    s32 tmpVarPos = m_state.getNextTmpVarNumber();
    // "downcast" might not be true; compare to be sure the atom has a quark "Foo"
    // get signed pos
    if(vuti == UAtom)
      {
	m_state.indent(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos).c_str());;
	fp->write(" = ");
	//internal method, takes uc, u32 and const char*, returns s32 position
	fp->write(m_state.getHasMangledFunctionName(vuti));
	fp->write("(");
	fp->write("uc, ");
	Node::genLocalMemberNameOfMethod(fp); //assume atom is a local var (neither dm nor ep)
	if(stgcos->isSelf())
	  fp->write("GetType(), "); //no read for self
	else
	  fp->write("read().GetType(), ");

	fp->write("\"");
	fp->write(nut->getUlamTypeMangledName().c_str());
	fp->write("\");\n"); //keeping pos in tmp
      }
    else
      {
	UlamType* vut = m_state.getUlamTypeByIndex(vuti);
	if(vut->getUlamClass() == UC_ELEMENT)
	  {
	    m_state.indent(fp);
	    fp->write("const s32 ");
	    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos).c_str());;
	    fp->write(" = ");
	    //internal method, takes uc, u32 and const char*, returns s32 position

	    fp->write(vut->getUlamTypeMangledName().c_str());
	    fp->write("<EC>::");

	    fp->write(m_state.getHasMangledFunctionName(vuti));
	    fp->write("(\"");
	    fp->write(nut->getUlamTypeMangledName().c_str());
	    fp->write("\");\n"); //keeping pos in tmp
	  }
        else
	  {
	    //e.g. a quark here would be wrong
	    std::ostringstream msg;
	    msg << "Casting 'incomplete' types: " << m_state.getUlamTypeNameByIndex(nuti).c_str();
	    msg << "(UTI" << nuti << ") to be " << m_state.getUlamTypeNameByIndex(vuti).c_str();
	    msg << "(UTI" << vuti << ")";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    assert(0);//return;
	  }
      }

    m_state.indent(fp);
    fp->write("if(");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos).c_str());
    fp->write(" < 0)\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("FAIL(BAD_CAST);\n\n");
    m_state.m_currentIndentLevel--;

    //informed by genCodedAutoLocal() in NodeVarDecl used for conditional-as
    // uses stgcos since there's no m_varSymbol in this situation.
    s32 tmpVarStg = m_state.getNextTmpVarNumber();
    UTI stguti = stgcos->getUlamTypeIdx();
    UlamType * stgut = m_state.getUlamTypeByIndex(stguti);
    assert(stguti == UAtom || stgut->getUlamClass() == UC_ELEMENT);

    // can't let Node::genCodeReadIntoTmpVar do this for us (we need a ref!):
    assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);
    m_state.indent(fp);
    // no const here
    fp->write(stgut->getTmpStorageTypeAsString().c_str());
    fp->write("& ");
    fp->write(m_state.getTmpVarAsString(stguti, tmpVarStg, TMPBITVAL).c_str());
    fp->write(" = ");
    fp->write(stgcos->getMangledName().c_str());

    if(!stgcos->isSelf())
      fp->write(".getRef()");
    fp->write("; //ref needed\n");

    // now we have our pos in tmpVarPos, and our T in tmpVarStg
    // time to (like a) "shadow 'self'" with auto local variable:
    m_state.indent(fp);
    fp->write(nut->getUlamTypeImmediateAutoMangledName().c_str()); //for C++ local vars, ie non-data members

    fp->write("<EC> ");
    s32 tmpIQ = m_state.getNextTmpVarNumber(); //tmp since no variable name
    fp->write(m_state.getTmpVarAsString(nuti, tmpIQ).c_str());
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(stguti, tmpVarStg, TMPBITVAL).c_str());

    //for known quark:
    fp->write(", ");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos).c_str());

    fp->write(");\n"); //like, shadow lhs of as

    //update the uvpass to have the casted immediate quark
    uvpass = UlamValue::makePtr(tmpIQ, uvpass.getPtrStorage(), nuti, m_state.determinePackable(nuti), m_state, 0); //POS 0 rightjustified;

    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs
  } //genCodeCastAtomAndQuark

  bool NodeCast::needsACast()
  {
    UTI tobeType = getNodeType();
    UTI nodeType = m_node->getNodeType();

    ULAMTYPECOMPARERESULTS uticr = UlamType::compare(nodeType, tobeType, m_state);
    if(uticr == UTIC_DONTKNOW)
      {
	std::ostringstream msg;
	msg << "Casting 'incomplete' types: " << m_state.getUlamTypeNameByIndex(nodeType).c_str();
	msg << "(UTI" << nodeType << ") to be " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
	msg << "(UTI" << tobeType << ")";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

    if(uticr == UTIC_SAME)
      return false;  //short-circuit if same exact type

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(tobeType)->getUlamTypeEnum();
    ULAMTYPE nodetypEnum = m_state.getUlamTypeByIndex(nodeType)->getUlamTypeEnum();

    // consider user requested first, then size independent;
    // even constant may need casting (e.g. narrowing for saturation)
    // Bool constants require casts to generate "full" true UlamValue (>1-bit).
    return( isExplicitCast() || typEnum != nodetypEnum  || (m_state.getBitSize(tobeType) != m_state.getBitSize(nodeType)) || (nodetypEnum == Bool && m_node->isAConstant() && m_state.getBitSize(tobeType)>1));
  } //needsACast

} //end MFM
