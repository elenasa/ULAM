#include "SymbolFunction.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolFunction::SymbolFunction(Token id, UTI typetoreturn, CompilerState& state ) : Symbol(id,typetoreturn,state), m_functionNode(NULL), m_hasVariableArgs(false), m_isVirtual(false), m_pureVirtual(false), m_definedinaQuark(false)
  {
    setDataMemberClass(m_state.getCompileThisIdx()); // by definition all function definitions are data members
  }

  SymbolFunction::SymbolFunction(const SymbolFunction& sref) : Symbol(sref), m_hasVariableArgs(sref.m_hasVariableArgs), m_isVirtual(sref.m_isVirtual), m_pureVirtual(sref.m_pureVirtual), m_definedinaQuark(sref.m_definedinaQuark)
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
	    bool hazyKin = false; //don't care?
	    AssertBool isDefined = m_state.alreadyDefinedSymbol(pid, sym, hazyKin);
	    assert(isDefined);
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

  UTI SymbolFunction::getParameterType(u32 n)
  {
    assert(n < m_parameterSymbols.size());
    return m_parameterSymbols[n]->getUlamTypeIdx();
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

  //supports overloading functions with SymbolFunctionName
  const std::string SymbolFunction::getMangledNameWithTypes()
  {
    std::ostringstream mangled;
    mangled << Symbol::getMangledName(); //e.g. Uf_14name, with lexNumbers

    u32 numParams = m_parameterSymbols.size();
    mangled << ToLeximitedNumber(numParams); //"10" if none

    // append mangled type name, e.g. 1023213Int, for each parameter
    // for template classes, including arg values 10133Bar121013iu1510111b11 for uniqueness
    // note: though Classes (as args) may be 'incomplete' (i.e. bit size == UNKNOWN),
    //        during this parse stage, the key remains consistent.
    for(u32 i = 0; i < numParams; i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);

	// dropping Uprefix:
	mangled << sut->getUlamTypeMangledType().c_str();

	//append args' types and values or '10' is none
	if(sut->getUlamTypeEnum() == Class)
	  {
	    UlamKeyTypeSignature keyOfArgType = sut->getUlamKeyTypeSignature();

	    SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(keyOfArgType.getUlamKeyTypeSignatureNameId());
	    assert(cnsym);
	    mangled << cnsym->formatAnInstancesArgValuesAsAString(suti);
	  }
      }
    return mangled.str();
  } //getMangledNameWithTypes

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
	UTI auti = argTypes[i];
	if(UlamType::compareForArgumentMatching(puti, auti, m_state) != UTIC_SAME)
	  {
	    rtnBool = false;
	    break;
	  }
      } //next param
    return rtnBool;
  } //matchingTypesStrictly

  bool SymbolFunction::matchingTypesStrictly(std::vector<Node *> argNodes)
  {
    u32 numArgs = argNodes.size();
    std::vector<UTI> argTypes;
    for(u32 i=0; i < numArgs; i++)
      {
	UTI auti = argNodes[i]->getNodeType();
	argTypes.push_back(auti);
      } //next arg
    return matchingTypesStrictly(argTypes);
  } //matchingTypesStrictly

  bool SymbolFunction::matchingTypes(std::vector<Node *> argNodes, bool& hasHazyArgs, u32& numUTmatch)
  {
    u32 numArgs = argNodes.size();
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
	UTI auti = argNodes[i]->getNodeType();
	if(UlamType::compareForArgumentMatching(puti, auti, m_state) != UTIC_SAME) //not same|not ready
	  {
	    if(argNodes[i]->isAConstant())
	      {
		if(m_state.isReference(puti))
		  {
		    rtnBool = false;
		    break;
		  }
		//constants can match any bit size, that it fits; not reference types
		FORECAST scr = argNodes[i]->safeToCastTo(puti);
		if(scr == CAST_BAD)
		  {
		    rtnBool = false;
		    break;
		  }
		else if(scr == CAST_HAZY)
		  hasHazyArgs = true;
		else //CAST_CLEAR
		  {
		    if(m_state.getUlamTypeByIndex(puti)->getUlamTypeEnum() == m_state.getUlamTypeByIndex(auti)->getUlamTypeEnum())
		      numUTmatch++;
		  }
	      } //constantarg
	    // catch it later for a better error message!!
	    //else if(argNodes[i]->isFunctionCall() && m_state.isReference(puti))
	    //  {
	    //		rtnBool = false;
	    //		break;
	    //  }
	    else
	      {
		//willing to cast arg Type safely TO puti; incomplete types are hazy.
		FORECAST scr = m_state.getUlamTypeByIndex(puti)->safeCast(auti);
		if(scr == CAST_BAD)
		  {
		    rtnBool = false;
		    break;
		  }
		else if(scr == CAST_HAZY)
		  hasHazyArgs = true;
		else //CAST_CLEAR
		  {
		    if(m_state.getUlamTypeByIndex(puti)->getUlamTypeEnum() == m_state.getUlamTypeByIndex(auti)->getUlamTypeEnum())
		      numUTmatch++;
		  }
	      }
	  }
	else
	  numUTmatch++;
      } //next param
    return rtnBool;
  } //matchingTypes

  u32 SymbolFunction::isNativeFunctionDeclaration()
  {
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func);
    return (func->isNative() ? 1 : 0);
  } //isNativeFunctionDeclaration

  bool SymbolFunction::isVirtualFunction()
  {
    return m_isVirtual;
  }

  void SymbolFunction::setVirtualFunction()
  {
    m_isVirtual = true;
  }

  bool SymbolFunction::isPureVirtualFunction()
  {
    assert(isVirtualFunction());
    return m_pureVirtual;
  }

  void SymbolFunction::setPureVirtualFunction()
  {
    assert(isVirtualFunction());
    m_pureVirtual = true;
  }

  u32 SymbolFunction::getVirtualMethodIdx()
  {
    assert(isVirtualFunction());
    return m_virtualIdx;
  }

  void SymbolFunction::setVirtualMethodIdx(u32 idx)
  {
    assert(isVirtualFunction());
    m_virtualIdx = idx;
  }

  bool SymbolFunction::isDefinedInAQuark()
  {
    return m_definedinaQuark;
  }

  void SymbolFunction::setDefinedInAQuark()
  {
    m_definedinaQuark = true;
  }

  void SymbolFunction::generateFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func); //how would a function symbol be without a body?

    //up to programmer to define this function!!!
    if(!declOnly && (func->isNative() || (isVirtualFunction() && isPureVirtualFunction())))
      return;

    m_state.outputTextAsComment(fp, func->getNodeLocation());

    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx()); //return type

    m_state.indent(fp);
    if(declOnly)
      {
	//only ulam virtual functions are c++ static functions
	if(isVirtualFunction())
	  fp->write("static ");
      }
    else
      {
	if(classtype == UC_QUARK)
	  fp->write("template<class EC, u32 POS>\n");
	else
	  fp->write("template<class EC>\n");
	m_state.indent(fp);
      }

    fp->write(sut->getLocalStorageTypeAsString().c_str()); //return type for C++
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

    fp->write("const UlamContext<EC>& uc, "); //first arg is unmangled context

    //the hidden arg is "atom" (was "self"), a T& (atom)
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
	fp->write(aut->getLocalStorageTypeAsString().c_str()); //for C++
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
	if(isVirtualFunction())
	  {
	    fp->write("; //virtual");
	    if(func->isNative())
	      fp->write(", native");
	    fp->write("\n\n");
	  }
	else
	  {
	    fp->write(" const"); //quark and element functions (incl natives) are const, not c++ static
	    if(func->isNative())
	      fp->write("; //native\n\n");
	    else
	      fp->write(";\n\n");
	  }
      }
    else
      {
	if(!isVirtualFunction())
	  fp->write(" const"); //quark and element functions (incl natives) are const, not c++ static

	UlamValue uvpass;
	func->genCode(fp, uvpass);
      }

    if(declOnly && isVirtualFunction()) //can be native too
      generateFunctionDeclarationVirtualTypedef(fp, declOnly, classtype);
  } //generateFunctionDeclaration

  void SymbolFunction::generateFunctionDeclarationVirtualTypedef(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func); //how would a function symbol be without a body?
                  //natives may also be virtuals.

    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx()); //return type

    m_state.indent(fp);
    fp->write("//and its contextual type info for virtual table entries:\n");

    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(sut->getLocalStorageTypeAsString().c_str()); //return type for C++
    fp->write(" (");

    fp->write("* "); //ptr to
    fp->write(getMangledNameWithTypes().c_str());
    fp->write(") (");

    //arg types only
    fp->write("const UlamContext<EC>&, "); //first arg is unmangled context
    fp->write("T& "); //the hidden arg is "atom" (was "self"), a T& (atom)

    u32 numparams = getNumberOfParameters();
    for(u32 i = 0; i < numparams; i++)
      {
	fp->write(", ");

	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	UTI auti = asym->getUlamTypeIdx();
	UlamType * aut = m_state.getUlamTypeByIndex(auti);
	fp->write(aut->getLocalStorageTypeAsString().c_str()); //for C++
      }
    fp->write(")");
    fp->write(";\n");
  } //generateFunctionDeclarationVirtualTypedef

  void SymbolFunction::setStructuredComment()
  {
    Token scTok;
    if(m_state.getStructuredCommentToken(scTok)) //and clears it
      {
	m_structuredCommentToken = scTok;
	m_gotStructuredCommentToken = true;
      }
  } //setStructuredComment

} //end MFM
