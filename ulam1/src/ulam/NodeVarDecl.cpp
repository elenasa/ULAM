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

    u32 arraysize = m_state.getArraySize(m_varSymbol->getUlamTypeIdx());
    if(arraysize > 0)
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
    offset += m_state.getUlamTypeByIndex(m_varSymbol->getUlamTypeIdx())->getTotalBitSize();
  }


  void NodeVarDecl::genCode(File * fp)
  {
    //like ST version: genCodeForTableOfVariableDataMembers, except in order declared
    UTI vuti = m_varSymbol->getUlamTypeIdx(); 
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();
    m_state.indent(fp);
    fp->write(vut->getUlamTypeMangledName(&m_state).c_str()); //for C++
    
    if(vclasstype == UC_QUARK)
      {
	if(m_varSymbol->isDataMember())
	  {
	    // position determined by previous data member bit lengths
	    fp->write("<");
	    fp->write_decimal(m_varSymbol->getPosOffset());
	    fp->write(">");
	  }
	else
	  {
	    fp->write(vut->getBitSizeTemplateString().c_str());  //for quark templates
	  }
      }
    
    fp->write(" ");
    fp->write(m_varSymbol->getMangledName().c_str());
    
#if 0
    u32 arraysize = vut->getArraySize();
    if(arraysize > 0)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
#endif
    
    fp->write(";\n");  //func call parameters aren't NodeVarDecl's
  }

} //end MFM


