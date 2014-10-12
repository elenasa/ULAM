#include <stdlib.h>
#include "NodeVarDecl.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"

namespace MFM {

  NodeVarDecl::NodeVarDecl(SymbolVariable * sym, CompilerState & state) : Node(state), m_varSymbol(sym) {}

  NodeVarDecl::~NodeVarDecl(){}

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
  }


  const char * NodeVarDecl::getName()
  {
    return m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str();
  }


  const std::string NodeVarDecl::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeVarDecl::checkAndLabelType()
  {
    UTI it;
    if(!m_varSymbol)
      {
	MSG("","Variable symbol is missing",ERR);
	it = Nav;
      }
    else
      {
	it = m_varSymbol->getUlamTypeIdx();  //base type has arraysize
	//check for incomplete Classes
	if(m_state.getUlamTypeByIndex(it)->getUlamClass() == UC_INCOMPLETE)
	  {
	    if(! m_state.completeIncompleteClassSymbol(it))
	      {
		std::ostringstream msg;
		msg << "Incomplete Var Decl for type: <" << m_state.getUlamTypeNameByIndex(it).c_str() << "> used with variable symbol name <" << getName() << ">";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
      }
    setNodeType(it);
    return getNodeType();
  }


  EvalStatus NodeVarDecl::eval()
  {
    assert(m_varSymbol);
    //evalNodeProlog(0); //new current frame pointer
    //copy result UV to stack, -1 relative to current frame pointer
    //    assignReturnValueToStack(m_varSymbol->getUlamValue(m_state));
    //evalNodeEpilog();
    if(getNodeType() == Atom || m_state.getUlamTypeByIndex(getNodeType())->getUlamClass() == UC_ELEMENT)
      {
	UlamValue atomUV = UlamValue::makeAtom(m_varSymbol->getUlamTypeIdx());
	m_state.m_funcCallStack.storeUlamValueInSlot(atomUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else
      {
	if(!m_varSymbol->isDataMember())
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
  }
  

  EvalStatus  NodeVarDecl::evalToStoreInto()
  {
    assert(0);  //no way to get here!
    return ERROR;
  }


  bool NodeVarDecl::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return true;
  }


  void NodeVarDecl::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_varSymbol->setPosOffset(offset);
    offset += m_state.getTotalBitSize(m_varSymbol->getUlamTypeIdx());
  }


  // parse tree in order declared, unlike the ST.
  void NodeVarDecl::genCode(File * fp, UlamValue& uvpass)
  {
    if(m_varSymbol->isDataMember())
      return genCodedBitFieldTypedef(fp, uvpass);

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    m_state.indent(fp);
    if(!m_varSymbol->isDataMember())
      fp->write(vut->getUlamTypeImmediateMangledName(&m_state).c_str()); //for C++ local vars, ie non-data members
    else
      {
	fp->write(vut->getUlamTypeMangledName(&m_state).c_str()); //for C++
	assert(0); //doesn't happen anymore..
      }

    fp->write(" ");
    fp->write(m_varSymbol->getMangledName().c_str());

    //initialize T to default atom (might need "OurAtom" if data member ?)
    if(vclasstype == UC_QUARK || vclasstype == UC_ELEMENT)
      {
	fp->write(" = ");
	UTI selfuti = m_state.m_currentSelfSymbolForCodeGen->getUlamTypeIdx();
	fp->write(m_state.getUlamTypeByIndex(selfuti)->getUlamTypeMangledName(&m_state).c_str());
	fp->write("<CC>");
	fp->write("::");
	fp->write("GetDefaultAtom()");
      }

    fp->write(";\n");  //func call parameters aren't NodeVarDecl's
  }


  // variable is a data member
  void NodeVarDecl::genCodedBitFieldTypedef(File * fp, UlamValue& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * vut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();
    ULAMCLASSTYPE classtype = m_state.getUlamClassForThisClass();

    m_state.indent(fp);

    // use typedef rather than atomic parameter for quarks within elements
    if(vclasstype == UC_QUARK)
      {
	fp->write("typedef ");
	fp->write(vut->getUlamTypeMangledName(&m_state).c_str()); //for C++
	fp->write("<CC, ");  
	fp->write_decimal(m_varSymbol->getPosOffset() + ATOMFIRSTSTATEBITPOS);
      }
    else
      {
	fp->write("typedef AtomicParameterType<");
	fp->write("CC");  //BITSPERATOM
	fp->write(", ");
	fp->write(vut->getUlamTypeVDAsStringForC().c_str());
	fp->write(", ");
	fp->write_decimal(vut->getTotalBitSize());   //include arraysize
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


} //end MFM


