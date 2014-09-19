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
	u32 arraysize = m_state.getArraySize(sym->getUlamTypeIdx());
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
	if(m_parameterSymbols.at(i)->getUlamTypeIdx() != argTypes[i])
	  {
	    rtnBool = false;
	    break;
	  }
      }

    return rtnBool;
  }


  void SymbolFunction::generateFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    if(classtype == UC_ELEMENT)
      {
	generateElementFunctionDeclaration(fp, declOnly, classtype);
      }
    else
      {
	generateQuarkFunctionDeclaration(fp, declOnly, classtype);
      }
  }
  
  
  void SymbolFunction::generateElementFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx());
    m_state.indent(fp);
    fp->write(sut->getUlamTypeMangledName(&m_state).c_str()); //return type for C++
    //fp->write(getUlamType()->getBitSizeTemplateString().c_str());  //for quark templates

#if 0
    // might want to permit quarks as return value; if not, then need to prohibit them as local variables.
    if(sut->getUlamClass() == UC_QUARK)
      {
	std::ostringstream msg;
	msg << "Return Type <"  << sut->getUlamTypeName(&m_state).c_str() << ">  for function '" << m_state.m_pool.getDataAsString(getId()).c_str() << "', is a 'quark' and not a valid return type" ;
	MSG("", msg.str().c_str(), ERR);
	fp->write("<POS>");
      }
#endif
    fp->write(" ");
    if(!declOnly)
      {
	UTI cuti = m_state.m_classBlock->getNodeType();
	//include the mangled class::
	fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName(&m_state).c_str());
	fp->write("::");
      }
    fp->write(getMangledName().c_str());
    fp->write("(");
    u32 numparams = getNumberOfParameters();
    
    for(u32 i = 0; i < numparams; i++)
      {
	if(i > 0)
	  fp->write(", ");
	
	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	UTI auti = asym->getUlamTypeIdx();
	UlamType * aut = m_state.getUlamTypeByIndex(auti);

	fp->write(aut->getUlamTypeMangledName(&m_state).c_str()); //for C++

#if 0
	// might want to permit quark's as args
	if(aut->getUlamClass() == UC_QUARK)
	  {
	    std::ostringstream msg;
	    msg << "Parameter " << i + 1 << ", <"  << aut->getUlamTypeName(&m_state).c_str() << ">, for function '" << m_state.m_pool.getDataAsString(getId()).c_str() << "', is a 'quark' and not a valid parameter" ;
	    MSG("", msg.str().c_str(),ERR);
	    fp->write("<POS>");
	  }
#endif

	fp->write(" ");
	fp->write(asym->getMangledName().c_str());
      }
    
    fp->write(")");

    if(declOnly)
      {
	fp->write(";\n\n");
      }
    else
      {
	NodeBlockFunctionDefinition * func = getFunctionNode();
	assert(func); //how would a function symbol be without a body?
	func->genCode(fp);
      }
  }
  

  void SymbolFunction::generateQuarkFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    if(declOnly)
      return;
    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx());
    //template<POS> declared outside the quark struct instead.
    m_state.indent(fp);
    fp->write(sut->getUlamTypeMangledName(&m_state).c_str()); //return type for C++
    fp->write(" ");
    
    fp->write(getMangledName().c_str());
    fp->write("(");

    //first one is always Atom a&
    //fp->write("Atom& a");   //mangled ???
    UlamType * atomut = m_state.getUlamTypeByIndex(Atom);
    fp->write(atomut->getUlamTypeMangledName(&m_state).c_str()); //type for C++
    fp->write("& ");          //only place we use a reference
    fp->write("Uv_11");       //hardcoded variable XXX
    fp->write(atomut->getUlamTypeAsSingleLowercaseLetter());  //a name

    u32 numparams = getNumberOfParameters();
    
    for(u32 i = 0; i < numparams; i++)
      {
	//if(i > 0)
	fp->write(", ");
	
	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	UlamType * aut = m_state.getUlamTypeByIndex(asym->getUlamTypeIdx());
	fp->write(aut->getUlamTypeMangledName(&m_state).c_str()); //type for C++
	if(aut->getUlamClass() == UC_QUARK) 
	  {
	    fp->write("<POS>");
	  }
	fp->write(" ");
	fp->write(asym->getMangledName().c_str());
      }
    
    fp->write(")");
    
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func);       // how would a function symbol be without a body?
    func->genCode(fp);  // it better know its a quark!!!
  }

} //end MFM
