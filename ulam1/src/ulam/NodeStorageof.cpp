#include <stdio.h>
#include "NodeStorageof.h"
#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  NodeStorageof::NodeStorageof(Token tokof, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeAtomof(tokof, nodetype, state)
  { }

  NodeStorageof::NodeStorageof(const NodeStorageof& ref) : NodeAtomof(ref)
  { }

  NodeStorageof::~NodeStorageof()
  { }

  Node * NodeStorageof::instantiate()
  {
    return new NodeStorageof(*this);
  }

  const char * NodeStorageof::getName()
  {
    return ".storageof";
  }

  const std::string NodeStorageof::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeStorageof::checkAndLabelType()
  {
    assert(m_token.m_type == TOK_IDENTIFIER); //caught at parse time (right?)
    UTI nuti = NodeAtomof::checkAndLabelType();
    if(m_state.okUTItoContinue(nuti))
      {
	assert(m_varSymbol);
	UTI vuti = m_varSymbol->getUlamTypeIdx();
	bool isself = m_varSymbol->isSelf(); //a ref

	//refs checked at runtime; non-refs here:
	if(!m_state.isReference(vuti) && !isself)
	  {
	    UTI oftype = NodeAtomof::getOfType();
	    UlamType * ofut = m_state.getUlamTypeByIndex(oftype);
	    //only an element or atom have real storage (ie. not quarks)
	    if(ofut->getUlamClass() == UC_QUARK)
	      {
		//only way to get storage for a quark is if its a DM
		// of an element; (XXX can't parse a.b.c.storageof)
		if(!m_varSymbol->isDataMember())
		  {
		    std::ostringstream msg;
		    msg << "<" << m_state.getTokenDataAsString(&m_token).c_str();
		    msg << "> is a quark and cannot be used with ";
		    msg << getName() << "; try a reference or self";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		  }
		else
		  {
		    UTI cuti = m_state.getCompileThisIdx();
		    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
		    std::ostringstream msg;
		    msg << "<" << m_state.getTokenDataAsString(&m_token).c_str();
		    msg << "> is a data member of ";
		    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		    if(cut->getUlamClass() == UC_QUARK)
		      {
			msg << "; Quarks cannot be used with " << getName();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			setNodeType(Nav);
		      }
		    else
		      {
			msg << "; 'self' used with " << getName();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		      }
		  }
	      }
	  }
      }
    return getNodeType(); //UAtomRef (storeintoable)
  } //checkAndLabelType

  UlamValue NodeStorageof::makeUlamValuePtr()
  {
    // (from NodeVarDecl's makeUlamValuePtr)
    UlamValue ptr;
    UlamValue atomuv;

    UTI auti = getOfType();
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    ULAMCLASSTYPE aclasstype = aut->getUlamClass();

    assert(m_varSymbol);

    if(m_varSymbol->isSelf())
      {
	//when "self/atom" is a quark, we're inside a func called on a quark (e.g. dm or local)
	//'atom' gets entire atom/element containing this quark; including its type!
	//'self' gets type/pos/len of the quark from which 'atom' can be extracted
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI ttype = selfuvp.getPtrTargetType();
	assert(m_state.okUTItoContinue(ttype));
	if((m_state.getUlamTypeByIndex(ttype)->getUlamClass() == UC_QUARK))
	  {
	    selfuvp = atomuv; //bail for error
	  }
	return selfuvp;
      } //done

    if(m_varSymbol->getAutoLocalType() == ALT_AS)
      return ((SymbolVariableStack *) m_varSymbol)->getAutoPtrForEval(); //haha! we're done.

    if(aclasstype == UC_ELEMENT)
      {
	// ptr to explicit atom or element, (e.g.'f' in f.a=1) becomes new m_currentObjPtr
	ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, auti, UNPACKED, m_state, 0, m_varSymbol->getId());
      }
    else
      {
	if(m_varSymbol->isDataMember())
	  {
	    UTI cuti = m_state.m_currentObjPtr.getPtrTargetType();
	    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	    if(cut->getUlamClass() == UC_QUARK)
	      ptr = atomuv; //bail
	    else
	      // return ptr to the m_currentObjPtr that contains this data member within
	      ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), auti, m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	  }
	else
	  {
	    UTI vuti = m_varSymbol->getUlamTypeIdx();
	    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
	    if(vut->getUlamClass() == UC_QUARK)
	      ptr = atomuv; //bail
	    else
	      //local variable on the stack; could be array ptr!
	      ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, auti, m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	  }
      }
    return ptr;
  } //makeUlamValuePtr

  void NodeStorageof::genCode(File * fp, UlamValue& uvpass)
  {
    //First, read into UAtomref immediate
    genCodeToStoreInto(fp, uvpass);
    s32 tmpVarNum = uvpass.getPtrSlotIndex(); //uatomref

    // THEN READ:
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp to read into, T

    UTI nuti = getNodeType(); //UAtomRef
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    m_state.indent(fp); //non-const
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, nut->getTmpStorageTypeForTmpVar()).c_str());
    fp->write(" = ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
    fp->write(".");
    //fp->write(nut->readMethodForCodeGen().c_str());
    fp->write("GetStorage"); //non-const
    fp->write("();\n");

    uvpass = UlamValue::makePtr(tmpVarNum2, nut->getTmpStorageTypeForTmpVar(), nuti, UNPACKED, m_state, 0, m_varSymbol ? m_varSymbol->getId() : 0);
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of rhs ?
  } //genCode

  void NodeStorageof::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    //lhs
    assert(getStoreIntoAble() == TBOOL_TRUE);
    UTI nuti = getNodeType(); //UAtomRef
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref

    assert(m_varSymbol);
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    bool isself = m_varSymbol->isSelf();
    //if var is a data member quark, then also isself

    if(m_state.isReference(vuti) || isself)
      {
	m_state.indent(fp);
	fp->write("if(");
	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write(".GetType() == T::ATOM_UNDEFINED_TYPE)\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write("FAIL(ILLEGAL_ARGUMENT);\n");
	m_state.m_currentIndentLevel--;
      }

    m_state.indent(fp); //non-const
    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
    fp->write("(");

    //data member's storage is self (not a ref)
    if(m_varSymbol->isDataMember() || isself)
      fp->write("ur");
    else
      fp->write(m_varSymbol->getMangledName().c_str()); //element or atom

    fp->write(".GetStorage()"); //can't be const
    fp->write(", uc); //storageof \n");

    uvpass = UlamValue::makePtr(tmpVarNum, TMPBITVAL, nuti, UNPACKED, m_state, 0, m_varSymbol ? m_varSymbol->getId() : 0);

    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of rhs ?
  } //genCodeToStoreInto

} //end MFM
