#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTable.h"
#include "SymbolFunctionName.h"
#include "SymbolParameterValue.h"
#include "SymbolVariable.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "NodeBlock.h"
#include "Node.h"

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
    //need to delete contents; ownership transferred here!!
    clearTheTable();
  }

  void SymbolTable::clearTheTable()
  {
    for(std::size_t i = 0; i < m_idToSymbolPtr.size(); i++)
      {
	delete m_idToSymbolPtr[i];
      }
    m_idToSymbolPtr.clear();
  } //clearTheTable

  bool SymbolTable::isInTable(u32 id, Symbol * & symptrref)
  {
    if(m_idToSymbolPtr.empty()) return false;

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
    else
      {
	addToTable(newid, s);
      }
  } //replaceInTable

  void SymbolTable::replaceInTable(Symbol * oldsym, Symbol * newsym)
  {
    assert(oldsym->getId() == newsym->getId());
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(oldsym->getId());
    if(it != m_idToSymbolPtr.end())
      {
	Symbol * oldsymptr = it->second;
	assert(oldsymptr == oldsym);
	it->second = newsym;
	delete(oldsymptr);
      }
  } //replaceInTable (overload)

  bool SymbolTable::removeFromTable(u32 id, Symbol *& rtnsymptr)
  {
    bool rtnok = false;
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	rtnsymptr = it->second;
	m_idToSymbolPtr.erase(it); //doesn't delete, up to caller
	rtnok = true;
      }
    return rtnok;
  } //removeFromTable

#if 0
  bool SymbolTable::mergeTables(NodeBlock * toTable)
  {
    if(m_idToSymbolPtr.empty()) return true;

    std::vector<Symbol *> copyList;
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	u32 sid = it->first;
	Symbol * fsym = it->second;
	assert(fsym);
	UTI fsuti = fsym->getUlamTypeIdx();
	Symbol * tsym = NULL;
	if(toTable->isIdInScope(sid, tsym))
	  {
	    UTI tsuti = tsym->getUlamTypeIdx();
	    if(tsuti != fsuti)
	      {
		m_state.updateUTIAlias(fsuti, tsuti);
	      }
	  }
	else
	  {
	    copyList.push_back(fsym);
	  }
	it++;
      } //next symbol

    NNO toNo = toTable->getNodeNo();
    while(!copyList.empty())
      {
	Symbol * sym = copyList.back();
	u32 sid = sym->getId();
	Symbol * copysym = sym->cloneKeepsType();
	copysym->setBlockNoOfST(toNo);
	toTable->addIdToScope(sid, copysym);
	copyList.pop_back(); //no deletion
      }

    return copyList.empty();
  } //mergeTables
