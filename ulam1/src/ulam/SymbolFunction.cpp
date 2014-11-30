#include "SymbolFunction.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolFunction::SymbolFunction(u32 id, UTI typetoreturn, CompilerState& state ) : Symbol(id,typetoreturn,state), m_functionNode(NULL) 
  { 
    setDataMember(); // by definition all function definitions are data members
  }

  SymbolFunction::~SymbolFunction()
  {
    delete m_functionNode; 
    // symbols belong to  NodeBlockFunctionDefinition's ST; deleted there.
    m_parameterSymbols.clear();
  }


  bool SymbolFunction::isFunction()
  {
    return true;
  }


  void SymbolFunction::addParameterSymbol(Symbol * sym)
  {
    m_parameterSymbols.push_back(sym);
  }


  u32 SymbolFunction::getNumberOfParameters()
  {
    return m_parameterSymbols.size();
  }


  u32 SymbolFunction::getTotalSizeOfParameters()
  {
    u32 totalsizes = 0;
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	totalsizes += m_state.slotsNeeded(sym->getUlamTypeIdx());
      }
    return totalsizes; 
  }


  Symbol * SymbolFunction::getParameterSymbolPtr(u32 n)
  {
    assert(n < m_parameterSymbols.size());
    return m_parameterSymbols[n];
  }


  void SymbolFunction::setFunctionNode(NodeBlockFunctionDefinition * func)
  {
    if(m_functionNode)
      delete m_functionNode;  //clean up any previous declarations

    m_functionNode = func;
  }


  NodeBlockFunctionDefinition *  SymbolFunction::getFunctionNode()
  {
    return m_functionNode;
  }


  const std::string SymbolFunction::getMangledPrefix()
  {
    return "Uf_";
  }


  //supports overloading functions with SymbolFunctionName
  const std::string SymbolFunction::getMangledNameWithTypes()
  {
    std::ostringstream mangled;
    mangled << Symbol::getMangledName();  //e.g. Uf_14name, with lexNumbers

    // use void type when no parameters
    if(m_parameterSymbols.empty())
      {
	UlamType * vit = m_state.getUlamTypeByIndex(Void);
	mangled << vit->getUlamTypeMangledName(&m_state).c_str();
      }

    // append mangled type name, e.g. 1023213Int, for each parameter
    // note: though Classes (as args) may be 'incomplete' (i.e. bit size == 0),
    //        during this parse stage, the key remains consistent.
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	UlamType * sut = m_state.getUlamTypeByIndex(sym->getUlamTypeIdx());
	mangled << sut->getUlamTypeMangledName(&m_state).c_str();
      }

    return mangled.str();
  }


  bool SymbolFunction::matchingTypes(std::vector<UTI> argTypes)
  {
    u32 numArgs = argTypes.size();

    // check number of args first
    if(numArgs != m_parameterSymbols.size())
      return false;

    bool rtnBool = true;

    //next match types; order counts!
    for(u32 i=0; i < numArgs; i++)
      {
	UTI puti = m_parameterSymbols.at(i)->getUlamTypeIdx();
	if( puti != argTypes[i])
	  {
	    //constants can match any bit size
	    ULAMTYPE ptypEnum = m_state.getUlamTypeByIndex(puti)->getUlamTypeEnum();
	    if(argTypes[i] != m_state.getUlamTypeOfConstant(ptypEnum))
	      {
		rtnBool = false;
		break;
	      }
	  }
      }

    return rtnBool;
  }


  u32 SymbolFunction::isNativeFunctionDeclaration()
  {
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func); 

    return (func->isNative() ? 1 : 0);
  }


  void SymbolFunction::generateFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func); //how would a function symbol be without a body?

    //up to programmer to define this function!!!
    if(!declOnly && func->isNative())
      return;

    m_state.outputTextAsComment(fp, func->getNodeLocation());

    //if(classtype == UC_ELEMENT)
    //	generateElementFunctionDeclaration(fp, declOnly, classtype);
    //else
    //	generateQuarkFunctionDeclaration(fp, declOnly, classtype);
    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx()); //return type
    
    m_state.indent(fp);
    if(declOnly)
      fp->write("static ");
    else
      {
	if(classtype == UC_QUARK)
	  fp->write("template<class CC, u32 POS>\n");
	else
	  fp->write("template<class CC>\n");
	m_state.indent(fp);
      }
    
    //fp->write(sut->getTmpStorageTypeAsString(&m_state).c_str()); //return type for C++
    fp->write(sut->getImmediateStorageTypeAsString(&m_state).c_str()); //return type for C++
    //fp->write(sut->getUlamTypeImmediateMangledName(&m_state).c_str()); //return type for C++
    //fp->write(getUlamType()->getBitSizeTemplateString().c_str());  //for quark templates
    
    fp->write(" ");
    if(!declOnly)
      {
	UTI cuti = m_state.m_classBlock->getNodeType();
	//include the mangled class::
	fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName(&m_state).c_str());
	
	if(classtype == UC_QUARK)
	  fp->write("<CC, POS>");
	else
	  fp->write("<CC>");
	
	fp->write("::");
      }
    
    fp->write(getMangledName().c_str());
    fp->write("(");
    
    //first one is always Atom a&
    //UlamType * atomut = m_state.getUlamTypeByIndex(Atom);
    //fp->write(atomut->getUlamTypeMangledName(&m_state).c_str()); //type for C++
    fp->write("T& ");          //only place we use a reference
    fp->write(m_state.getHiddenArgName());
    
    u32 numparams = getNumberOfParameters();
    
    for(u32 i = 0; i < numparams; i++)
      {
	//if(i > 0)
	fp->write(", ");
	
	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	UTI auti = asym->getUlamTypeIdx();
	UlamType * aut = m_state.getUlamTypeByIndex(auti);
	
	fp->write(aut->getImmediateStorageTypeAsString(&m_state).c_str()); //for C++
	//fp->write(aut->getUlamTypeImmediateMangledName(&m_state).c_str()); //for C++
	//fp->write(aut->getBitSizeTemplateString().c_str());  //for quark templates
	
	if(aut->getUlamClass() == UC_QUARK) 
	  {
	    fp->write("<CC,POS>");
	  }
	
	fp->write(" ");
	fp->write(asym->getMangledName().c_str());
      }
    
    fp->write(")");
    
    if(declOnly)
      {
	if(func->isNative())
	  fp->write("; //native\n\n");
	else
	  fp->write(";\n\n");
      }
    else
      {
	UlamValue uvpass;
	func->genCode(fp, uvpass);
      }
  } //generateFunctionDeclaration

} //end MFM
