#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTableOfVariables.h"
#include "SymbolModelParameterValue.h"
#include "SymbolVariable.h"
#include "SymbolVariableDataMember.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
//#include "MapDataMemberDesc.h"
//#include "MapParameterDesc.h"
//#include "MapConstantDesc.h"
//#include "MapTypedefDesc.h"

namespace MFM {

  SymbolTableOfVariables::SymbolTableOfVariables(CompilerState& state): SymbolTable(state) { }

  SymbolTableOfVariables::SymbolTableOfVariables(const SymbolTableOfVariables& ref) : SymbolTable(ref) { }

  SymbolTableOfVariables::~SymbolTableOfVariables() { }

  //called by NodeBlockClass.
  u32 SymbolTableOfVariables::getNumberOfConstantSymbolsInTable(bool argsOnly)
  {
    u32 cntOfConstants = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym);
	if(sym->isConstant())
	  {
	    if(!argsOnly || ((SymbolConstantValue *) sym)->isClassArgument())
	      cntOfConstants++;
	  }
	it++;
      }
    return cntOfConstants;
  } //getNumberOfConstantSymbolsInTable

  //called by NodeBlockContext (e.g. String (scalar or array))
  bool SymbolTableOfVariables::hasUlamTypeSymbolsInTable(ULAMTYPE etyparg)
  {
    bool rtnb = false;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym);

	//skip constantdefs (e.g. class args), typedefs, modelparameters
	if(variableSymbolWithCountableSize(sym)) //t3948, t3959
	  {
	    UTI suti = sym->getUlamTypeIdx();
	    ULAMTYPE etyp = m_state.getUlamTypeByIndex(suti)->getUlamTypeEnum();
	    if(etyp == etyparg)
	      {
		rtnb = true;
		break;
	      }
	  }
	it++;
      }
    return rtnb;
  } //hasUlamTypeSymbolsInTable

  u32 SymbolTableOfVariables::findTypedefSymbolNameIdByTypeInTable(UTI type)
  {
    u32 rtnId = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym);
	if(sym->isTypedef())
	  {
	    if(sym->getUlamTypeIdx() == type) //compare?
	      {
		rtnId = sym->getId();
		break;
	      }
	  }
	it++;
      }
    return rtnId;
  } //findTypedefSymbolNameIdByTypeInTable

  //called by NodeBlock.
  u32 SymbolTableOfVariables::getTotalSymbolSize()
  {
    u32 totalsizes = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	//typedefs don't contribute to the total bit size
	if(variableSymbolWithCountableSize(sym))
	  {
	    totalsizes += m_state.slotsNeeded(sym->getUlamTypeIdx());
	  }
	it++;
      }
    return totalsizes;
  } //getTotalSymbolSize

  s32 SymbolTableOfVariables::getTotalVariableSymbolsBitSize()
  {
    UTI cuti = m_state.getCompileThisIdx();
    ULAMCLASSTYPE cclasstype = m_state.getUlamTypeByIndex(cuti)->getUlamClassType();

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
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(sym->getTokPtr(), msg.str().c_str(), DEBUG);
	    totalsizes = UNKNOWNSIZE;
	    break;
	  }
	else
	  m_state.setBitSize(suti, symsize); //symsize does not include arrays

	UlamType * sut = m_state.getUlamTypeByIndex(suti); //no sooner!
	s32 arraysize = sut->getArraySize();
	arraysize = ((arraysize == NONARRAYSIZE) ? 1 : arraysize); //Mon Jul  4 14:14:44 2016
	if(arraysize == UNKNOWNSIZE)
	  {
	    totalsizes = UNKNOWNSIZE; //Thu Feb  8 16:06:18 2018 (t3504)
	  }
	else if((cclasstype == UC_TRANSIENT) && (sut->getUlamClassType() == UC_ELEMENT))
	  {
	    totalsizes += (BITSPERATOM * arraysize); //atom-based element regardless of its bitsize(t3879)
	  }
	else
	  totalsizes += sut->getTotalBitSize(); //covers up any unknown sizes; includes arrays (t3504)

	it++;
      } //while
    return totalsizes;
  } //getTotalVariableSymbolsBitSize

  s32 SymbolTableOfVariables::getMaxVariableSymbolsBitSize()
  {
    UTI cuti = m_state.getCompileThisIdx();
    ULAMCLASSTYPE cclasstype = m_state.getUlamTypeByIndex(cuti)->getUlamClassType();
    assert(cclasstype == UC_QUARK);

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

	UTI suti = sym->getUlamTypeIdx();
	s32 symsize = calcVariableSymbolTypeSize(suti); //recursively

	if(symsize == CYCLEFLAG) //was < 0
	  {
	    std::ostringstream msg;
	    msg << "cycle error!!!! " << m_state.getUlamTypeNameByIndex(suti).c_str();
	    MSG(sym->getTokPtr(), msg.str().c_str(),ERR);
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
	    msg << " UTI" << suti << " while compiling quark union: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(sym->getTokPtr(), msg.str().c_str(), DEBUG);
	    maxsize = UNKNOWNSIZE; //t41145
	    break;
	  }
	else
	  {
	    m_state.setBitSize(suti, symsize); //symsize does not include arrays
	  }

	UlamType * sut = m_state.getUlamTypeByIndex(suti); //no sooner!
	s32 x = (s32) sut->getTotalBitSize(); //covers up any unknown sizes
	if(x > maxsize)
	  maxsize = x; //includes arrays

	it++;
      }
    return maxsize;
  } //getMaxVariableSymbolsBitSize

  //#define OPTIMIZE_PACKED_BITS