#endif

  Symbol * SymbolTable::getSymbolPtr(u32 id)
  {
    if(m_idToSymbolPtr.empty()) return NULL;

    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	return it->second;
      }
    return NULL; //impossible!!
  } //getSymbolPtr

  u32 SymbolTable::getTableSize()
  {
    return (m_idToSymbolPtr.empty() ? 0 : m_idToSymbolPtr.size());
  }

  //called by NodeBlock.
  u32 SymbolTable::getTotalSymbolSize()
  {
    u32 totalsizes = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    u32 depth = ((SymbolFunctionName *) sym)->getDepthSumOfFunctions();
	    totalsizes += depth;
	    //was used for debugging (see NodeBlockClass eval)
	    assert(0); //function symbols are not in same table as variables
	  }
	else
	  {
	    //typedefs don't contribute to the total bit size
	    if(variableSymbolWithCountableSize(sym))
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
    s32 totalsizes = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());

	if(!variableSymbolWithCountableSize(sym))
	  {
	    it++;
	    continue;
	  }

	UTI suti = sym->getUlamTypeIdx();
	s32 symsize = calcVariableSymbolTypeSize(suti); //recursively

	if(symsize == CYCLEFLAG) //was < 0
	  {
	    std::ostringstream msg;
	    msg << "cycle error!!! " << m_state.getUlamTypeNameByIndex(suti).c_str();
	    MSG(sym->getTokPtr(), msg.str().c_str(), ERR);
	  }
	else if(symsize == EMPTYSYMBOLTABLE)
	  {
	    symsize = 0;
	    m_state.setBitSize(suti, symsize); //total bits NOT including arrays
	  }
	else if(symsize <= UNKNOWNSIZE)
	  {
	    std::ostringstream msg;
	    msg << "UNKNOWN !!! " << m_state.getUlamTypeNameByIndex(suti).c_str();
	    msg << " UTI" << suti << " while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(sym->getTokPtr(), msg.str().c_str(), DEBUG);
	    totalsizes = UNKNOWNSIZE;
	    break;
	  }
	else
	  m_state.setBitSize(suti, symsize); //symsize does not include arrays

	totalsizes += m_state.getTotalBitSize(suti); //covers up any unknown sizes; includes arrays
	it++;
      } //while
    return totalsizes;
  } //getTotalVariableSymbolsBitSize

  s32 SymbolTable::getMaxVariableSymbolsBitSize()
  {
    s32 maxsize = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());

	if(!variableSymbolWithCountableSize(sym))
	  {
	    it++;
	    continue;
	  }

	UTI sut = sym->getUlamTypeIdx();
	s32 symsize = calcVariableSymbolTypeSize(sut); //recursively

	if(symsize == CYCLEFLAG) //was < 0
	  {
	    std::ostringstream msg;
	    msg << "cycle error!!!! " << m_state.getUlamTypeNameByIndex(sut).c_str();
	    MSG(sym->getTokPtr(), msg.str().c_str(),ERR);
	  }
	else if(symsize == EMPTYSYMBOLTABLE)
	  {
	    symsize = 0;
	    m_state.setBitSize(sut, symsize); //total bits NOT including arrays
	  }
	else
	  {
	    m_state.setBitSize(sut, symsize); //symsize does not include arrays
	  }

	if((s32) m_state.getTotalBitSize(sut) > maxsize)
	  maxsize = m_state.getTotalBitSize(sut); //includes arrays

	it++;
      }
    return maxsize;
  } //getMaxVariableSymbolsBitSize

  //#define OPTIMIZE_PACKED_BITS
