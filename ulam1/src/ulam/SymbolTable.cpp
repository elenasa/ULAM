#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTable.h"
#include "SymbolFunctionName.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTable::SymbolTable(CompilerState& state): m_state(state){}
  SymbolTable::SymbolTable(const SymbolTable& ref) : m_state(ref.m_state)
  {
    std::map<u32, Symbol* >::const_iterator it = ref.m_idToSymbolPtr.begin();
    while(it != ref.m_idToSymbolPtr.end())
      {
	u32 fid = it->first;
	Symbol * found = it->second;
	Symbol * cloned = found->clone();
	addToTable(fid, cloned);
	it++;
      }
  }

  SymbolTable::~SymbolTable()
  {
    // need to delete contents; ownership transferred here!!
    for(std::size_t i = 0; i < m_idToSymbolPtr.size(); i++)
      {
	delete m_idToSymbolPtr[i];
      }
    m_idToSymbolPtr.clear();
  }


  bool SymbolTable::isInTable(u32 id, Symbol * & symptrref)
  {
    std::map<u32, Symbol* >::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	symptrref = it->second;
	assert( symptrref->getId() == id);
	return true;
      }
    return false;
  } //isInTable


  void SymbolTable::addToTable(u32 id, Symbol* sptr)
  {
    m_idToSymbolPtr.insert(std::pair<u32,Symbol*> (id,sptr));
  }

  void SymbolTable::replaceInTable(u32 oldid, u32 newid, Symbol * s)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(oldid);
    if(it != m_idToSymbolPtr.end())
      {
	Symbol * oldsym = it->second;
	assert(oldsym == s);
	m_idToSymbolPtr.erase(it);
	addToTable(newid, s);
      }
  } //replaceInTable

  Symbol * SymbolTable::getSymbolPtr(u32 id)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	return it->second;
      }
    return NULL;  //impossible!!
  } //getSymbolPtr


  u32 SymbolTable::getTableSize()
  {
    return (m_idToSymbolPtr.empty() ? 0 : m_idToSymbolPtr.size());
  }


  //called by NodeBlock.
  u32 SymbolTable::getTotalSymbolSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 totalsizes = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    u32 depth = ((SymbolFunctionName *) sym)->getDepthSumOfFunctions();
	    totalsizes += depth;
	    assert(0); //function symbols are not in same table as variables
	  }
	else
	  {
	    // typedefs don't contribute to the total bit size
	    if(!sym->isTypedef())
	      {
		totalsizes += m_state.slotsNeeded(sym->getUlamTypeIdx());
	      }
	  }
	it++;
      }
    return totalsizes;
  } //getTotalSymbolSize


  s32 SymbolTable::getTotalVariableSymbolsBitSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    s32 totalsizes = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());

	// don't count typedef's or element parameters toward total, nor named constants
	if(variableSymbolWithCountableSize(sym))
	  {
	    UTI sut = sym->getUlamTypeIdx();
	    s32 symsize = calcVariableSymbolTypeSize(sut);  //recursively

	    if(symsize == CYCLEFLAG)  // was < 0
	      {
		std::ostringstream msg;
		msg << "cycle error!!! " << m_state.getUlamTypeNameByIndex(sut).c_str();
		MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	      }
	    else if(symsize == EMPTYSYMBOLTABLE)
	      {
		symsize = 0;
		m_state.setBitSize(sut, symsize);  //total bits NOT including arrays
	      }
	    else if(symsize <= UNKNOWNSIZE)
	      {
		std::ostringstream msg;
		msg << "UNKNOWN !!! " << m_state.getUlamTypeNameByIndex(sut).c_str();
		MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
		totalsizes = UNKNOWNSIZE;
		break;
	      }
	    else
	      {
		m_state.setBitSize(sut, symsize);  //symsize does not include arrays
	      }
	    totalsizes += m_state.getTotalBitSize(sut); //covers up any unknown sizes; includes arrays
	  }
	it++;
      }
    return totalsizes;
  } //getTotalVariableSymbolsBitSize


  s32 SymbolTable::getMaxVariableSymbolsBitSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    s32 maxsize = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());

	// don't count typedef's or element parameters toward max
	if(variableSymbolWithCountableSize(sym))
	  {
	    UTI sut = sym->getUlamTypeIdx();
	    s32 symsize = calcVariableSymbolTypeSize(sut);  //recursively

	    if(symsize == CYCLEFLAG)  // was < 0
	      {
		std::ostringstream msg;
		msg << "cycle error!!!! " << m_state.getUlamTypeNameByIndex(sut).c_str();
		MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	      }
	    else if(symsize == EMPTYSYMBOLTABLE)
	      {
		symsize = 0;
		m_state.setBitSize(sut, symsize);  //total bits NOT including arrays
	      }
	    else
	      {
		m_state.setBitSize(sut, symsize);  //symsize does not include arrays
	      }

	    if((s32) m_state.getTotalBitSize(sut) > maxsize)
	      maxsize = m_state.getTotalBitSize(sut);  //includes arrays
	  }
	it++;
      }
    return maxsize;
  } //getMaxVariableSymbolsBitSize


  //#define OPTIMIZE_PACKED_BITS