#ifdef OPTIMIZE_PACKED_BITS
  //currently, packing is done by Nodes since the order of declaration is available;
  //but in case we may want to optimize the layout someday,
  //we keep this here since all the symbols are available in one place.
  void SymbolTableOfVariables::packBitsForTableOfVariableDataMembers(UTI cuti)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 offsetIntoAtom = 0;
    bool isAQuarkUnion = m_state.isClassAQuarkUnion(cuti); // (t3209, t41145) untested
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	UTI suti = sym->getUlamTypeIdx();
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym))
	  {
	    //updates the offset with the bit size of sym
	    ((SymbolVariable *) sym)->setPosOffset(offsetIntoAtom);

	    //quark union needs default pos = 0 for each data member
	    if(!isAQuarkUnion)
	      offsetIntoAtom += m_state.getTotalBitSize(suti); //times array size
	  }
	it++;
      }
  }
#endif

  void SymbolTableOfVariables::initializeElementDefaultsForEval(UlamValue& uvsite, UTI startuti)
  {
    if(m_idToSymbolPtr.empty()) return;

    u32 startpos = ATOMFIRSTSTATEBITPOS; //use relative offsets

    std::map<u32, Symbol* >::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);

	//skip quarkunion initializations (o.w. misleading, which value? e.g. t3782)
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym) && !m_state.isClassAQuarkUnion(suti))
	  {
	    s32 len = sut->getTotalBitSize(); //include arrays (e.g. t3512)
	    assert(sym->isPosOffsetReliable());
	    u32 pos = sym->getPosOffset();

	    //updates the UV at offset with the default of sym;
	    // support initialized non-class arrays
	    if(((SymbolVariableDataMember *) sym)->hasInitValue())
	      {
		assert(len <= MAXSTATEBITS);
		BV8K dval;
		if(((SymbolVariableDataMember *) sym)->getInitValue(dval))
		  {
		    uvsite.putDataBig(pos + startpos, len, dval); //t3772, t3776
		  }
	      }
	    //else nothing to do
	  } //countable
	it++;
      } //while
    return;
  } //initializeElementDefaultsForEval

  s32 SymbolTableOfVariables::findPosOfUlamTypeInTable(UTI utype, UTI& insidecuti)
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
		posfound = sym->getPosOffset();
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
		    posfound = sym->getPosOffset(); //starts at beginning
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
  void SymbolTableOfVariables::genCodeForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(!sym->isTypedef() && !sym->isConstant() && sym->isDataMember()) //including model parameters
	  {
	    ((SymbolVariable *) sym)->generateCodedVariableDeclarations(fp, classtype);
	  }
	it++;
      }
  } //genCodeForTableOfVariableDataMembers (unused)

  void SymbolTableOfVariables::genModelParameterImmediateDefinitionsForTableOfVariableDataMembers(File *fp)
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

  //storage for class members persists, so we give up preserving
  //order of declaration that the NodeVarDecl in the parseTree
  //provides, in order to distinguish between an instance's data
  //members on the STACK verses the classes' data members in
  //EVENTWINDOW.
  void SymbolTableOfVariables::printPostfixValuesForTableOfVariableDataMembers(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
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

  //PRIVATE HELPER METHODS:
  s32 SymbolTableOfVariables::calcVariableSymbolTypeSize(UTI arguti)
  {
    if(!m_state.okUTItoContinue(arguti))
      {
	assert(arguti != Nav);
	if(arguti == Nouti)
	  return UNKNOWNSIZE;
	//else continue if Hzy
      }

    UlamType * argut = m_state.getUlamTypeByIndex(arguti);
    s32 totbitsize = argut->getBitSize(); // why not total bit size? findNodeNoInThisClass fails (e.g. t3144, etc)
    ULAMCLASSTYPE argclasstype = argut->getUlamClassType();
    s32 argarraysize = argut->getArraySize();
    if(argclasstype == UC_NOTACLASS) //includes Atom type
      {
	if(argarraysize == UNKNOWNSIZE)
	  return UNKNOWNSIZE; //Thu Feb  8 16:09:50 2018
	return totbitsize; //arrays handled by caller, just bits here
      }
    //else element size adjusted for Transents by getTotalVariableSymbolsBitSize, after bitsize is set.

    //not a primitive (class), array
    if(argarraysize >= 0)
      {
	if(totbitsize >= 0)
	  {
	    return totbitsize; //array size adjustment by caller getTotal or getMax..
	  }
	if(totbitsize == CYCLEFLAG) //was < 0
	  {
	    m_state.abortShouldntGetHere();
	    return CYCLEFLAG;
	  }
	if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0; //empty, ok
	  }
	else
	  {
	    assert(totbitsize <= UNKNOWNSIZE || m_state.getArraySize(arguti) == UNKNOWNSIZE);

	    m_state.setBitSize(arguti, CYCLEFLAG); //before the recusive call..

	    //get base type, scalar type of class
	    SymbolClass * csym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(arguti, csym))
	      {
		return calcVariableSymbolTypeSize(csym->getUlamTypeIdx()); //NEEDS CORRECTION
	      }
	  }
      }
    else if(m_state.isScalar(arguti)) //not primitive type (class), and not array (scalar)
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
	    if(m_state.alreadyDefinedSymbolClass(arguti, csym))
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
		else if(m_state.isAnonymousClass(cuti))
		  {
		    return UNKNOWNSIZE; //csize?
		  }
		else
		  {
		    //==0, redo variable total
		    NodeBlockClass * classblock = csym->getClassBlockNode();
		    assert(classblock);

		    //a class cannot contain a copy of itself!
		    if(classblock == m_state.getContextBlock())
		      {
			UTI suti = csym->getUlamTypeIdx();
			std::ostringstream msg;
			msg << "Class '" << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
			msg << "' cannot contain a copy of itself"; //error/t3390
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

  bool SymbolTableOfVariables::variableSymbolWithCountableSize(Symbol * sym)
  {
    // may be a zero-sized quark!!
    return (!sym->isTypedef() && !sym->isModelParameter() && !sym->isConstant());
  }

} //end MFM
