#include <stdlib.h>
#include "NodeVarDecl.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableStatic.h"

namespace MFM {

  NodeVarDecl::NodeVarDecl(SymbolVariable * sym, CompilerState & state) : Node(state), m_varSymbol(sym) {}

  NodeVarDecl::~NodeVarDecl(){}

  void NodeVarDecl::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(m_varSymbol->getUlamType()->getUlamTypeNameBrief(&m_state).c_str()); //short type name
    fp->write(" ");
    fp->write(getName());

    u32 arraysize = m_varSymbol->getUlamType()->getUlamKeyTypeSignature().getUlamKeyTypeSignatureArraySize();
    if(arraysize > 0)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }

    if(m_varSymbol->isDataMember())
      {
	char * myval = new char[arraysize * 8 + 32];	
	m_varSymbol->getUlamValue(m_state).getUlamValueAsString(myval, &m_state);
	fp->write("(");
	fp->write(myval);
	fp->write(")");
	delete [] myval;
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


  UlamType * NodeVarDecl::checkAndLabelType()
  {
    UlamType * it;
    if(!m_varSymbol)
      {
	MSG("","Variable symbol is missing",ERR);
	it = m_state.getUlamTypeByIndex(Nav);
      }
    else
      {
	it = m_varSymbol->getUlamType();  //base type has arraysize
	//check for incomplete Classes
	if(it->getUlamClassType() == UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Var Decl for type: <" << m_state.getUlamTypeNameByIndex(m_state.getUlamTypeIndex(it)).c_str() << "> used with variable symbol name <" << getName() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.completeIncompleteClassSymbol(it);
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
    return NORMAL;
  }
  

  EvalStatus  NodeVarDecl::evalToStoreInto()
  {
    assert(m_varSymbol);

    evalNodeProlog(0); //new current frame pointer

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(m_varSymbol->getUlamValueToStoreInto());

    evalNodeEpilog();
    return NORMAL;
  }


  bool NodeVarDecl::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return true;
  }


  void NodeVarDecl::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_varSymbol->setPosOffset(offset);
    offset += m_varSymbol->getUlamType()->getTotalBitSize();
  }


  void NodeVarDecl::genCode(File * fp)
  {
    UlamType * vut = m_varSymbol->getUlamType(); 
    m_state.indent(fp);
    fp->write(vut->getUlamTypeMangledName(&m_state).c_str()); //for C++
    
    if(m_varSymbol->isDataMember())
      {
	fp->write("<POS>");  //needs to adjust position by previous quark bit lengths XXX
      }
    else
      {
	fp->write(vut->getBitSizeTemplateString().c_str());  //for quark templates
      }
    
    fp->write(" ");
    fp->write(m_varSymbol->getMangledName(&m_state).c_str());

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


