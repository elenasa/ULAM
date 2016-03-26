#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTable.h"
#include "SymbolFunctionName.h"
#include "SymbolParameterValue.h"
#include "SymbolVariable.h"
#include "SymbolVariableDataMember.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "NodeBlock.h"
#include "Node.h"
#include "MapParameterDesc.h"
#include "MapDataMemberDesc.h"
#include "MapConstantDesc.h"
#include "MapTypedefDesc.h"

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
    std::pair<std::map<u32, Symbol *>::iterator, bool> reti;
    reti = m_idToSymbolPtr.insert(std::pair<u32,Symbol*> (id,sptr));
    assert(reti.second); //false if already existed, i.e. not added (leak?)
  }

  void SymbolTable::replaceInTable(u32 oldid, u32 newid, Symbol * s)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(oldid);
    if(it != m_idToSymbolPtr.end())
      {
	Symbol * oldsym = it->second;
	assert(oldsym == s);
	m_idToSymbolPtr.erase(it);
      }
    addToTable(newid, s);
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

  //called by NodeBlockClass.
  u32 SymbolTable::getNumberOfConstantSymbolsInTable(bool argsOnly)
  {
    u32 cntOfConstants = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym);
	if(sym->isConstant())
	  {
	    if(!argsOnly || ((SymbolConstantValue *) sym)->isArgument())
	      cntOfConstants++;
	  }
	it++;
      }
    return cntOfConstants;
  } //getNumberOfConstantSymbolsInTable

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
	UTI suti = sym->getUlamTypeIdx();
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym) && !m_state.isClassAQuarkUnion(suti))
	  {
	    //updates the offset with the bit size of sym
	    ((SymbolVariable *) sym)->setPosOffset(offsetIntoAtom);
	    offsetIntoAtom += m_state.getTotalBitSize(suti); //times array size
	  }
	it++;
      }
  }
