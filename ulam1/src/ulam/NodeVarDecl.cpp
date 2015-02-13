#include <stdlib.h>
#include "NodeVarDecl.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"

namespace MFM {

  NodeVarDecl::NodeVarDecl(SymbolVariable * sym, CompilerState & state) : Node(state), m_varSymbol(sym), m_vid(0), m_currBlock(NULL), m_currBlockNo(0)
  {
    if(sym)
      {
	m_vid = sym->getId();
	m_currBlockNo = sym->getBlockNoOfST();
      }
  }

  NodeVarDecl::NodeVarDecl(const NodeVarDecl& ref) : Node(ref), m_varSymbol(NULL), m_vid(ref.m_vid), m_currBlock(NULL), m_currBlockNo(ref.m_currBlockNo) {}

  NodeVarDecl::~NodeVarDecl() {}

  Node * NodeVarDecl::instantiate()
  {
    return new NodeVarDecl(*this);
  }

  void NodeVarDecl::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(m_varSymbol->getUlamTypeIdx()).c_str()); //short type name
    fp->write(" ");
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(m_varSymbol->getUlamTypeIdx());
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    fp->write("; ");
  } //printPostfix

  const char * NodeVarDecl::getName()
  {
    return m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str();
  }

  const std::string NodeVarDecl::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeVarDecl::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return true;
  }

  UTI NodeVarDecl::checkAndLabelType()
  {
    UTI it = Nav;
    // instantiate, look up in current block
    if(m_varSymbol == NULL)
      {
	NodeBlock * savecurrentblock = m_state.m_currentBlock; //**********
	//in case of a cloned unknown
	if(m_currBlock == NULL)
	  setBlock();

	NodeBlockClass * savememberclassblock = m_state.m_currentMemberClassBlock;
	bool saveUseMemberBlock = m_state.m_useMemberBlock;
	m_state.m_useMemberBlock = false;

	m_state.m_currentBlock = m_currBlock; //before lookup

	Symbol * asymptr = NULL;
	if(m_state.alreadyDefinedSymbol(m_vid, asymptr))
	  {
	    if(!asymptr->isTypedef() && !asymptr->isConstant() && !asymptr->isFunction())
	      {
		m_varSymbol = (SymbolVariable *) asymptr;
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.m_pool.getDataAsString(m_vid).c_str() << "> is not a variable, and cannot be used as one";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) Variable <" << m_state.m_pool.getDataAsString(m_vid).c_str() << "> is not defined, and cannot be used";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      	m_state.m_currentBlock = savecurrentblock; //restore
	m_state.m_useMemberBlock = saveUseMemberBlock;
	m_state.m_currentMemberClassBlock = savememberclassblock;
      } //to instantiate

    if(m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx();  //base type has arraysize
	//check for incomplete Classes
	UlamType * tdut = m_state.getUlamTypeByIndex(it);
	ULAMCLASSTYPE tdclasstype = tdut->getUlamClass();
	if(tdclasstype == UC_UNSEEN)
	  {
	    if(!m_state.completeIncompleteClassSymbol(it))
	      {
		std::ostringstream msg;
		msg << "Incomplete Var Decl for class type: " << m_state.getUlamTypeNameByIndex(it).c_str() << " used with variable symbol name <" << getName() << "> (UTI" << it << ") while labeling class: " << m_state.getUlamTypeNameByIndex(m_state.m_compileThisIdx).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		it = Nav;
	      }
	  }
	else if(tdclasstype != UC_NOTACLASS)
	  {
	    if(!m_state.constantFoldPendingArgs(it))
	      {
		std::ostringstream msg;
		msg << "Incomplete Variable Decl for type: " << m_state.getUlamTypeNameByIndex(it).c_str() << " used with variable symbol name <" << getName() << "> UTI(" << it << ") while labeling class: " << m_state.getUlamTypeNameByIndex(m_state.m_compileThisIdx).c_str() << ", class arguments pending";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		it = Nav;
	      }
	  }
	else if(!tdut->isComplete())
	  {
	    m_state.constantFoldIncompleteUTI(it); //update if possible
	    tdut = m_state.getUlamTypeByIndex(it); //reload
	    if(!tdut->isComplete())
	      {
		std::ostringstream msg;
		msg << "Incomplete Variable Decl for type: " << m_state.getUlamTypeNameByIndex(it).c_str() << " used with variable symbol name <" << getName() << "> UTI(" << it << ") while labeling class: " << m_state.getUlamTypeNameByIndex(m_state.m_compileThisIdx).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		it = Nav;
	      }
	  }
      }
    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  NNO NodeVarDecl::getBlockNo()
  {
    return m_currBlockNo;
  }

  void NodeVarDecl::setBlock()
  {
    assert(m_currBlockNo);
    m_currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(m_currBlock);
  }

  void NodeVarDecl::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert((s32) offset >= 0); //neg is invalid
    assert(m_varSymbol);

    //skip element parameter variables
    if(!m_varSymbol->isElementParameter())
      {
	m_varSymbol->setPosOffset(offset);
	UTI it = m_varSymbol->getUlamTypeIdx();

	if(!m_state.isComplete(it))
	  {
	    m_state.constantFoldIncompleteUTI(it); //update if possible
	  }

	if(m_state.getTotalBitSize(it) > MAXBITSPERINT)
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of type: " << m_state.getUlamTypeNameByIndex(it).c_str() << " (UTI" << it << ") total size: " << (s32) m_state.getTotalBitSize(it) << " MUST fit into " << MAXBITSPERINT << " bits; Local variables do not have this restriction";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	offset += m_state.getTotalBitSize(m_varSymbol->getUlamTypeIdx());
      }
  } //packBitsInOrderOfDeclaration

  EvalStatus NodeVarDecl::eval()
  {
    assert(m_varSymbol);
    //evalNodeProlog(0); //new current frame pointer
    //copy result UV to stack, -1 relative to current frame pointer
    //    assignReturnValueToStack(m_varSymbol->getUlamValue(m_state));
    //evalNodeEpilog();
    if(getNodeType() == UAtom || m_state.getUlamTypeByIndex(getNodeType())->getUlamClass() == UC_ELEMENT)
      {
	UlamValue atomUV = UlamValue::makeAtom(m_varSymbol->getUlamTypeIdx());
	m_state.m_funcCallStack.storeUlamValueInSlot(atomUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else
      {
	if(!m_varSymbol->isDataMember() || m_varSymbol->isElementParameter())
	  {
	    //local variable to a function
	    UlamValue immUV = UlamValue::makeImmediate(m_varSymbol->getUlamTypeIdx(), 0, m_state);
	    m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableDataMember *) m_varSymbol)->getStackFrameSlotIndex());
	  }
	else
	  {
	    assert(0);
	  }
      }
    return NORMAL;
  } //eval

  EvalStatus  NodeVarDecl::evalToStoreInto()
  {
    assert(0);  //no way to get here!
    return ERROR;
  }

  // parse tree in order declared, unlike the ST.
  void NodeVarDecl::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_varSymbol);

    // use immediate storage for "static" element parameters
    if(m_varSymbol->isElementParameter())
      {
	return genCodedElementParameter(fp, uvpass);
      }

    if(m_varSymbol->isDataMember())
      {
	return genCodedBitFieldTypedef(fp, uvpass);
      }

    if(m_varSymbol->isAutoLocal())
      {
	return genCodedAutoLocal(fp, uvpass);
      }

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indent(fp);
    if(!m_varSymbol->isDataMember() || m_varSymbol->isElementParameter())
      {
	fp->write(vut->getImmediateStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
      }
    else
      {
	fp->write(vut->getUlamTypeMangledName().c_str()); //for C++
	assert(0); //doesn't happen anymore..
      }

    fp->write(" ");
    fp->write(m_varSymbol->getMangledName().c_str());

    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    //initialize T to default atom (might need "OurAtom" if data member ?)
    if(vclasstype == UC_ELEMENT)
      {
	fp->write(" = ");
	fp->write(m_state.getUlamTypeByIndex(vuti)->getUlamTypeMangledName().c_str());
	fp->write("<CC>");
	fp->write("::THE_INSTANCE");
	fp->write(".GetDefaultAtom()");  //returns object of type T
      }

    if(vclasstype == UC_QUARK)
      {
	//right-justified?
      }

    fp->write(";\n");  //func call parameters aren't NodeVarDecl's
  } //genCode


  // variable is a data member; not an element
  void NodeVarDecl::genCodedBitFieldTypedef(File * fp, UlamValue& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE nclasstype = nut->getUlamClass();
    ULAMCLASSTYPE classtype = m_state.getUlamClassForThisClass();

    m_state.indent(fp);
    if(nclasstype == UC_QUARK && nut->isScalar())
      {
	// use typedef rather than atomic parameter for quarks within elements,
	// except if an array of quarks.
	fp->write("typedef ");
	fp->write(nut->getUlamTypeMangledName().c_str()); //for C++
	fp->write("<CC, ");
	if(classtype == UC_ELEMENT)
	  fp->write_decimal(m_varSymbol->getPosOffset() + ATOMFIRSTSTATEBITPOS);
	else
	  {
	    //inside a quark
	    fp->write("POS + ");
	    fp->write_decimal(m_varSymbol->getPosOffset());
	  }
      }
    else
      {
	fp->write("typedef AtomicParameterType<");
	fp->write("CC");  //BITSPERATOM
	fp->write(", ");
	fp->write(nut->getUlamTypeVDAsStringForC().c_str());
	fp->write(", ");
	fp->write_decimal(nut->getTotalBitSize());   //include arraysize
	fp->write(", ");
	if(classtype == UC_QUARK)
	  {
	    fp->write("POS + ");
	    fp->write_decimal(m_varSymbol->getPosOffset());
	  }
	else
	  {
	    assert(classtype == UC_ELEMENT);
	    fp->write_decimal(m_varSymbol->getPosOffset() + ATOMFIRSTSTATEBITPOS);
	  }
      }
    fp->write("> ");
    fp->write(m_varSymbol->getMangledNameForParameterType().c_str());
    fp->write(";\n");  //func call parameters aren't NodeVarDecl's
  } //genCodedBitFieldTypedef

  void NodeVarDecl::genCodedElementParameter(File * fp, UlamValue uvpass)
  {
    assert(m_varSymbol->isDataMember());

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indent(fp);
    fp->write("mutable ");

    fp->write(vut->getImmediateStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");
    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write(";\n");  //func call parameters aren't NodeVarDecl's
  }  //genCodedElementParameter

  // this is the auto local variable's node, created at parse time,
  // for Conditional-As case.
  void NodeVarDecl::genCodedAutoLocal(File * fp, UlamValue & uvpass)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    // the uvpass comes from NodeControl, and still has the POS obtained
    // during the condition statement for As..unorthodox, but necessary.
    assert(uvpass.getUlamValueTypeIdx() == Ptr);
    s32 tmpVarPos = uvpass.getPtrSlotIndex();

    // before shadowing the lhs of the as-conditional variable with its auto,
    // let's load its storage from the currentSelfSymbol:
    s32 tmpVarStg = m_state.getNextTmpVarNumber();
    UTI stguti = m_state.m_currentObjSymbolsForCodeGen[0]->getUlamTypeIdx();
    UlamType * stgut = m_state.getUlamTypeByIndex(stguti);
    assert(stguti == UAtom || stgut->getUlamClass() == UC_ELEMENT);

    // can't let Node::genCodeReadIntoTmpVar do this for us:
    assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);
    m_state.indent(fp);
    fp->write(stgut->getTmpStorageTypeAsString().c_str());
    fp->write("& ");
    fp->write(m_state.getTmpVarAsString(stguti, tmpVarStg, TMPBITVAL).c_str());
    fp->write(" = ");
    fp->write(m_state.m_currentObjSymbolsForCodeGen[0]->getMangledName().c_str());
    fp->write(".getRef();\n");

    // now we have our pos in tmpVarPos, and our T in tmpVarStg
    // time to shadow 'self' with auto local variable:
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    m_state.indent(fp);
    fp->write(vut->getUlamTypeImmediateAutoMangledName().c_str()); //for C++ local vars, ie non-data members

    fp->write("<CC> ");
    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(stguti, tmpVarStg, TMPBITVAL).c_str());

    if(vclasstype == UC_QUARK)
      {
	fp->write(", ");
	fp->write(m_state.getTmpVarAsString(uvpass.getPtrTargetType(), tmpVarPos).c_str());
      }
    else if(vclasstype == UC_ELEMENT)
      {
	fp->write(", true");  //invokes 'badass' constructor
      }
    else
      assert(0);

    fp->write(");   //shadows lhs of 'as'\n");

    m_state.m_genCodingConditionalAs = false;       // done
    m_state.m_currentObjSymbolsForCodeGen.clear();  //clear remnant of lhs ?
  } //genCodedAutoLocal

} //end MFM
