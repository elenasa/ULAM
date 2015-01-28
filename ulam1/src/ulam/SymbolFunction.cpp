#include "SymbolFunction.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolFunction::SymbolFunction(u32 id, UTI typetoreturn, CompilerState& state ) : Symbol(id,typetoreturn,state), m_functionNode(NULL), m_hasVariableArgs(false)
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


  void SymbolFunction::markForVariableArgs(bool m)
  {
    m_hasVariableArgs = m;
  }


  bool SymbolFunction::takesVariableArgs()
  {
    return m_hasVariableArgs;
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


  //supports overloading functions with SymbolFunctionName;
  // join function name with comma-delimited UTI parameters
  const std::string SymbolFunction::getMangledNameWithUTIparameters()
  {
    std::ostringstream mangled;
    mangled << Symbol::getMangledName();  //e.g. Uf_14name, with lexNumbers

    // use void type when no parameters
    if(m_parameterSymbols.empty())
      {
	UlamType * vit = m_state.getUlamTypeByIndex(Void);
	UTI avuti;
	assert(m_state.aDefinedUTI(vit->getUlamKeyTypeSignature(), avuti));
	mangled << "," << avuti;
      }

    // append UTI for each parameter
    // note: though Classes (as args) may be 'incomplete' (i.e. bit size == 0),
    //        during this parse stage, the key remains consistent.
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	mangled << "," << sym->getUlamTypeIdx();
      }
    return mangled.str();
  } //getMangledNameWithUTIparameters


  //supports overloading functions with SymbolFunctionName
  const std::string SymbolFunction::getMangledNameWithTypes()
  {
    std::ostringstream mangled;
    mangled << Symbol::getMangledName();  //e.g. Uf_14name, with lexNumbers

    // use void type when no parameters
    if(m_parameterSymbols.empty())
      {
	UlamType * vit = m_state.getUlamTypeByIndex(Void);
	mangled << vit->getUlamTypeMangledName().c_str();
      }

    // append mangled type name, e.g. 1023213Int, for each parameter
    // note: though Classes (as args) may be 'incomplete' (i.e. bit size == 0),
    //        during this parse stage, the key remains consistent.
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	UlamType * sut = m_state.getUlamTypeByIndex(sym->getUlamTypeIdx());
	mangled << sut->getUlamTypeMangledName().c_str();
      }
    return mangled.str();
  } //getMangledNameWithTypes


  bool SymbolFunction::matchingTypes(std::vector<UTI> argTypes)
  {
    u32 numArgs = argTypes.size();
    u32 numParams = m_parameterSymbols.size();

    // numArgs could be greater if this function takes variable args
    // check number of args first
    //    if(numArgs != m_parameterSymbols.size())
    if(numArgs < numParams || (numArgs > numParams && !takesVariableArgs()))
      return false;

    bool rtnBool = true;

    //next match types; order counts!
    for(u32 i=0; i < numParams; i++)
      {
	UTI puti = m_parameterSymbols.at(i)->getUlamTypeIdx();
	//if( puti != argTypes[i])
	if(UlamType::compare(puti, argTypes[i], m_state) == UTIC_NOTSAME)
	  {
	    //constants can match any bit size
	    ULAMTYPE ptypEnum = m_state.getUlamTypeByIndex(puti)->getUlamTypeEnum();
	    //if(argTypes[i] != m_state.getUlamTypeOfConstant(ptypEnum))
	    if(UlamType::compare(argTypes[i], m_state.getUlamTypeOfConstant(ptypEnum), m_state) == UTIC_NOTSAME)
	      {
		rtnBool = false;
		break;
	      }
	  }
      }
    return rtnBool;
  } //matchingTypes


  u32 SymbolFunction::isNativeFunctionDeclaration()
  {
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func);

    return (func->isNative() ? 1 : 0);
  } //isNativeFunctionDeclaration


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
      {
	if(classtype == UC_QUARK)
	  fp->write("static ");   //element functions are not static
      }
    else
      {
	if(classtype == UC_QUARK)
	  fp->write("template<class CC, u32 POS>\n");
	else
	  fp->write("template<class CC>\n");
	m_state.indent(fp);
      }

    fp->write(sut->getImmediateStorageTypeAsString().c_str()); //return type for C++
    fp->write(" ");
    if(!declOnly)
      {
	UTI cuti = m_state.m_classBlock->getNodeType();
	//include the mangled class::
	fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());

	if(classtype == UC_QUARK)
	  fp->write("<CC, POS>");
	else
	  fp->write("<CC>");

	fp->write("::");
      }

    fp->write(getMangledName().c_str());
    fp->write("(");

    fp->write("UlamContext<CC>& uc, ");  //first arg is unmangled context

    //the hidden arg is "self", a T& (atom)
    fp->write("T& ");          //a reference
    fp->write(m_state.getHiddenArgName());

    u32 numparams = getNumberOfParameters();

    for(u32 i = 0; i < numparams; i++)
      {
	fp->write(", ");

	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	UTI auti = asym->getUlamTypeIdx();
	UlamType * aut = m_state.getUlamTypeByIndex(auti);

	fp->write(aut->getImmediateStorageTypeAsString().c_str()); //for C++
	fp->write(" ");
	fp->write(asym->getMangledName().c_str());
      }

    if(takesVariableArgs())
      {
	assert(func->isNative());
	fp->write(", ...");  //ellipses must be after at least one param
      }

    fp->write(")");

    if(declOnly)
      {
	if(func->isNative())
	  fp->write("; //native\n\n");
	else
	  {
	    if(classtype == UC_ELEMENT)
	      fp->write(" const");   //element functions are const, not static

	    fp->write(";\n\n");
	  }
      }
    else
      {
	if(classtype == UC_ELEMENT)
	  fp->write(" const");      //element functions are const, not static

	UlamValue uvpass;
	func->genCode(fp, uvpass);
      }
  } //generateFunctionDeclaration

} //end MFM
