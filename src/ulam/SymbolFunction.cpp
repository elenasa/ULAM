#include "SymbolFunction.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  SymbolFunction::SymbolFunction(const Token& id, UTI typetoreturn, CompilerState& state ) : Symbol(id,typetoreturn,state), m_functionNode(NULL), m_hasVariableArgs(false), m_isVirtual(false), m_pureVirtual(false), m_insureVirtualOverride(false), m_isConstructor(false), m_definedinaQuark(false)
  {
    setDataMemberClass(m_state.getCompileThisIdx()); // by definition all function definitions are data members
  }

  SymbolFunction::SymbolFunction(const SymbolFunction& sref) : Symbol(sref), m_hasVariableArgs(sref.m_hasVariableArgs), m_isVirtual(sref.m_isVirtual), m_pureVirtual(sref.m_pureVirtual), m_insureVirtualOverride(sref.m_insureVirtualOverride), m_isConstructor(sref.m_isConstructor), m_definedinaQuark(sref.m_definedinaQuark)
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
  }

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
    Symbol::setBlockNoOfST(m_state.getContextBlockNo()); //SF not in the func def ST
  }

  NodeBlockFunctionDefinition *  SymbolFunction::getFunctionNode()
  {
    return m_functionNode;
  }

  const std::string SymbolFunction::getMangledPrefix()
  {
    return "Uf_";
  }

  const std::string SymbolFunction::getFunctionNameWithTypes()
  {
    std::ostringstream fname;
    fname << m_state.getUlamTypeNameBriefByIndex(getUlamTypeIdx()).c_str(); //return type
    fname << " ";
    fname << m_state.m_pool.getDataAsString(getId()); //ulam func name

    fname << "(";

    u32 numParams = m_parameterSymbols.size();
    for(u32 i = 0; i < numParams; i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);

	if(i > 0)
	  fname << ", ";
	fname << sut->getUlamTypeNameBrief().c_str();
	fname << " ";
	fname << m_state.m_pool.getDataAsString(sym->getId());
      }
    fname << ")";
    return fname.str();
  } //getFunctionNameWithTypes

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

  bool SymbolFunction::matchingTypesStrictly(std::vector<UTI> argTypes, bool& hasHazyArgs)
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
	if(!m_state.okUTItoContinue(puti) || !m_state.isComplete(puti))
	  hasHazyArgs = true;
	UTI auti = argTypes[i];
	if(UlamType::compareForArgumentMatching(puti, auti, m_state) != UTIC_SAME)
	  {
	    rtnBool = false;
	    break;
	  }
      } //next param
    return rtnBool;
  } //matchingTypesStrictly

  bool SymbolFunction::matchingTypesStrictly(std::vector<Node *> argNodes, bool& hasHazyArgs)
  {
    u32 numArgs = argNodes.size();
    std::vector<UTI> argTypes;
    for(u32 i=0; i < numArgs; i++)
      {
	UTI auti = argNodes[i]->getNodeType();
	argTypes.push_back(auti);
      } //next arg
    return matchingTypesStrictly(argTypes, hasHazyArgs);
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
	    UlamType * put = m_state.getUlamTypeByIndex(puti);
	    UlamType * aut = m_state.getUlamTypeByIndex(auti);
	    if(argNodes[i]->isAConstant())
	      {
		if(put->isAltRefType())
		  {
		    if(!((SymbolVariableStack *) m_parameterSymbols.at(i))->isConstantFunctionParameter())
		      {
			rtnBool = false;
			break;
		      }
		    else if(argNodes[i]->isExplicitCast())
		      {
			rtnBool = false; //t41256
			break;
		      }
		  }
		//else (t41238, t41240, t3114..)

		//constants can match any bit size, that it fits; not non-constant reference types
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
		    if(put->getUlamTypeEnum() == aut->getUlamTypeEnum())
		      numUTmatch++;
		  }
	      } //constantarg
	    // catch it later for a better error message!!
	    //else if(argNodes[i]->isFunctionCall() && m_state.isAltRefType(puti))
	    else
	      {
		//willing to cast arg Type safely TO puti; incomplete types are hazy.
		//except for references.
		FORECAST scr = put->safeCast(auti);
		if(scr == CAST_BAD)
		  {
		    rtnBool = false;
		    break;
		  }
		else if(scr == CAST_HAZY)
		  hasHazyArgs = true;
		else //CAST_CLEAR
		  {
		    if(m_state.isConstantRefType(puti) && argNodes[i]->isExplicitCast())
		      {
			rtnBool = false; //t41258
			break;
		      }
		    if(put->getUlamTypeEnum() == aut->getUlamTypeEnum())
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
  }

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

  bool SymbolFunction::getInsureVirtualOverrideFunction()
  {
    return m_insureVirtualOverride;
  }

  void SymbolFunction::setInsureVirtualOverrideFunction()
  {
    m_insureVirtualOverride = true;
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

  bool SymbolFunction::isConstructorFunction()
  {
    return m_isConstructor;
  }

  void SymbolFunction::setConstructorFunction()
  {
    m_isConstructor = true;
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

    if(!declOnly)
      {
	Locator floc = func->getNodeLocation();
	m_state.outputTextAsCommentWithLocationUpdate(fp, floc);
      }

    UTI suti = getUlamTypeIdx();
    UlamType * sut = m_state.getUlamTypeByIndex(suti); //return type

    if(declOnly)
      {
	m_state.indent(fp);
	//only ulam virtual functions are c++ static functions;
	// because of vtable, cast void * from a reference.
	if(isVirtualFunction())
	  fp->write("static ");
      }
    else
      {
	m_state.indentUlamCode(fp);
	fp->write("template<class EC>\n"); //same for elements and quarks
	m_state.indentUlamCode(fp);
      }

    fp->write(sut->getLocalStorageTypeAsString().c_str()); //return type for C++
    fp->write(" ");
    if(!declOnly)
      {
	UTI cuti = m_state.getCompileThisIdx();
	//include the mangled class::
	fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
      }

    fp->write(getMangledName().c_str());
    fp->write("(");

    fp->write("const UlamContext<EC>& uc, "); //first arg is unmangled context
    //the hidden arg is "atom" (was "self"), a T& (atom), now UlamRef (ur)
    fp->write("UlamRef<EC>& "); //a reference
    fp->write(m_state.getHiddenArgName());

    u32 numparams = getNumberOfParameters();

    for(u32 i = 0; i < numparams; i++)
      {
	fp->write(", ");

	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	assert(asym->isFunctionParameter()); //sanity
	UTI auti = asym->getUlamTypeIdx();
	UlamType * aut = m_state.getUlamTypeByIndex(auti);
	fp->write(aut->getLocalStorageTypeAsString().c_str()); //for C++
	fp->write("&"); //gen C++ reference for funcs args; avoids g++ synthesized copy constructor
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
	    GCNL;
	    fp->write("\n");
	  }
	else
	  {
	    fp->write(" const"); //quark and element functions (incl natives) are const, not c++ static
	    if(func->isNative())
	      {
		fp->write("; //native"); GCNL;
	      }
	    else
	      {
		fp->write(";"); GCNL;
	      }
	    fp->write("\n");
	  }
      }
    else
      {
	if(!isVirtualFunction())
	  fp->write(" const"); //quark and element functions (incl natives) are const, not c++ static

	UVPass uvpass;
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
    assert(declOnly);
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
    fp->write("UlamRef<EC>& "); //the hidden arg is "atom" (was "self"), a T& (atom), now UlamRef (self)

    u32 numparams = getNumberOfParameters();
    for(u32 i = 0; i < numparams; i++)
      {
	fp->write(", ");

	Symbol * asym = getParameterSymbolPtr(i);
	assert(asym);
	UTI auti = asym->getUlamTypeIdx();
	UlamType * aut = m_state.getUlamTypeByIndex(auti);
	fp->write(aut->getLocalStorageTypeAsString().c_str()); //for C++
	fp->write("&"); //gen C++ reference for func args; avoids g++ synthesized copy constructor
      }
    fp->write(")");
    fp->write(";"); GCNL;
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

  //for Ulam Info, with any Ulam typedefs
  const std::string SymbolFunction::generateUlamFunctionSignature()
  {
    NodeBlockFunctionDefinition * func = getFunctionNode();
    assert(func); //how would a function symbol be without a body?

    std::ostringstream sig;

    if(isVirtualFunction())
      sig << "virtual ";

    sig << m_state.m_pool.getDataAsString(func->getTypeNameId()).c_str(); //return type
    sig << " " << m_state.m_pool.getDataAsString(getId()).c_str()  << "("; //func name

    u32 numparams = getNumberOfParameters();

    for(u32 i = 0; i < numparams; i++)
      {
	if(i > 0)
	  sig << ", ";

	Node * pnode = func->getParameterNode(i);
	assert(pnode);
	sig << m_state.m_pool.getDataAsString(pnode->getTypeNameId()).c_str();
	sig << " " << pnode->getName(); //arg name
      }

    if(takesVariableArgs())
      {
	assert(func->isNative());
	sig << ", ..."; //ellipses must be after at least one param
      }

    sig << ")"; //end of args

    if(func->isNative())
      sig << " native";

    return sig.str();
  } //generateUlamFunctionSignature

} //end MFM
