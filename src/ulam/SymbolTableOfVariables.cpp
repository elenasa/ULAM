#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTableOfVariables.h"
#include "SymbolModelParameterValue.h"
#include "SymbolTypedef.h"
#include "SymbolVariable.h"
#include "SymbolVariableDataMember.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"

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

  u32 SymbolTableOfVariables::getAllRemainingCulamGeneratedTypedefSymbolsInTable(std::map<u32, Symbol*>& mapref)
  {
    u32 rtnnum = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym);
	if(sym->isTypedef())
	  {
	    if(sym->isCulamGeneratedTypedef())
	      {
		UTI sid = sym->getId();
		std::pair<std::map<u32,Symbol*>::iterator, bool> reti;
		reti = mapref.insert(std::pair<u32,Symbol*>(sid, sym));
		assert(reti.second); //false if already existed, i.e. not added.(t41013)
		rtnnum++;
	      }
	  }
	it++;
      }
    return rtnnum;
  }

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

  s32 SymbolTableOfVariables::getTotalVariableSymbolsBitSize(std::set<UTI>& seensetref)
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
	s32 symsize = UNKNOWNSIZE;
	if(!m_state.okUTItoContinue(suti))
	  //if(!m_state.okUTItoContinue(suti) || m_state.isHolder(suti))
	  {
	    totalsizes = UNKNOWNSIZE;
	    break; //Hzy possibility (t41301)
	  }
	symsize = calcVariableSymbolTypeSize(suti, seensetref); //recursively

	if(symsize == CYCLEFLAG) //was < 0
	  {
	    std::ostringstream msg;
	    msg << "cycle error!!! " << m_state.getUlamTypeNameByIndex(suti).c_str();
	    MSG(sym->getTokPtr(), msg.str().c_str(), ERR);
	    totalsizes = CYCLEFLAG; //t41427
	    break;
	  }
	else if(symsize == EMPTYSYMBOLTABLE)
	  {
	    symsize = 0;
	    m_state.setUTIBitSize(suti, symsize); //total bits NOT including arrays
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
	  {
	    //symsize does not include arrays
	    bool isabaseclass = m_state.isScalar(suti) && m_state.isAClass(suti);
	    s32 savebasebitsize = 0;
	    if(isabaseclass) //t41298,9 (atom), t3143 (array)
	      savebasebitsize = m_state.getBaseClassBitSize(suti);
	    m_state.setUTIBitSize(suti, symsize);
	    if(isabaseclass && (savebasebitsize != UNKNOWNSIZE)) //t41298,9 (atom);t41269
	      m_state.setBaseClassBitSize(suti,savebasebitsize);//restr t3755
	  }

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

  s32 SymbolTableOfVariables::getMaxVariableSymbolsBitSize(std::set<UTI>& seensetref)
  {
    UTI cuti = m_state.getCompileThisIdx();
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
	s32 symsize = calcVariableSymbolTypeSize(suti, seensetref); //recursively

	if(symsize == CYCLEFLAG) //was < 0
	  {
	    std::ostringstream msg;
	    msg << "cycle error!!!! " << m_state.getUlamTypeNameByIndex(suti).c_str();
	    MSG(sym->getTokPtr(), msg.str().c_str(),ERR);
	    maxsize = CYCLEFLAG;
	    break;
	  }
	else if(symsize == EMPTYSYMBOLTABLE)
	  {
	    symsize = 0;
	    m_state.setUTIBitSize(suti, symsize); //total bits NOT including arrays
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
	    m_state.setUTIBitSize(suti, symsize); //symsize does not include arrays
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
  s32 SymbolTableOfVariables::calcVariableSymbolTypeSize(UTI arguti, std::set<UTI>& seensetref)
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
    //ULAMCLASSTYPE argclasstype = argut->getUlamClassType();Hzy fails t41288
    s32 argarraysize = argut->getArraySize();
    if(!m_state.isAClass(arguti)) //includes Atom type, Hzy arrays
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
	    //if(m_state.getArraySize(arguti) == UNKNOWNSIZE)
	    //  return UNKNOWNSIZE; //Tue Jul 13 17:14:35 2021
	    //m_state.setUTIBitSize(arguti, CYCLEFLAG); //before the recusive call..
	    //get base type, scalar type of class
#if 1
	    UTI tduti = Nouti;
	    UTI tdscalaruti = Nouti;
	    Symbol * tdsymptr = NULL;
	    u32 nameid = m_state.getUlamTypeNameIdByIndex(arguti);
	    if(m_state.getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(nameid, tduti, tdscalaruti, tdsymptr))
	      {
		//don't use getTotalBitSize, assumes 0 bits for UNKNOWN, 1 for UNKNOWN arraysize(t3653)
		s32 sizeofclass = m_state.getBitSize(tduti); //calcVariableSymbolTypeSize(tduti, seensetref);
		//		if(sizeofclass >= 0)
		//  return sizeofclass * m_state.getArraySize(arguti);
		return  sizeofclass; //t3145
	      }
	    else
#endif
	      {
		SymbolClass * csym = NULL;
		if(m_state.alreadyDefinedSymbolClass(arguti, csym))
		  {
		    s32 sizeofclass = calcVariableSymbolTypeSize(csym->getUlamTypeIdx(), seensetref); //CORRECTED
		    if(sizeofclass >= 0)
		      return sizeofclass * m_state.getArraySize(arguti);
		    return sizeofclass; //unknownsize? cycle? empty?
		    //when do we multiply by the arraysize??? TODO?? (t3653, t3145)
		  }
	      }
	  }
      }
    else if(m_state.isScalar(arguti)) //not primitive type (class), and not array (scalar)
      {
	if(totbitsize >= 0)
	  {
	    return totbitsize;
	  }
	else if(totbitsize == CYCLEFLAG) //was < 0
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
#if 0
	    UTI tduti = Nouti;
	    UTI tdscalaruti = Nouti;
	    Symbol * tdsymptr = NULL;
	    u32 nameid = m_state.getUlamTypeNameIdByIndex(arguti);
	    if(m_state.getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(nameid, tduti, tdscalaruti, tdsymptr))
	      {
		return m_state.getBitSize(tduti);
	      }
	    else
#endif
	      {
		SymbolClass * csym = NULL;
		if(m_state.alreadyDefinedSymbolClass(arguti, csym))
		  {
		    s32 csize;
		    UTI cuti = csym->getUlamTypeIdx(); //not arguti(t41233,t41262)

		    if((csize = m_state.getBitSize(cuti)) >= 0)
		      {
			return csize;
		      }
		    else if(csize == CYCLEFLAG)  //was < 0
		      {
			//error! cycle..replace with message..at last.
			std::ostringstream msg;
			msg << "Class '" << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
			msg << "' contains a cycle, cannot calculate its bitsize";
			MSG(csym->getTokPtr(), msg.str().c_str(), ERR);
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
			    std::ostringstream msg;
			    msg << "Class '" << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
			    msg << "' cannot contain a copy of itself"; //error/t3390
			    MSG(csym->getTokPtr(), msg.str().c_str(), ERR);
			    classblock->setNodeType(Nav);
			    return UNKNOWNSIZE;
			  }

			bool aok = true;
			s32 sharedbits = UNKNOWNSIZE;
			s32 basebits = 0; //overstated, no sharing
			s32 mybits = 0; //main goal of trySetBitsize..

			std::pair<std::set<UTI>::iterator,bool> ret = seensetref.insert(cuti);
			if(ret.second)
			  {
			    //first sighting
			    m_state.pushClassContext(cuti, classblock, classblock, false, NULL);
			    aok = csym->trySetBitsizeWithUTIValues(basebits, mybits, seensetref);
			  }
			else
			  {
			    aok = false;
			    m_state.setUTIBitSize(cuti, CYCLEFLAG);//t41427, t3383
			    return CYCLEFLAG;
			  }

			if(aok)
			  {
			    s32 sharedbitssaved = UNKNOWNSIZE;
			    aok = csym->determineSharedBasesAndTotalBitsize(sharedbitssaved, sharedbits);
			    if(aok)
			      {
				assert(sharedbits >= 0);
				assert(sharedbitssaved >= sharedbits);
				csize = (mybits + sharedbits); //updates total here!!
				//before setUTIsizes, restored later (t3755)
				aok = m_state.setBaseClassBitSize(cuti, mybits); //noop for elements
			      }
			  }
			m_state.popClassContext(); //restore
			return aok ? csize : UNKNOWNSIZE;
		      }
		  } //class
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