#endif

  s32 SymbolTable::findPosOfUlamTypeInTable(UTI utype, UTI& insidecuti)
  {
    s32 posfound = -1;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym))
	  {
	    UTI suti = sym->getUlamTypeIdx();
	    if(UlamType::compare(suti, utype, m_state) == UTIC_SAME)
	      {
		posfound = ((SymbolVariable *) sym)->getPosOffset();
		insidecuti = suti;
		break;
	      }
	    else
	      {
		// check possible inheritance
		UTI superuti = m_state.isClassASubclass(suti);
		assert(superuti != Hzy);
		if((superuti != Nouti) && (UlamType::compare(superuti, utype, m_state) == UTIC_SAME))
		  {
		    posfound = ((SymbolVariable *) sym)->getPosOffset(); //starts at beginning
		    insidecuti = suti;
		    break;
		  }
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

  void SymbolTable::genModelParameterImmediateDefinitionsForTableOfVariableDataMembers(File *fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isModelParameter())
	  {
	    UTI suti = sym->getUlamTypeIdx();
	    m_state.getUlamTypeByIndex(suti)->genUlamTypeMangledImmediateModelParameterDefinitionForC(fp);
	  }
	it++;
      }
  } //genModelParameterImmediateDefinitionsForTableOfVariableDataMembers

  void SymbolTable::genCodeBuiltInFunctionHasOverTableOfVariableDataMember(File * fp)
  {
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
		fp->write("\")) return (");
		fp->write_decimal(((SymbolVariable *) sym)->getPosOffset());
		fp->write("); //pos offset\n");

		UTI superuti = m_state.isClassASubclass(suti);
		assert(superuti != Hzy);
		while(superuti != Nouti) //none
		  {
		    UlamType * superut = m_state.getUlamTypeByIndex(superuti);
		    m_state.indent(fp);
		    fp->write("if(!strcmp(namearg,\"");
		    fp->write(superut->getUlamTypeMangledName().c_str()); //mangled, including class args!
		    fp->write("\")) return (");
		    fp->write_decimal(((SymbolVariable *) sym)->getPosOffset()); //same offset starts at 0
		    fp->write("); //inherited pos offset\n");

		    superuti = m_state.isClassASubclass(superuti); //any more
		  } //while
	      }
	  }
	it++;
      }
  } //genCodeBuiltInFunctionHasOverTableOfVariableDataMember

  void SymbolTable::genCodeBuiltInFunctionBuildDefaultsOverTableOfVariableDataMember(File * fp, UTI cuti)
  {
    bool useFullClassName = (cuti != m_state.getCompileThisIdx()); //from its superclass

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
		if(m_state.getBitSize(suti) > 0)
		  {
		    if(m_state.isScalar(suti))
		      {
			SymbolClass * csym = NULL;
			AssertBool isDefined = m_state.alreadyDefinedSymbolClass(suti, csym);
			assert(isDefined);

			u32 qval = 0;
			AssertBool isDefinedQuark = csym->getDefaultQuark(qval);
			assert(isDefinedQuark);

			std::ostringstream qdhex;
			qdhex << "0x" << std::hex << qval;

			m_state.indent(fp);
			fp->write("UlamRef<EC>("); //open wrapper
			fp->write_decimal_unsigned(sym->getPosOffset()); //rel offset
			fp->write(", ");
			fp->write_decimal_unsigned(sut->getBitSize()); //len
			fp->write(", da, &");
			fp->write(m_state.getEffectiveSelfMangledNameByIndex(suti).c_str());
			fp->write(")."); //close wrapper
			fp->write(sut->writeMethodForCodeGen().c_str());
			fp->write("(");
			fp->write(qdhex.str().c_str());
			fp->write("); //"); //include var name in a comment
			fp->write(m_state.m_pool.getDataAsString(sym->getId()).c_str());
			fp->write("=");
			fp->write_decimal_unsigned(qval);
			fp->write("\n");
		      }
		    else
		      {
			//an array of quarks, OR reference to an array of quarks
			// first, get default value of its scalar quark
			UTI scalaruti = m_state.getUlamTypeAsScalar(suti);
			scalaruti = m_state.getUlamTypeAsDeref(scalaruti); //not ref of.
			UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
			SymbolClass * csym = NULL;
			AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalaruti, csym);
			assert(isDefined);

			u32 qval = 0;
			AssertBool isDefinedQuark = csym->getDefaultQuark(qval);
			assert(isDefinedQuark);

			std::ostringstream qdhex;
			qdhex << "0x" << std::hex << qval;

			//initialize each array item
			u32 arraysize = sut->getArraySize();
			u32 itemlen = sut->getBitSize();
			for(u32 j = 0; j < arraysize; j++)
			  {
			    m_state.indent(fp);
			    fp->write("UlamRef<EC>(");
			    fp->write_decimal_unsigned(sym->getPosOffset()); //rel offset
			    fp->write("u + ");
			    fp->write_decimal_unsigned(j * itemlen); //rel offset
			    fp->write("u, ");
			    fp->write_decimal_unsigned(itemlen); //len
			    fp->write("u, da, &");
			    fp->write(m_state.getEffectiveSelfMangledNameByIndex(scalaruti).c_str());
			    fp->write(").");
			    fp->write(scalarut->writeMethodForCodeGen().c_str());
			    fp->write("(");
			    fp->write(qdhex.str().c_str());
			    fp->write("); //"); //include var name in a comment
			    fp->write(m_state.m_pool.getDataAsString(sym->getId()).c_str());
			    fp->write("[");
			    fp->write_decimal((s32) j);
			    fp->write("]");
			    fp->write("=");
			    fp->write_decimal_unsigned(qval);
			    fp->write("\n");
			  }
		      } //array of quarks
		  } //countable size
	      } //quark
	    else if(((SymbolVariableDataMember*)sym)->hasInitValue())
	      {
		//neither typedef, nor model parameter, nor named constant
		// but does this data member have an initialization value?
		// o.w. zero
		u64 val = 0;
		AssertBool isDefined = ((SymbolVariableDataMember*)sym)->getInitValue(val);
		assert(isDefined);

		m_state.indent(fp);
		if(useFullClassName)
		  {
		    fp->write("typename ");
		    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
		    fp->write("<EC>::");
		  }
		fp->write(sym->getMangledNameForParameterType().c_str());
		fp->write("(da, &");
		if(useFullClassName)
		  {
		    fp->write(m_state.getUlamTypeByIndex(m_state.getCompileThisIdx())->getUlamTypeMangledName().c_str()); //effself
		    fp->write("<EC>::");
		  }
		fp->write("THE_INSTANCE).");
		fp->write(sut->writeMethodForCodeGen().c_str());
		fp->write("(");
		fp->write_decimal_unsignedlong(val);
		fp->write("u);\n");
	      }
	  }
	it++;
      } //end while
  } //genCodeBuiltInFunctionBuildDefaultsOverTableOfVariableDataMember

  void SymbolTable::addClassMemberDescriptionsToMap(UTI classType, ClassMemberMap& classmembers)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	ClassMemberDesc * descptr = NULL;
	Symbol * sym = it->second;
	if(sym->isModelParameter() && ((SymbolParameterValue *)sym)->isReady())
	  {
	    descptr = new ParameterDesc((SymbolParameterValue *) sym, classType, m_state);
	    assert(descptr);
	  }
	else if(sym->isDataMember())
	  {
	    descptr = new DataMemberDesc((SymbolVariableDataMember *) sym, classType, m_state);
	    assert(descptr);
	  }
	else if(sym->isTypedef())
	  {
	    descptr = new TypedefDesc((SymbolTypedef *) sym, classType, m_state);
	    assert(descptr);
	  }
	else if(sym->isConstant() && ((SymbolConstantValue *)sym)->isReady())
	  {
	    descptr = new ConstantDesc((SymbolConstantValue *) sym, classType, m_state);
	    assert(descptr);
	  }
	else
	  {
	    //error not ready perhaps
	    assert(0); //(functions done separately)
	  }

	if(descptr)
	  {
	    //concat mangled class and parameter names to avoid duplicate keys into map
	    std::ostringstream fullMangledName;
	    fullMangledName << descptr->m_mangledClassName << "_" << descptr->m_mangledMemberName;
	    classmembers.insert(std::pair<std::string, ClassMemberDescHolder>(fullMangledName.str(), ClassMemberDescHolder(descptr)));
	  }
	it++;
      }
  } //addClassMemberDescriptionsToMap

  void SymbolTable::addClassMemberFunctionDescriptionsToMap(UTI classType, ClassMemberMap& classmembers)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());

	((SymbolFunctionName *) sym)->addFunctionDescriptionsToClassMemberMap(classType, classmembers);
	it++;
      }
  } //addClassMemberFunctionDescriptionsToMap

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
	if(sym->isTypedef() || sym->isConstant() || sym->isModelParameter() || sym->isDataMember())
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

  void SymbolTable::printUnresolvedLocalVariablesForTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isFunction());
	((SymbolFunctionName *) sym)->printUnresolvedLocalVariablesInFunctionDefs();
	it++;
      }
  } //printUnresolvedVariablesForTableOfClasses

  void SymbolTable::countNavNodesAcrossTableOfFunctions(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->countNavNodesInFunctionDefs(ncnt, hcnt, nocnt);
	  }
	it++;
      }
    return;
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

  void SymbolTable::calcMaxIndexForVirtualTableOfFunctions(s32& maxidx)
  {
    UTI cuti = m_state.getCompileThisIdx();
    UTI superuti = m_state.isClassASubclass(cuti);

    if(m_idToSymbolPtr.empty() && (superuti == Nouti))
      {
	assert(maxidx <= 0);
    	maxidx = 0; //use zero when empty
      }

    //initialize this classes VTable to super classes' VTable, or empty
    // some entries may be modified; or table may expand
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    csym->initVTable(maxidx);

    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	((SymbolFunctionName *) sym)->calcMaxIndexOfVirtualFunctions(maxidx);
	it++;
      }
    return;
  } //calcMaxIndexForVirtualTableOfFunctions

  void SymbolTable::checkAbstractInstanceErrorsAcrossTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	((SymbolFunctionName *) sym)->checkAbstractInstanceErrorsInFunctions();
	it++;
      }
    return;
  } //checkAbstractInstanceErrorsAcrossTableOfFunctions

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
	if(!m_state.isClassACustomArray(cuti))
	  {
	    std::ostringstream msg;
	    msg << "Custom array get method '";
	    msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str();
	    msg << "' FOUND in class: " << m_state.getUlamTypeByIndex(cuti)->getUlamTypeNameOnly().c_str();
	    MSG(cblock->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    probcount++;
	  }
	else
	  {
	    UTI caType = Nouti;
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
    UTI caType = Nouti;
    Symbol * fnsym = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsym))
      {
	caType = ((SymbolFunctionName *) fnsym)->getCustomArrayReturnType();
      }
    return caType;
  } //getCustomArrayReturnTypeGetFunction

  //called by current Class block on its function ST; need to find
  // a "safe" match for rt.
  u32 SymbolTable::getCustomArrayIndexTypeGetFunction(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    u32 camatches = 0;
    Symbol * fnsym = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsym))
      {
	camatches = ((SymbolFunctionName *) fnsym)->getCustomArrayIndexTypeFor(rnode, idxuti, hasHazyArgs);
      }
    return camatches;
  } //getCustomArrayIndexTypeGetFunction

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
	if(!isAnonymousClass(cuti))
	  {
	    ((SymbolClassName *) sym)->getTargetDescriptorsForClassInstances(classtargets);
	  }
	it++;
      } //while
  } //getTargets

  void SymbolTable::getClassMembers(ClassMemberMap& classmembers)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    ((SymbolClassName *) sym)->getClassMemberDescriptionsForClassInstances(classmembers);
	  }
	it++;
      } //while
  } //getClassMembers

  void SymbolTable::initializeElementDefaultsForEval(UlamValue& uvsite)
  {
    if(m_idToSymbolPtr.empty()) return;

    std::map<u32, Symbol* >::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	UTI suti = sym->getUlamTypeIdx();
	//skip quarkunion initializations
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym) && !m_state.isClassAQuarkUnion(suti))
	  {
	    s32 bitsize = m_state.getBitSize(suti);
	    u32 pos = ((SymbolVariableDataMember *) sym)->getPosOffset();

	    //updates the UV at offset with the default of sym; non-class arrays have none
	    if(((SymbolVariableDataMember *) sym)->hasInitValue())
	      {
		u64 dval = 0;
		if(((SymbolVariableDataMember *) sym)->getInitValue(dval))
		  {
		    u32 wordsize = m_state.getTotalWordSize(suti);
		    if(wordsize <= MAXBITSPERINT)
		      uvsite.putData(pos + ATOMFIRSTSTATEBITPOS, bitsize, (u32) dval);
		    else if(wordsize <= MAXBITSPERLONG)
		      uvsite.putDataLong(pos + ATOMFIRSTSTATEBITPOS, bitsize, dval);
		    else
		      assert(0);
		  }
	      }
	    else if(m_state.getUlamTypeByIndex(suti)->getUlamClass() == UC_QUARK)
	      {
		UTI scalarquark = m_state.getUlamTypeAsScalar(suti);
		SymbolClass * csym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalarquark, csym);
		assert(isDefined);
		u32 dval = 0;
		if(csym->getDefaultQuark(dval))
		  {
		    //could be an array of quarks
		    s32 arraysize = m_state.getArraySize(suti);
		    arraysize = (arraysize == NONARRAYSIZE ? 1 : arraysize);
		    for(s32 i = 0; i < arraysize; i++)
		      uvsite.putData(pos + ATOMFIRSTSTATEBITPOS + i * bitsize, bitsize, dval);
		  }
	      }
	  }
	it++;
      } //while
    return;
  } //initializeElementDefaultsForEval

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
	if(!isAnonymousClass(cuti))
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

  void SymbolTable::buildDefaultQuarksFromTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    ULAMCLASSTYPE classtype = cut->getUlamClass();
	    if( classtype == UC_QUARK)
	      ((SymbolClassName *) sym)->buildDefaultQuarkForClassInstances(); //builds when not ready; must be qk
	    //else if (classtype == UC_ELEMENT)
	    //  assert(0);
	  }
	it++;
      } //while
  } //buildDefaultQuarksFromTableOfClasses

  void SymbolTable::printPostfixForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

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

  // adds unknown type names as incomplete classes if still "hzy" after parsing done
  bool SymbolTable::checkForUnknownTypeNamesInTableOfClasses()
  {
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI suti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(suti))
	  {
	    aok &= ((SymbolClassName *) sym)->statusUnknownTypeNamesInClassInstances();
	  }
	it++;
      } //while
    return aok;
  } //checkForUnknownTypeNamesInTableOfClasses

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
	if(!isAnonymousClass(cuti))
	  {
	    ((SymbolClassName *) sym)->updateLineageOfClass();
	    //only regular and templates immediately after updating lineages
	    // not just for efficiency; helps resolves types
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
	if(!isAnonymousClass(cuti))
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
	if(!isAnonymousClass(cuti))
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
	if(!isAnonymousClass(cuti))
	  {
	    ((SymbolClassName *) sym)->calcMaxDepthOfFunctionsForClassInstances();
	  }
	it++;
      }
  } //calcMaxDepthOfFunctionsForTableOfClasses

  bool SymbolTable::calcMaxIndexOfVirtualFunctionsForTableOfClasses()
  {
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    aok &= ((SymbolClassName *) sym)->calcMaxIndexOfVirtualFunctionsForClassInstances();
	  }
	it++;
      }
    return aok;
  } //calcMaxIndexOfVirtualFunctionsForTableOfClasses

  void SymbolTable::checkAbstractInstanceErrorsForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    ((SymbolClassName *) sym)->checkAbstractInstanceErrorsForClassInstances();
	  }
	it++;
      }
  } //checkAbstractInstanceErrorsForTableOfClasses

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
	    if(!isAnonymousClass(cuti))
	      {
		std::ostringstream msg;
		msg << "Unresolved type <";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		msg << "> was never defined; Fails labeling";
		MSG(cnsym->getTokPtr(), msg.str().c_str(), WARN); //was ERR but typedef junk
		cnsym->getClassBlockNode()->setNodeType(Nav); //for compiler counter
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

  void SymbolTable::countNavNodesAcrossTableOfClasses(u32& navcount, u32& hzycount, u32& unsetcount)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    ((SymbolClassName *) sym)->countNavNodesInClassInstances(navcount, hzycount, unsetcount);
	  }
	it++;
      }
    return;
  } //countNavNodesAcrossTableOfClasses

  u32 SymbolTable::reportUnknownTypeNamesAcrossTableOfClasses()
  {
    u32 totalcnt = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    totalcnt += ((SymbolClassName *) sym)->reportUnknownTypeNamesInClassInstances();
	  }
	it++;
      }
    return totalcnt;
  } //reportUnknownTypeNamesAcrossTableOfClasses

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
	bool anonymousClass = isAnonymousClass(cuti);
	ULAMCLASSTYPE classtype = ((SymbolClass *) sym)->getUlamClass();
	if((classtype == UC_UNSEEN) || anonymousClass)
	  {
	    std::ostringstream msg;
	    msg << "Unresolved type <";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << "> was never defined; Fails sizing";
	    if(anonymousClass)
	      MSG(sym->getTokPtr(), msg.str().c_str(), DEBUG);
	    else
	      MSG(sym->getTokPtr(), msg.str().c_str(), DEBUG); //was ERR but typedef junk; also catch at use!
	    //m_state.completeIncompleteClassSymbol(sym->getUlamTypeIdx()); //too late
	    aok = false; //moved here;
	  }
	else
	  {
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
	if(!isAnonymousClass(cuti))
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
	if(!isAnonymousClass(cuti))
	  {
	    //quark union keep default pos = 0 for each data member, hence skip packing bits.
	    if(!(m_state.isClassAQuarkUnion(cuti)))
	      {
		((SymbolClassName *) sym)->packBitsForClassInstances();
	      }
	  }
	it++;
      }
  } //packBitsForTableOfClasses

  void SymbolTable::printUnresolvedVariablesForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    ((SymbolClassName *) sym)->printUnresolvedVariablesForClassInstances();
	  }
	it++;
      }
  } //printUnresolvedVariablesForTableOfClasses

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
	if(!isAnonymousClass(cuti))
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
	if(!isAnonymousClass(cuti))
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
	if(!isAnonymousClass(cuti))
	  {
	    //first output all the element typedefs, skipping quarks
	    if(((SymbolClass * ) sym)->getUlamClass() != UC_QUARK)
	      ((SymbolClassName *) sym)->generateTestInstanceForClassInstances(fp, NORUNTEST);
	  }
	it++;
      } //while for typedefs only

    fp->write("\n");

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
	if(!isAnonymousClass(cuti))
	  {
	    //output header/body for this class next
	    ((SymbolClassName *) sym)->generateCodeForClassInstances(fm);
	  }
	it++;
      } //while
  } //genCodeForTableOfClasses

  UTI SymbolTable::findClassNodeNoForTableOfClasses(NNO n)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	UTI cuti = sym->getUlamTypeIdx();
	//skip anonymous classes
	if(!isAnonymousClass(cuti))
	  {
	    NodeBlockClass * classblock = ((SymbolClassName *) sym)->getClassBlockNode();
	    if(classblock->getNodeNo() == n)
	      return sym->getUlamTypeIdx(); //found it!!
	  }
	it++;
      } //while
    return Nav;
  } //findClassNodeNoForTableOfClasses

  //PRIVATE HELPER METHODS:
  s32 SymbolTable::calcVariableSymbolTypeSize(UTI argut)
  {
    if(!m_state.okUTItoContinue(argut))
      {
	assert(argut != Nav);
	if(argut == Nouti)
	  return UNKNOWNSIZE;
	//else continue if Hzy
      }

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
    else if(m_state.isScalar(argut)) //not primitive type (class), and not array (scalar)
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
		UTI cuti = csym->getUlamTypeIdx();
		if((csize = m_state.getBitSize(cuti)) >= 0)
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
		else if(isAnonymousClass(cuti))
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
			classblock->setNodeType(Nav);
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
    // may be a zero-sized quark!!
    return (!sym->isTypedef() && !sym->isModelParameter() && !sym->isConstant());
  }

  bool SymbolTable::isAnonymousClass(UTI cuti)
  {
    assert(m_state.okUTItoContinue(cuti));
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    return(!m_state.isARootUTI(cuti) || cut->isHolder());
  }

} //end MFM
