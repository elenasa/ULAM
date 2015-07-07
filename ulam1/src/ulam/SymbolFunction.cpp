#include "SymbolFunction.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolFunction::SymbolFunction(Token id, UTI typetoreturn, CompilerState& state ) : Symbol(id,typetoreturn,state), m_functionNode(NULL), m_hasVariableArgs(false)
  {
    setDataMember(); // by definition all function definitions are data members
  }

  SymbolFunction::SymbolFunction(const SymbolFunction& sref) : Symbol(sref), m_hasVariableArgs(sref.m_hasVariableArgs)
  {
    //parameters belong to functiondefinition block's ST; do not clone them again here!
    if(sref.m_functionNode)
      {
	m_functionNode = (NodeBlockFunctionDefinition *) sref.m_functionNode->instantiate();
	m_state.pushCurrentBlockAndDontUseMemberBlock(m_functionNode);
	for(u32 i = 0; i < sref.m_parameterSymbols.size(); i++)
	  {
	    u32 pid = sref.m_parameterSymbols[i]->getId();
	    Symbol * sym = NULL; //NOT here: sref.m_parameterSymbols[i]->clone();
	    assert(m_state.alreadyDefinedSymbol(pid, sym));
	    m_parameterSymbols.push_back(sym);
	  }
	m_state.popClassContext();
	m_functionNode->setFuncSymbolPtr(this); //might as well
      }
    else
      {
	m_functionNode = NULL; //is this possible?
	std::ostringstream msg;
	msg << "Undefined function block <";
	msg << m_state.m_pool.getDataAsString(getId()).c_str() << ">";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	//assert(0);
      }
  }

  SymbolFunction::~SymbolFunction()
  {
    delete m_functionNode;
    // symbols belong to  NodeBlockFunctionDefinition's ST; deleted there.
    m_parameterSymbols.clear();
  }

  Symbol * SymbolFunction::clone()
  {
    return new SymbolFunction(*this);
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

  u32 SymbolFunction::getTotalParameterSlots()
  {
    u32 totalsizes = 0;
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	totalsizes += m_state.slotsNeeded(sym->getUlamTypeIdx());
      }
    return totalsizes;
  } //getTotalParameterSlots

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
      delete m_functionNode; //clean up any previous declarations

    m_functionNode = func; //could be null if error occurs while parsing func body
    Symbol::setBlockNoOfST(m_state.getClassBlockNo()); //SF not in the func def ST
  } //setFunctionNode

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
    mangled << Symbol::getMangledName(); //e.g. Uf_14name, with lexNumbers

    // use void type when no parameters
    if(m_parameterSymbols.empty())
      {
	UTI avuti = Void;
	mangled << "," << avuti;
      }

    // append UTI for each parameter
    // note: though Classes (as args) may be 'incomplete' (i.e. bit size == UNKNOWN),
    //        during this parse stage, the key remains consistent.
    // many UTI -to- one key, how does this impact the scheme?
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
    mangled << Symbol::getMangledName(); //e.g. Uf_14name, with lexNumbers

    // use void type when no parameters
    if(m_parameterSymbols.empty())
      {
	UlamType * vit = m_state.getUlamTypeByIndex(Void);
	mangled << vit->getUlamTypeMangledName().c_str();
      }

    // append mangled type name, e.g. 1023213Int, for each parameter
    // note: though Classes (as args) may be 'incomplete' (i.e. bit size == UNKNOWN),
    //        during this parse stage, the key remains consistent.
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	UlamType * sut = m_state.getUlamTypeByIndex(sym->getUlamTypeIdx());
	mangled << sut->getUlamTypeMangledName().c_str();
      }
    return mangled.str();
  } //getMangledNameWithTypes

  bool SymbolFunction::checkParameterTypes()
  {
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func); //how would a function symbol be without a body? perhaps an ACCESSOR to-be-made?
    return func->checkParameterNodeTypes();
  } //checkParamterTypes

  bool SymbolFunction::matchingTypesStrictly(std::vector<UTI> argTypes)
  {
    u32 numArgs = argTypes.size();
    u32 numParams = m_parameterSymbols.size();

    // numArgs could be greater if this function takes variable args
    // check number of args first
    if(numArgs < numParams || (numArgs > numParams && !takesVariableArgs()))
      return false;

    bool rtnBool = true;
    //next match types; order counts!
    for(u32 i=0; i < numParams; i++)
      {
	UTI puti = m_parameterSymbols.at(i)->getUlamTypeIdx();
	if(UlamType::compare(puti, argTypes[i], m_state) != UTIC_SAME)
	  {
	    rtnBool = false;
	    break;
	  }
      } //next param
    return rtnBool;
  } //matchingTypesStrictly

  bool SymbolFunction::matchingTypes(std::vector<UTI> argTypes, std::vector<Node *> constantArg, bool& hasHazyArgs)
  {
    u32 numArgs = argTypes.size();
    u32 numParams = m_parameterSymbols.size();

    // numArgs could be greater if this function takes variable args
    // check number of args first
    if(numArgs < numParams || (numArgs > numParams && !takesVariableArgs()))
      return false;

    bool rtnBool = true;
    //next, liberally match types; order counts!
    for(u32 i=0; i < numParams; i++)
      {
	UTI puti = m_parameterSymbols.at(i)->getUlamTypeIdx();
	// if(UlamType::compare(puti, argTypes[i], m_state) == UTIC_NOTSAME) //same|not ready
	if(UlamType::compare(puti, argTypes[i], m_state) != UTIC_SAME) //not same|not ready
	  {
	    if(constantArg[i])
	      {
		assert(constantArg[i]->isAConstant());
		//constants can match any bit size, that it fits
		FORECAST scr = constantArg[i]->safeToCastTo(puti);
		if( scr == CAST_BAD)
		  {
		    rtnBool = false;
		    break;
		  }
		else if(scr == CAST_HAZY)
		  hasHazyArgs = true;
		//else CAST_CLEAR
	      } //constantarg
	    else
	      {
		//willing to cast argType safely TO puti; incomplete types are hazy.
		FORECAST scr = m_state.getUlamTypeByIndex(puti)->safeCast(argTypes[i]);
		if( scr == CAST_BAD)
		  {
		    rtnBool = false;
		    break;
		  }
		else if(scr == CAST_HAZY)
		  hasHazyArgs = true;
		//else CAST_CLEAR
	      }
	  }
      } //next param
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
	  fp->write("template<class EC, u32 POS>\n");
	else
	  fp->write("template<class EC>\n");
	m_state.indent(fp);
      }

    fp->write(sut->getImmediateStorageTypeAsString().c_str()); //return type for C++
    fp->write(" ");
    if(!declOnly)
      {
	UTI cuti = m_state.getCompileThisIdx();
	//include the mangled class::
	fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());

	if(classtype == UC_QUARK)
	  fp->write("<EC, POS>");
	else
	  fp->write("<EC>");

	fp->write("::");
      }

    fp->write(getMangledName().c_str());
    fp->write("(");

    fp->write("UlamContext<EC>& uc, "); //first arg is unmangled context

    //the hidden arg is "self", a T& (atom)
    fp->write("T& "); //a reference
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
	fp->write(", ..."); //ellipses must be after at least one param
      }

    fp->write(")");

    if(declOnly)
      {
	if(func->isNative())
	  fp->write("; //native\n\n");
	else
	  {
	    if(classtype == UC_ELEMENT)
	      fp->write(" const"); //element functions are const, not static

	    fp->write(";\n\n");
	  }
      }
    else
      {
	if(classtype == UC_ELEMENT)
	  fp->write(" const"); //element functions are const, not static

	UlamValue uvpass;
	func->genCode(fp, uvpass);
      }
  } //generateFunctionDeclaration

} //end MFM