#ifdef OPTIMIZE_PACKED_BITS
  // currently, packing is done by Nodes since the order of declaration is available;
  // but in case we may want to optimize the layout someday,
  // we keep this here since all the symbols are available in one place.
  void SymbolTable::packBitsForTableOfVariableDataMembers()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 offsetIntoAtom = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym) && !((SymbolClass *) sym)->isQuarkUnion())
	  {
	    //updates the offset with the bit size of sym
	    ((SymbolVariable *) sym)->setPosOffset(offsetIntoAtom);
	    offsetIntoAtom += m_state.getTotalBitSize(sym->getUlamTypeIdx());  // times array size
	  }
	it++;
      }
  }
#endif


  s32 SymbolTable::findPosOfUlamTypeInTable(UTI utype)
  {
    s32 posfound = -1;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym))
	  {
	    if(sym->getUlamTypeIdx() == utype)
	      {
		posfound = ((SymbolVariable *) sym)->getPosOffset();
		break;
	      }
	  }
	it++;
      }
    return posfound;
  } //findPosOfUlamTypeInTable


  // replaced with parse tree method to preserve order of declaration
  void SymbolTable::genCodeForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(!sym->isTypedef() && sym->isDataMember()) //including element parameters
	  {
	    ((SymbolVariable *) sym)->generateCodedVariableDeclarations(fp, classtype);
	  }
	it++;
      }
  } //genCodeForTableOfVariableDataMembers (unused)


  void SymbolTable::genCodeBuiltInFunctionsOverTableOfVariableDataMember(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    // 'has' applies to both quarks and elements
    UTI cuti = m_state.m_classBlock->getNodeType();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method not called directly\n");

	m_state.indent(fp);
	fp->write("s32 ");
	fp->write(m_state.getHasMangledFunctionName(cuti));
	fp->write("(const char * namearg) const;\n\n");
	return;
      }

    m_state.indent(fp);
    if(classtype == UC_ELEMENT)
      fp->write("template<class CC>\n");
    else if(classtype == UC_QUARK)
      fp->write("template<class CC, u32 POS>\n");
    else
      assert(0);

    m_state.indent(fp);
    fp->write("s32 ");  //return pos offset, or -1 if not found

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    if(classtype == UC_ELEMENT)
      fp->write("<CC>");
    else if(classtype == UC_QUARK)
      fp->write("<CC, POS>");

    fp->write("::");
    fp->write(m_state.getHasMangledFunctionName(cuti));
    fp->write("(const char * namearg) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym))
	  {
	    UTI suti = sym->getUlamTypeIdx();
	    UlamType * sut = m_state.getUlamTypeByIndex(suti);
	    if(sut->getUlamClass() == UC_QUARK)
	      {
		m_state.indent(fp);
		fp->write("if(!strcmp(namearg,\"");
		fp->write(sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str());
		fp->write("\")) return ");
		fp->write("(");
		fp->write_decimal(((SymbolVariable *) sym)->getPosOffset());
		fp->write(");   //pos offset\n");
	      }
	  }
	it++;
      }
    fp->write("\n");
    m_state.indent(fp);
    fp->write("return ");
    fp->write("(-1);   //not found\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}  //has\n\n");
  } //genCodeBuiltInFunctionsOverTableOfVariableDataMember


  // storage for class members persists, so we give up preserving
  // order of declaration that the NodeVarDecl in the parseTree
  // provides, in order to distinguish between an instance's data
  // members on the STACK verses the classes' data members in
  // EVENTWINDOW.
  void SymbolTable::printPostfixValuesForTableOfVariableDataMembers(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isTypedef() || sym->isConstant() || (sym->isDataMember() && !sym->isElementParameter()))
	{
	  sym->printPostfixValuesOfVariableDeclarations(fp, slot, startpos, classtype);
	}
	it++;
      }
  } //printPostfixValuesForTableOfVariableDataMembers


  // convert UTI to mangled strings to insure overload uniqueness
  void SymbolTable::checkTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->checkFunctionNames();
	  }
	it++;
      }
  } //checkTableOfFunctions


  void SymbolTable::labelTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->labelFunctions();
	  }
	it++;
      }
  } //labelTableOfFunctions


  void SymbolTable::countNavNodesAcrossTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->countNavNodesInFunctionDefs();
	  }
	it++;
      }
  } //countNavNodesAcrossTableOfFunctions


  //called by current Class block on its function ST
  bool SymbolTable::checkCustomArrayTypeFuncs()
  {
    bool rtnBool = false;
    Symbol * fnsym = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsym))
      {
	// not necessarily a native function..tis prudent to remember
	UTI futi = fnsym->getUlamTypeIdx();
	assert(futi != Void);

	// set class type to custom array; the current class block
	// node type was set to its class symbol type at start of parsing it.
	UTI cuti = m_state.m_classBlock->getNodeType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	assert(((UlamTypeClass *) cut)->isCustomArray());
	rtnBool = true;

	{
	  std::ostringstream msg;
	  msg << "Custom array get method: '" << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str() << "' FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	  MSG("", msg.str().c_str(), DEBUG);
	}

	// check that its 'set' function has a matching parameter type
	fnsym = NULL;
	if(isInTable(m_state.getCustomArraySetFunctionNameId(), fnsym))
	  {
	    SymbolFunction * funcSymbol = NULL;
	    std::vector<UTI> argTypes;
	    argTypes.push_back(Int);
	    argTypes.push_back(futi);
	    if(!((SymbolFunctionName *) fnsym)->findMatchingFunction(argTypes, funcSymbol))
	      {
		std::ostringstream msg;
		msg << "Custom array set method: '" << m_state.m_pool.getDataAsString(fnsym->getId()).c_str() << "' does not match '" << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str() << "' argument types: ";
		for(u32 i = 0; i < argTypes.size(); i++)
		  {
		    msg << m_state.getUlamTypeNameByIndex(argTypes[i]).c_str() << ", ";
		  }
		msg << "and cannot be called in class: " << cut->getUlamTypeNameOnly().c_str();
		MSG("", msg.str().c_str(), ERR);
		rtnBool = false;
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Custom array set method: '" << m_state.m_pool.getDataAsString(m_state.getCustomArraySetFunctionNameId()).c_str() << "' FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
		MSG("", msg.str().c_str(), DEBUG);
	      }
	    argTypes.clear();
	  }//set found
	else
	  {
	    std::ostringstream msg;
	    msg << "Custom array set method: '" << m_state.m_pool.getDataAsString(m_state.getCustomArraySetFunctionNameId()).c_str() << "' NOT FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	    MSG("", msg.str().c_str(), WARN);
	  }
      } //get found
    else
      {
	UTI cuti = m_state.m_classBlock->getNodeType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);

	std::ostringstream msg;
	msg << "Custom array get method: '" << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str() << "' NOT FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	MSG("", msg.str().c_str(), DEBUG);
	rtnBool = false;
      }
    return rtnBool;
  } //checkForAndInitializeClassCustomArrayType


  u32 SymbolTable::countNativeFuncDeclsForTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 nativeCount = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	nativeCount += ((SymbolFunctionName *) sym)->countNativeFuncDecls();
	it++;
      }
    return nativeCount;
  } //countNativeFuncDeclsForTableOfFunctions


  void SymbolTable::genCodeForTableOfFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->generateCodedFunctions(fp, declOnly, classtype);
	  }
	it++;
      }
  } //genCodeForTableOfFunctions


  void SymbolTable::testForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    m_state.m_err.clearCounts();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	if(classNode->findTestFunctionNode())
	  {
	    // set up an atom in eventWindow; init m_currentObjPtr to point to it
	    // set up STACK since func call not called
	    m_state.setupCenterSiteForTesting();

	    m_state.m_nodeEvalStack.addFrameSlots(1);     //prolog, 1 for return
	    s32 rtnValue = 0;
	    EvalStatus evs = classNode->eval();
	    if(evs != NORMAL)
	      {
		rtnValue =  -1;   //error!
	      }
	    else
	      {
		UlamValue rtnUV = m_state.m_nodeEvalStack.popArg();
		rtnValue = rtnUV.getImmediateData(32);
	      }

	    //#define CURIOUS_T3146
#ifdef CURIOUS_T3146
	    //curious..
	    {
	      UlamValue objUV = m_state.m_eventWindow.loadAtomFromSite(c0.convertCoordToIndex());
	      u32 data = objUV.getData(25,32);  //Int f.m_i (t3146)
	      std::ostringstream msg;
	      msg << "Output for m_i = <" << data << "> (expecting 4 for t3146)";
	      MSG("",msg.str().c_str() , INFO);
	    }
#endif

	    m_state.m_nodeEvalStack.returnFrame();       //epilog

	    fp->write("Exit status: " );    //in compared answer
	    fp->write_decimal(rtnValue);
	    fp->write("\n");
	  } //test eval
	it++;
      } //while

    // output informational warning and error counts
    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warning" << (warns > 1 ? "s " : " ") << "during eval";
	MSG("", msg.str().c_str(), INFO);
      }

    u32 errs = m_state.m_err.getErrorCount();
    if(errs > 0)
      {
	std::ostringstream msg;
	msg << errs << " TOO MANY EVAL ERRORS";
	MSG("", msg.str().c_str(), INFO);
      }
  } //testForTableOfClasses


  void SymbolTable::printPostfixForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	classNode->printPostfix(fp);
	it++;
      } //while
  } //printPostfixForTableOfClasses


  void SymbolTable::printForDebugForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	classNode->print(fp);
	it++;
      } //while
  } //printForDebugForTableOfClasses


  void SymbolTable::updateLineageForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	classNode->updateLineage(NULL);
	it++;
      } //while
  } //updateLineageForTableOfClasses


  void SymbolTable::checkCustomArraysForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	// custom array flag set at parse time
	UTI cuti = classNode->getNodeType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	if(((UlamTypeClass *) cut)->isCustomArray())
	  {
	    classNode->checkCustomArrayTypeFunctions();
	  }
	it++;
      }
  } //checkCustomArraysForTableOfClasses()



  void SymbolTable::labelTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	SymbolClassName * cnsym = (SymbolClassName *) (it->second);
	assert(cnsym->isClass());
	if( ((SymbolClass *) cnsym)->getUlamClass() == UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class: "  << m_state.getUlamTypeNameByIndex(cnsym->getUlamTypeIdx()).c_str() << " was never defined, fails labeling";
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	    //assert(0); wasn't a class at all, e.g. out-of-scope typedef/variable
	    break;
	  }
	else
	  {
	    cnsym->checkAndLabelClassInstances();
	  }
	it++;
      }
  } //labelTableOfClasses


  void SymbolTable::countNavNodesAcrossTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	{
	  NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	  assert(classNode);
	  m_state.m_classBlock = classNode;
	  m_state.m_currentBlock = m_state.m_classBlock;

	  u32 datamembercnt = 0;
	  classNode->countNavNodes(datamembercnt);
	  if(datamembercnt > 0)
	    {
	      std::ostringstream msg;
	      msg << datamembercnt << " data member nodes with illegal 'Nav' types remain in class <";
	      msg << m_state.m_pool.getDataAsString(sym->getId());
	      msg << ">";
	      MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    }
	}
	it++;
      }
  } //countNavNodesAcrossTableOfClasses


  // separate pass...after labeling all classes is completed;
  // purpose is to set the size of all the classes, by totalling the size
  // of their data members; returns true if all class sizes complete.
  bool SymbolTable::setBitSizeOfTableOfClasses()
  {
    std::vector<UTI> lostClasses;
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	if( ((SymbolClass *) sym)->getUlamClass() == UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class: "  << m_state.getUlamTypeNameByIndex(sym->getUlamTypeIdx()).c_str() << " was never defined, fails sizing";
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	    //m_state.completeIncompleteClassSymbol(sym->getUlamTypeIdx()); //too late
	    aok = false;  //moved here;
	  }

	//of course they always aren't! but we know to keep looping..
	UTI suti = sym->getUlamTypeIdx();
	if(! m_state.isComplete(suti))
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class Type: "  << m_state.getUlamTypeNameByIndex(suti).c_str() << " (UTI" << suti << ") has 'unknown' sizes, fails sizing pre-test";
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
	    aok = false;  //moved here;
	  }

	// try..
	aok = ((SymbolClassName *) sym)->setBitSizeOfClassInstances();

	//track classes that fail to be sized.
	if(!aok)
	  lostClasses.push_back(suti);

	aok = true; //reset for next class
	it++;
      } //next class

    aok = lostClasses.empty();
    if(!aok)
      {
	std::ostringstream msg;
	msg << lostClasses.size() << " Classes without sizes";
	while(!lostClasses.empty())
	  {
	    UTI cuti = lostClasses.back();
	    msg << ", " << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    lostClasses.pop_back();
	  }
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
      }
    else
      {
	std::ostringstream msg;
	msg << m_idToSymbolPtr.size() << " Class" <<( m_idToSymbolPtr.size() > 1 ? "es ALL " : " ") << "sized SUCCESSFULLY";
	MSG("", msg.str().c_str(),INFO);
      }
    lostClasses.clear();
    return aok;
  } //setBitSizeOfTableOfClasses

  // separate pass...after labeling all classes is completed;
  void SymbolTable::printBitSizeOfTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	((SymbolClassName *) sym)->printBitSizeOfClassInstances();
	it++;
      }
  } //printBitSizeOfTableOfClasses

  void SymbolTable::packBitsForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	// quark union keep default pos = 0 for each data member, hence skip packing bits.
	if(!((SymbolClass *) sym)->isQuarkUnion())
	  {
	    ((SymbolClassName *) sym)->packBitsForClassInstances();
	  }
	it++;
      }
  } //packBitsForTableOfClasses

  //bypasses THIS class being compiled
  void SymbolTable::generateIncludesForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());

	UTI suti = sym->getUlamTypeIdx();
	//if(sym->getId() != m_state.m_compileThisId)
	if(suti != m_state.m_compileThisIdx)
	  {
	    m_state.indent(fp);
	    fp->write("#include \"");
	    fp->write(m_state.getFileNameForAClassHeader(sym->getId(), suti).c_str());
	    fp->write("\"\n");
	  }
	it++;
      }
  } //generateIncludesForTableOfClasses


  //bypasses THIS class being compiled
  void SymbolTable::generateForwardDefsForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());

	if(sym->getId() != m_state.m_compileThisId)
	  {
	    UTI suti = sym->getUlamTypeIdx();
	    UlamType * sut = m_state.getUlamTypeByIndex(suti);
	    ULAMCLASSTYPE sclasstype = sut->getUlamClass();

	    m_state.indent(fp);
	    fp->write("namespace MFM { template ");
	    if(sclasstype == UC_QUARK)
	      fp->write("<class CC, u32 POS> ");
	    else if(sclasstype == UC_ELEMENT)
	      fp->write("<class CC> ");
	    else
	      assert(0);

	    fp->write("struct ");
	    fp->write(sut->getUlamTypeMangledName().c_str());
	    fp->write("; }  //FORWARD\n");
	  }
	it++;
      }
  } //generateForwardDefsForTableOfClasses


  std::string SymbolTable::generateTestInstancesForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    s32 idcounter = 1;
    std::ostringstream runThisTest;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());

	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	ULAMCLASSTYPE sclasstype = sut->getUlamClass();

	if(sclasstype == UC_QUARK)
	  {
	    it++;
	    continue;
	  }

	std::string namestr = sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state);
	std::string lowercasename = firstletterTolowercase(namestr);
	std::ostringstream ourname;
	ourname << "Our" << namestr;

	// only for elements
	fp->write("\n");
	m_state.indent(fp);
	fp->write("typedef ");
	fp->write("MFM::");
	fp->write(sut->getUlamTypeMangledName().c_str());

	fp->write("<OurCoreConfig> ");
	fp->write(ourname.str().c_str());
	fp->write(";\n");

	m_state.indent(fp);
	fp->write(ourname.str().c_str());
	fp->write("& ");
	fp->write(lowercasename.c_str());
	fp->write(" = ");
	fp->write(ourname.str().c_str());
	fp->write("::THE_INSTANCE;\n");

	m_state.indent(fp);
	fp->write(lowercasename.c_str());
	fp->write(".AllocateType();  // Force element type allocation now\n");
	m_state.indent(fp);
	fp->write("theTile.RegisterElement(");
	fp->write(lowercasename.c_str());
	fp->write(");\n");

	if(sym->getId() == m_state.m_compileThisId)
	  {
	    m_state.indent(fp);
	    fp->write("OurAtom ");
	    fp->write(lowercasename.c_str());
	    fp->write("Atom = ");
	    fp->write(lowercasename.c_str());
	    fp->write(".GetDefaultAtom();\n");

	    runThisTest << lowercasename.c_str() << ".Uf_4test(" << "uc, " << lowercasename.c_str() << "Atom)";
	  }
	it++;
	idcounter++;
      }
    fp->write("\n");

    return runThisTest.str();
  } //generateTestInstancesForTableOfClasses


  //not sure we use this; go back and forth between the files that are output
  // if just this class, then NodeProgram can start the ball rolling
  void SymbolTable::genCodeForTableOfClasses(FileManager * fm)
  {
    u32 saveCompileThisId = m_state.m_compileThisId;
    UTI saveCompileThisIdx = m_state.m_compileThisIdx;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    //m_state.m_err.clearCounts();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());

	//output header/body for this class next
	m_state.m_compileThisId = sym->getId();
	m_state.m_compileThisIdx = sym->getUlamTypeIdx();
	((SymbolClassName *) sym)->generateCodeForClassInstances(fm);
	it++;
      } //while

    m_state.m_compileThisId = saveCompileThisId;  //restore
    m_state.m_compileThisIdx = saveCompileThisIdx;  //restore
  } //genCodeForTableOfClasses


  // PRIVATE HELPER METHODS:
  s32 SymbolTable::calcVariableSymbolTypeSize(UTI argut)
  {
    if(!m_state.isComplete(argut))
       m_state.constantFoldIncompleteUTI(argut);

    s32 totbitsize = m_state.getBitSize(argut);

    if(m_state.getUlamTypeByIndex(argut)->getUlamClass() == UC_NOTACLASS) //includes Atom type
      {
	return totbitsize;  //arrays handled by caller, just bits here
      }

    //not a primitive (class), array
    if(m_state.getArraySize(argut) > 0)
      {
	if(totbitsize >= 0)
	  {
	    return totbitsize;
	  }
	if(totbitsize == CYCLEFLAG)  //was < 0
	  {
	    assert(0);
	    return CYCLEFLAG;
	  }
	if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0;  //empty, ok
	  }
	else
	  {
	    assert(totbitsize <= UNKNOWNSIZE || m_state.getArraySize(argut) == UNKNOWNSIZE);

	    m_state.setBitSize(argut, CYCLEFLAG);  //before the recusive call..

	    //get base type, scalar type of class
	    SymbolClass * csym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(argut, csym))
	      {
		return calcVariableSymbolTypeSize(csym->getUlamTypeIdx());  // NEEDS CORRECTION
	      }
	  }
      }
    else  // not primitive type (class), and not array (scalar)
      {
	if(totbitsize >= 0)
	  {
	    return totbitsize;
	  }

	if(totbitsize == CYCLEFLAG) // was < 0
	  {
	    return CYCLEFLAG;       //error! cycle
	  }
	else if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0;  //empty, ok
	  }
	else
	  {
	    assert(totbitsize == UNKNOWNSIZE);
	    //get base type
	    SymbolClass * csym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(argut, csym))
	      {
		s32 csize;
		if((csize = m_state.getBitSize(csym->getUlamTypeIdx())) >= 0)
		  {
		    return csize;
		  }
		else if(csize == CYCLEFLAG)  // was < 0
		  {
		    //error! cycle..replace with message
		    return csize;
		  }
		else if(csize == EMPTYSYMBOLTABLE)
		  {
		    return 0;  //empty, ok
		  }
		else
		  {
		    // ==0, redo variable total
		    NodeBlockClass * classblock = csym->getClassBlockNode();
		    assert(classblock);

		    //quark cannot contain a copy of itself!
		    assert(classblock != m_state.m_classBlock);

		    if(csym->isQuarkUnion())
		      csize = classblock->getMaxBitSizeOfVariableSymbolsInTable();
		    else
		      csize = classblock->getBitSizesOfVariableSymbolsInTable(); //data members only
		    return csize;
		  }
	      }
	  } //totbitsize == 0
      } //not primitive, not array
    return CYCLEFLAG;
  } //calcVariableSymbolTypeSize (recursively)

  bool SymbolTable::variableSymbolWithCountableSize(Symbol * sym)
  {
    return (!sym->isTypedef() && !sym->isElementParameter() && !sym->isConstant());
  }

  std::string SymbolTable::firstletterTolowercase(const std::string s) //static method
  {
    std::ostringstream up;
    assert(!s.empty());
    std::string c(1,(s[0] >= 'A' && s[0] <= 'Z') ? s[0]-('A'-'a') : s[0]);
    up << c << s.substr(1,s.length());
    return up.str();
  } //firstletterTolowercase

} //end MFM
