#include "SymbolFunction.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolFunction::SymbolFunction(u32 id, UlamType * typetoreturn) : Symbol(id,typetoreturn), m_functionNode(NULL) 
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
	u32 arraysize = sym->getUlamType()->getArraySize();
	totalsizes += (arraysize > 0 ? arraysize : 1);
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
  const std::string SymbolFunction::getMangledNameWithTypes(CompilerState * state)
  {
    std::ostringstream mangled;
    mangled << Symbol::getMangledName(state);  //e.g. Uf_14name, with lexNumbers

    // use void type when no parameters
    if(m_parameterSymbols.empty())
      {
	UlamType * vit = state->getUlamTypeByIndex(Void);
	mangled << vit->getUlamTypeMangledName(state).c_str();
      }

    // append mangled type name, e.g. 1023213Int, for each parameter
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	mangled << sym->getUlamType()->getUlamTypeMangledName(state).c_str();
      }

    return mangled.str();
  }


  bool SymbolFunction::matchingTypes(std::vector<UlamType *> argTypes)
  {
    u32 numArgs = argTypes.size();

    // check number of args first
    if(numArgs != m_parameterSymbols.size())
      return false;

    bool rtnBool = true;

    //next match types; order counts!
    for(u32 i=0; i < numArgs; i++)
      {
	if(m_parameterSymbols.at(i)->getUlamType() != argTypes[i])
	  {
	    rtnBool = false;
	    break;
	  }
      }

    return rtnBool;
  }


  void SymbolFunction::generateFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype, CompilerState * state)
  {
    if(classtype == UC_ELEMENT)
      {
	generateElementFunctionDeclaration(fp, declOnly, classtype, state);
      }
    else
      {
	generateQuarkFunctionDeclaration(fp, declOnly, classtype, state);
      }
  }
  
  
  void SymbolFunction::generateElementFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype, CompilerState * state)
  {
    state->indent(fp);
    fp->write(getUlamType()->getUlamTypeMangledName(state).c_str()); //return type for C++
    fp->write(getUlamType()->getBitSizeTemplateString().c_str());    //for quark templates
    fp->write(" ");
    if(!declOnly)
      {
	UlamTypeClass * cut = (UlamTypeClass *) state->m_classBlock->getNodeType();
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName(state).c_str());
	fp->write("::");
      }
    fp->write(getMangledName(state).c_str());
    fp->write("(");
    u32 numparams = getNumberOfParameters();
    
    for(u32 i = 0; i < numparams; i++)
      {
	if(i > 0)
	  fp->write(", ");
	
	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	fp->write(asym->getUlamType()->getUlamTypeMangledName(state).c_str()); //for C++
	fp->write(getUlamType()->getBitSizeTemplateString().c_str());  //for quark templates
	fp->write(" ");
	fp->write(asym->getMangledName(state).c_str());
      }
    
    fp->write(")");

    if(declOnly)
      {
	//state->indent(fp);
	fp->write(";\n\n");
      }
    else
      {
	NodeBlockFunctionDefinition * func = getFunctionNode();
	assert(func); //how would a function symbol be without a body?
	func->genCode(fp);
      }
  }
  

  void SymbolFunction::generateQuarkFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype, CompilerState * state)
  {
    if(declOnly)
      return;

    //template declared outside the quark struct instead.
    state->indent(fp);
    fp->write(getUlamType()->getUlamTypeMangledName(state).c_str()); //return type for C++
    fp->write(" ");
    
    fp->write(getMangledName(state).c_str());
    fp->write("(");

    //first one is always Atom a&
    //fp->write("Atom& a");   //mangled ???
    UlamType * atomut = state->getUlamTypeByIndex(Atom);
    fp->write(atomut->getUlamTypeMangledName(state).c_str()); //type for C++
    fp->write("& ");          //only place we use a reference
    //fp->write("Uv_11");     //hardcoded variable XXX
    fp->write(atomut->getUlamTypeAsSingleLowercaseLetter());  //a name

    u32 numparams = getNumberOfParameters();
    
    for(u32 i = 0; i < numparams; i++)
      {
	//if(i > 0)
	fp->write(", ");
	
	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	UlamType * aut = asym->getUlamType();
	fp->write(aut->getUlamTypeMangledName(state).c_str()); //type for C++
	if(aut->getUlamClassType() == UC_QUARK) 
	  {
	    fp->write("<POS>");
	  }
	fp->write(" ");
	fp->write(asym->getMangledName(state).c_str());
      }
    
    fp->write(")");
    
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func); //how would a function symbol be without a body?
    func->genCode(fp);  //it better know its a quark!!!
  }

} //end MFM