#ifdef OPTIMIZE_PACKED_BITS
  //currently, packing is done by Nodes since the order of declaration is available;
  //but in case we may want to optimize the layout someday,
  //we keep this here since all the symbols are available in one place.
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
	    offsetIntoAtom += m_state.getTotalBitSize(sym->getUlamTypeIdx()); //times array size
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
	    if(UlamType::compare(sym->getUlamTypeIdx(), utype, m_state) == UTIC_SAME)
	      {
		posfound = ((SymbolVariable *) sym)->getPosOffset();
		break;
	      }
	  }
	it++;
      }
    return posfound;
  } //findPosOfUlamTypeInTable

  //replaced with parse tree method to preserve order of declaration
  void SymbolTable::genCodeForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(!sym->isTypedef() && sym->isDataMember()) //including model parameters
	  {
	    ((SymbolVariable *) sym)->generateCodedVariableDeclarations(fp, classtype);
	  }
	it++;
      }
  } //genCodeForTableOfVariableDataMembers (unused)

  void SymbolTable::genCodeBuiltInFunctionsOverTableOfVariableDataMember(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    //'has' applies to both quarks and elements
    UTI cuti = m_state.getCompileThisIdx();

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
      fp->write("template<class EC>\n");
    else if(classtype == UC_QUARK)
      fp->write("template<class EC, u32 POS>\n");
    else
      assert(0);

    m_state.indent(fp);
    fp->write("s32 "); //return pos offset, or -1 if not found

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    if(classtype == UC_ELEMENT)
      fp->write("<EC>");
    else if(classtype == UC_QUARK)
      fp->write("<EC, POS>");

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
		fp->write(sut->getUlamTypeMangledName().c_str()); //mangled, including class args!
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

  void SymbolTable::addModelParameterDescriptionsToMap(ParameterMap& classmodelparameters)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isModelParameter() && ((SymbolParameterValue *)sym)->isReady())
	  {
	    //similar to SymbolClass' addTargetDescriptionMapEntry for class targets
	    struct ParameterDesc desc;
	    desc.m_loc = sym->getLoc();
	    desc.m_mangledType = m_state.getUlamTypeByIndex(sym->getUlamTypeIdx())->getUlamTypeMangledName();
	    assert(((SymbolParameterValue *) sym)->getValue(desc.m_val)); //is ready.
	    desc.m_parameterName = m_state.m_pool.getDataAsString(sym->getId());
	    Token scTok;
	    if(((SymbolParameterValue *) sym)->getStructuredComment(scTok))
	      {
		std::ostringstream sc;
		sc << "/**";
		sc << m_state.m_pool.getDataAsString(scTok.m_dataindex).c_str();
		sc << "*/";
		desc.m_structuredComment = sc.str();
	      }
	    else
	      desc.m_structuredComment = "NONE";

	    std::string mangledName = sym->getMangledName();
	    classmodelparameters.insert(std::pair<std::string, struct ParameterDesc>(mangledName, desc));
	  }
	it++;
      }
  } //addModelParameterDescriptionsToMap

  //storage for class members persists, so we give up preserving
  //order of declaration that the NodeVarDecl in the parseTree
  //provides, in order to distinguish between an instance's data
  //members on the STACK verses the classes' data members in
  //EVENTWINDOW.
  void SymbolTable::printPostfixValuesForTableOfVariableDataMembers(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isTypedef() || sym->isConstant() || (sym->isDataMember() && !sym->isModelParameter()))
	  {
	    sym->printPostfixValuesOfVariableDeclarations(fp, slot, startpos, classtype);
	  }
	it++;
      }
  } //printPostfixValuesForTableOfVariableDataMembers

  //convert UTI to mangled strings to insure overload uniqueness
  bool SymbolTable::checkTableOfFunctions()
  {
    u32 probcnt = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    probcnt += ((SymbolFunctionName *) sym)->checkFunctionNames();
	  }
	it++;
      }
    return (probcnt > 0);
  } //checkTableOfFunctions

  void SymbolTable::linkToParentNodesAcrossTableOfFunctions(NodeBlockClass * p)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->linkToParentNodesInFunctionDefs(p);
	  }
	it++;
      }
  } //linkToParentNodesAcrossTableOfFunctions

  void SymbolTable::updatePrevBlockPtrAcrossTableOfFunctions(NodeBlockClass * p)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->updatePrevBlockPtrInFunctionDefs(p);
	  }
	it++;
      }
  } //updatePrevBlockPtrAcrossTableOfFunctions

  bool SymbolTable::findNodeNoAcrossTableOfFunctions(NNO n, Node*& foundNode)
  {
    bool rtnfound = false;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    if(((SymbolFunctionName *) sym)->findNodeNoInFunctionDefs(n, foundNode))
	      {
		rtnfound = true;
		break;
	      }
	  }
	it++;
      }
    return rtnfound;
  } //findNodeNoAcrossTableOfFunctions

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

  u32 SymbolTable::countNavNodesAcrossTableOfFunctions()
  {
    u32 totalNavCount = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    totalNavCount += ((SymbolFunctionName *) sym)->countNavNodesInFunctionDefs();
	  }
	it++;
      }
    return totalNavCount;
  } //countNavNodesAcrossTableOfFunctions

  void SymbolTable::calcMaxDepthForTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	((SymbolFunctionName *) sym)->calcMaxDepthOfFunctions();
	it++;
      }
    return;
  } //calcMaxDepthForTableOfFunctions

  //called by current Class block on its function ST
  bool SymbolTable::checkCustomArrayTypeFuncs()
  {
    bool rtnBool = false;
    NodeBlockClass * cblock = m_state.getClassBlock();

    Symbol * fnsymget = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsymget))
      {
	u32 probcount = 0;
	//LOOP over SymbolFunctions to verify same return type
	// CANT use UTI directly, must build string of keys to compare
	// as they may change.

	//class type should already be flagged as a custom array
	UTI cuti = cblock->getNodeType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	if(!((UlamTypeClass *) cut)->isCustomArray())
	  {
	    std::ostringstream msg;
	    msg << "Custom array get method '";
	    msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str();
	    msg << "' FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	    MSG(cblock->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    probcount++;
	  }
	else
	  {
	    UTI caType = Nav;
	    probcount = ((SymbolFunctionName *) fnsymget)->checkCustomArrayGetFunctions(caType); //sets caType

	    if(!probcount)
	      {
		// for each aset that exists: it has two params, the 2nd is
		// the same as the get return type, and the set return type is Void.
		Symbol * fnsymset = NULL;
		if(isInTable(m_state.getCustomArraySetFunctionNameId(), fnsymset))
		  {
		    probcount = ((SymbolFunctionName *) fnsymset)->checkCustomArraySetFunctions(caType);
		  }
	      }
	  }
	rtnBool = (probcount == 0);
      } //get found
    else
      {
	UTI cuti = m_state.getCompileThisIdx();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	if(((UlamTypeClass *) cut)->isCustomArray())
	  {
	    std::ostringstream msg;
	    msg << "Custom array get method '";
	    msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str();
	    msg << "' NOT FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	    MSG(cblock->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    return rtnBool;
  } //checkCustomArrayTypeFuncs

  //called by current Class block on its function ST
  UTI SymbolTable::getCustomArrayReturnTypeGetFunction()
  {
    UTI caType = Nav;
    Symbol * fnsym = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsym))
      {
	caType = ((SymbolFunctionName *) fnsym)->getCustomArrayReturnType();
      }
    return caType;
  } //getCustomArrayReturnTypeGetFunction

  u32 SymbolTable::countNativeFuncDeclsForTableOfFunctions()
  {
    u32 nativeCount = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
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

  void SymbolTable::getTargets(TargetMap& classtargets)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->getTargetDescriptorsForClassInstances(classtargets);
	  }
	it++;
      } //while
  } //getTargets

  void SymbolTable::getModelParameters(ParameterMap& classmodelparameters)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->getModelParameterDescriptionsForClassInstances(classmodelparameters);
	  }
	it++;
      } //while
  } //getModelParameters

  void SymbolTable::testForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    m_state.m_err.clearCounts();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->testForClassInstances(fp);
	  }
	it++;
      } //while

    //output informational warning and error counts
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
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(sym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	    classNode->printPostfix(fp);
	    m_state.popClassContext(); //restore
	  }
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
	m_state.pushClassContext(sym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	classNode->print(fp);
	m_state.popClassContext(); //restore
	it++;
      } //while
  } //printForDebugForTableOfClasses

  bool SymbolTable::statusNonreadyClassArgumentsInTableOfClasses()
  {
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	if(((SymbolClassName *) sym)->isClassTemplate())
	  aok &= ((SymbolClassNameTemplate *) sym)->statusNonreadyClassArgumentsInStubClassInstances();
	it++;
      } //while
    return aok;
  } //statusNonreadyClassArgumentsInTableOfClasses

  bool SymbolTable::fullyInstantiateTableOfClasses()
  {
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	if(((SymbolClassName *) sym)->isClassTemplate())
	  aok &= ((SymbolClassNameTemplate *) sym)->fullyInstantiate();
	it++;
      } //while
    return aok;
  } //fullyInstantiateTableOfClasses

  //done after cloning and before checkandlabel;
  //blocks without prevblocks set, are linked to prev block;
  //used for searching for missing symbols in STs during c&l.
  void SymbolTable::updateLineageForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->updateLineageOfClass();
	    //only regular and templates immediate after updating lineages
	    ((SymbolClassName *) sym)->checkAndLabelClassFirst();
	  }
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

	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->checkCustomArraysOfClassInstances();
	  }
	it++;
      }
  } //checkCustomArraysForTableOfClasses()

  void SymbolTable::checkDuplicateFunctionsForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->checkDuplicateFunctionsForClassInstances();
	  }
	it++;
      }
  } //checkDuplicateFunctionsForTableOfClasses

  void SymbolTable::calcMaxDepthOfFunctionsForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->calcMaxDepthOfFunctionsForClassInstances();
	  }
	it++;
      }
  } //calcMaxDepthOfFunctionsForTableOfClasses

  bool SymbolTable::labelTableOfClasses()
  {
    m_state.clearGoAgain();

    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	SymbolClassName * cnsym = (SymbolClassName *) (it->second);
	assert(cnsym->isClass());
	UTI cuti = cnsym->getUlamTypeIdx();
	if( ((SymbolClass *) cnsym)->getUlamClass() == UC_UNSEEN)
	  {
	    //skip anonymous classes
	    if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	      {
		std::ostringstream msg;
		msg << "Unresolved type <";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		msg << "> was never defined; Fails labeling";
		MSG(cnsym->getTokPtr(), msg.str().c_str(), ERR);
		//assert(0); wasn't a class at all, e.g. out-of-scope typedef/variable
		break;
	      }
	  }
	else
	  cnsym->checkAndLabelClassInstances();

	it++;
      }
    return (!m_state.goAgain() && (m_state.m_err.getErrorCount() + m_state.m_err.getWarningCount() == 0));
  } //labelTableOfClasses

  u32 SymbolTable::countNavNodesAcrossTableOfClasses()
  {
    u32 navcount = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    navcount += ((SymbolClassName *) sym)->countNavNodesInClassInstances();
	  }
	it++;
      }
    return navcount;
  } //countNavNodesAcrossTableOfClasses

  //separate pass...after labeling all classes is completed;
  //purpose is to set the size of all the classes, by totalling the size
  //of their data members; returns true if all class sizes complete.
  bool SymbolTable::setBitSizeOfTableOfClasses()
  {
    std::vector<u32> lostClassesIds;
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	bool isAnonymousClass = m_state.getUlamTypeByIndex(cuti)->isHolder() || !m_state.isARootUTI(cuti);
	ULAMCLASSTYPE classtype = ((SymbolClass *) sym)->getUlamClass();
	if( classtype == UC_UNSEEN)
	  {
	    std::ostringstream msg;
	    msg << "Unresolved type <";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << "> was never defined; Fails sizing";
	    if(isAnonymousClass)
	      MSG(sym->getTokPtr(), msg.str().c_str(), DEBUG);
	    else
	      MSG(sym->getTokPtr(), msg.str().c_str(), ERR);
	    //m_state.completeIncompleteClassSymbol(sym->getUlamTypeIdx()); //too late
	    aok = false; //moved here;
	  }
	else
	  {
	    //skip unseen and anonymous classes, o.w. try..
	    aok = ((SymbolClassName *) sym)->setBitSizeOfClassInstances();
	  }

	//track classes that fail to be sized.
	if(!aok)
	  lostClassesIds.push_back(sym->getId());

	aok = true; //reset for next class
	it++;
      } //next class

    aok = lostClassesIds.empty();
    if(!aok)
      {
	std::ostringstream msg;
	msg << lostClassesIds.size() << " Classes without sizes";
	while(!lostClassesIds.empty())
	  {
	    u32 cid = lostClassesIds.back();
	    msg << ", " << m_state.m_pool.getDataAsString(cid).c_str();
	    lostClassesIds.pop_back();
	  }
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
      }
    lostClassesIds.clear();
    return aok;
  } //setBitSizeOfTableOfClasses

  //separate pass...after labeling all classes is completed;
  void SymbolTable::printBitSizeOfTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->printBitSizeOfClassInstances();
	  }
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

	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    //quark union keep default pos = 0 for each data member, hence skip packing bits.
	    if(!((SymbolClass *) sym)->isQuarkUnion())
	      {
		((SymbolClassName *) sym)->packBitsForClassInstances();
	      }
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
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->generateIncludesForClassInstances(fp);
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
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    ((SymbolClassName *) sym)->generateForwardDefsForClassInstances(fp);
	  }
	it++;
      }
  } //generateForwardDefsForTableOfClasses

  enum { NORUNTEST = 0, RUNTEST = 1  };

  //test for the current compileThisIdx, with test method
  void SymbolTable::generateTestInstancesForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {
	    //first output all the element typedefs, skipping quarks
	    if(((SymbolClass * ) sym)->getUlamClass() != UC_QUARK)
	      ((SymbolClassName *) sym)->generateTestInstanceForClassInstances(fp, NORUNTEST);
	  }
	it++;
      } //while for typedefs only

    fp->write("\n");
    m_state.indent(fp);
    fp->write("OurAtomAll atom;\n");
    m_state.indent(fp);
    fp->write("MFM::Ui_Ut_102321i rtn;\n");

    it = m_idToSymbolPtr.begin();
    s32 idcounter = 1;
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	//next output all the element typedefs that are m_compileThisId; skipping quarks
	if(sym->getId() == m_state.getCompileThisId() && ((SymbolClass * ) sym)->getUlamClass() != UC_QUARK)
	  ((SymbolClassName *) sym)->generateTestInstanceForClassInstances(fp, RUNTEST);
	it++;
	idcounter++;
      } //while to run this test

    fp->write("\n");
    m_state.indent(fp);
    fp->write("return 0;\n");
  } //generateTestInstancesForTableOfClasses

  void SymbolTable::genCodeForTableOfClasses(FileManager * fm)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(m_state.isARootUTI(cuti) && !m_state.getUlamTypeByIndex(cuti)->isHolder())
	  {

	    //output header/body for this class next
	    ((SymbolClassName *) sym)->generateCodeForClassInstances(fm);
	  }
	it++;
      } //while
  } //genCodeForTableOfClasses

  //PRIVATE HELPER METHODS:
  s32 SymbolTable::calcVariableSymbolTypeSize(UTI argut)
  {
    s32 totbitsize = m_state.getBitSize(argut);

    if(m_state.getUlamTypeByIndex(argut)->getUlamClass() == UC_NOTACLASS) //includes Atom type
      {
	return totbitsize; //arrays handled by caller, just bits here
      }

    //not a primitive (class), array
    if(m_state.getArraySize(argut) > 0)
      {
	if(totbitsize >= 0)
	  {
	    return totbitsize;
	  }
	if(totbitsize == CYCLEFLAG) //was < 0
	  {
	    assert(0);
	    return CYCLEFLAG;
	  }
	if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0; //empty, ok
	  }
	else
	  {
	    assert(totbitsize <= UNKNOWNSIZE || m_state.getArraySize(argut) == UNKNOWNSIZE);

	    m_state.setBitSize(argut, CYCLEFLAG); //before the recusive call..

	    //get base type, scalar type of class
	    SymbolClass * csym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(argut, csym))
	      {
		return calcVariableSymbolTypeSize(csym->getUlamTypeIdx()); //NEEDS CORRECTION
	      }
	  }
      }
    else //not primitive type (class), and not array (scalar)
      {
	if(totbitsize >= 0)
	  {
	    return totbitsize;
	  }

	if(totbitsize == CYCLEFLAG) //was < 0
	  {
	    return CYCLEFLAG; //error! cycle
	  }
	else if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0; //empty, ok
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
		else if(csize == CYCLEFLAG)  //was < 0
		  {
		    //error! cycle..replace with message
		    return csize;
		  }
		else if(csize == EMPTYSYMBOLTABLE)
		  {
		    return 0; //empty, ok
		  }
		else if(csym->isStub())
		  {
		    return UNKNOWNSIZE; //csize?
		  }
		else
		  {
		    //==0, redo variable total
		    NodeBlockClass * classblock = csym->getClassBlockNode();
		    assert(classblock);

		    //quark cannot contain a copy of itself!
		    if(classblock == m_state.getClassBlock())
		      {
			UTI suti = csym->getUlamTypeIdx();
			std::ostringstream msg;
			msg << "Quark/Element '" << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
			msg << "' cannot contain a copy of itself";
			MSG(csym->getTokPtr(), msg.str().c_str(), ERR);
			return UNKNOWNSIZE;
		      }

		    m_state.pushClassContext(csym->getUlamTypeIdx(), classblock, classblock, false, NULL);

		    if(csym->isQuarkUnion())
		      csize = classblock->getMaxBitSizeOfVariableSymbolsInTable();
		    else
		      csize = classblock->getBitSizesOfVariableSymbolsInTable(); //data members only

		    m_state.popClassContext(); //restore
		    return csize;
		  }
	      }
	  } //totbitsize == 0
      } //not primitive, not array
    return UNKNOWNSIZE; //was CYCLEFLAG
  } //calcVariableSymbolTypeSize (recursively)

  bool SymbolTable::variableSymbolWithCountableSize(Symbol * sym)
  {
    return (!sym->isTypedef() && !sym->isModelParameter() && !sym->isConstant());
  }

} //end MFM
