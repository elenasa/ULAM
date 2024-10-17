#include <stack>
#include <sstream>
#include <string.h>
#include <errno.h>
#include "CompilerState.h"
#include "SymbolClass.h"
#include "SymbolTypedef.h"
#include "SymbolClassName.h"
#include "Resolver.h"
#include "Parity2D_4x4.h"

namespace MFM {

  SymbolClass::SymbolClass(const Token& id, UTI utype, NodeBlockClass * classblock, SymbolClassNameTemplate * parent, CompilerState& state) : Symbol(id, utype, state), m_resolver(NULL), m_classBlock(classblock), m_parentTemplate(parent), m_quarkunion(false), m_stub(true), m_stubcopy(false), m_stubForTemplate(false), m_stubForTemplateType(Nouti), m_stubForTemplateTypeIncomplete(false), m_stubcopyOf(Nouti), /*m_defaultValue(NULL),*/ m_isreadyDefaultValue(false) /* default */, m_bitsPacked(false), m_registryNumber(UNINITTED_REGISTRY_NUMBER), m_elementType(UNDEFINED_ELEMENT_TYPE), m_vtableinitialized(false), m_concreteclass(false)
  {
    appendBaseClass(Nouti, true);
  }
  SymbolClass::SymbolClass(const SymbolClass& sref) : Symbol(sref), m_resolver(NULL), m_parentTemplate(sref.m_parentTemplate), m_quarkunion(sref.m_quarkunion), m_stub(sref.m_stub), m_stubcopy(sref.m_stubcopy), m_stubForTemplate(false), m_stubForTemplateType(Nouti), m_stubForTemplateTypeIncomplete(false), m_stubcopyOf(sref.m_stubcopyOf), /*m_defaultValue(NULL),*/ m_isreadyDefaultValue(false), m_bitsPacked(false), m_registryNumber(UNINITTED_REGISTRY_NUMBER), m_elementType(UNDEFINED_ELEMENT_TYPE), m_vtableinitialized(false), m_concreteclass(sref.m_concreteclass)
  {
    resetUlamType(m_state.getCompileThisIdx()); //symbols Hzy by default

    for(u32 i = 0; i < sref.m_basestable.size(); i++)
      {
	appendBaseClass(m_state.mapIncompleteUTIForCurrentClassInstance(sref.m_basestable[i].m_base,sref.getLoc()), sref.isDirectSharedBase(i));
	setNumberSharingBase(i, sref.getNumberSharingBase(i));
      }

    for(u32 j = 0; j < sref.m_sharedbasestable.size(); j++)
      {
	appendSharedBaseClass(m_state.mapIncompleteUTIForCurrentClassInstance(sref.m_sharedbasestable[j].m_base,sref.getLoc()), sref.getNumberSharingSharedBase(j));
      }

    if(sref.m_classBlock)
      {
	m_classBlock = (NodeBlockClass * ) sref.m_classBlock->instantiate(); //note: wasn't correct uti during cloning

	// note: if superclass, the prevBlockPtr of m_classBlock hasn't been set yet!
	// does new m_functionST op= work instead? YES.
	//m_classBlock->setMemberFunctionsSymbolTable(m_classBlock->getNodeType(),*sref.m_classBlock);
      }
    else
      m_classBlock = NULL; //i.e. UC_UNSEEN

    if(sref.m_resolver)
      m_resolver = new Resolver(getUlamTypeIdx(), m_state); //not a clone, populated later
  }

  SymbolClass::~SymbolClass()
  {
    delete m_classBlock;
    m_classBlock = NULL;

    if(m_resolver)
      {
	delete m_resolver;
	m_resolver = NULL;
      }

    m_basestable.clear();
    m_sharedbasestable.clear();
    m_basesVTstart.clear();
  }

  Symbol * SymbolClass::clone()
  {
    m_state.abortShouldntGetHere();
    return new SymbolClass(*this);
  }

  void SymbolClass::setClassBlockNode(NodeBlockClass * node)
  {
    //SYMBOL_ASSERT(m_classBlock == NULL); t3130, etc..
    m_classBlock = node;
    if(m_classBlock)
      Symbol::setBlockNoOfST(node->getNodeNo());
    else
      Symbol::setBlockNoOfST(0);
  }

  void SymbolClass::resetClassBlockNode(NodeBlockClass * node)
  {
    if(m_classBlock)
      {
	delete m_classBlock; //avoid leak
	m_classBlock = NULL;
      }
    setClassBlockNode(node);
  }

  NodeBlockClass * SymbolClass::getClassBlockNode()
  {
    return m_classBlock;
  }

  SymbolClassNameTemplate * SymbolClass::getParentClassTemplate()
  {
    return m_parentTemplate; //could be self
  }

  void SymbolClass::setParentClassTemplate(SymbolClassNameTemplate  * p)
  {
    SYMBOL_ASSERT(p);
    m_parentTemplate = p;
  }

  bool SymbolClass::isClass()
  {
    return true;
  }

  bool SymbolClass::isClassTemplateByUTI(UTI cuti)
  {
    return false;
  }

  bool SymbolClass::isStubForTemplate()
  {
    return m_stubForTemplate; //e.g. base class, class parameter
  }

  void SymbolClass::setStubForTemplate()
  {
    m_stubForTemplate = true;
  }

  void SymbolClass::clearStubForTemplate()
  {
    m_stubForTemplate = false;
    setStubForTemplateType(Nouti);
  }

  UTI SymbolClass::getStubForTemplateType()
  {
    return m_stubForTemplateType;
  }

  void SymbolClass::setStubForTemplateType(UTI ttype)
  {
    m_stubForTemplateType = ttype;
    setStubForTemplate();
  }

  bool SymbolClass::isStubForTemplateTypeIncomplete()
  {
    return m_stubForTemplateTypeIncomplete;
  }

  void SymbolClass::setStubForTemplateTypeIncomplete(bool incomplete)
  {
    m_stubForTemplateTypeIncomplete = incomplete;
  }

  u32 SymbolClass::getBaseClassCount()
  {
    u32 numbases = m_basestable.size();
    SYMBOL_ASSERT(numbases >= 1);
    return numbases - 1; //excluding super class
  }

  UTI SymbolClass::getBaseClass(u32 item)
  {
    SYMBOL_ASSERT(item < m_basestable.size());
    return m_basestable[item].m_base;
  }

  s32 SymbolClass::isABaseClassItem(UTI buti)
  {
    return isABaseClassItemSearch(buti); //slow, uses compare..
  } //isABaseClassItem

  s32 SymbolClass::isABaseClassItemSearch(UTI buti)
  {
    s32 item = -1; //negative is not found
    if(m_basestable.empty()) return item;

    for(u32 i = 0; i < m_basestable.size(); i++)
      {
	UTI baseuti = m_basestable[i].m_base;
	if( baseuti != Nouti) //super optional (t3102)
	  {
	    //incomplete wont match, yet (20210722 ish)
	    if(UlamType::compare(baseuti, buti, m_state) != UTIC_NOTSAME)
	      {
		item = i;
		break;
	      }
	  }
      }
    return item; //includes super
  } //isABaseClassItemSearch

  bool SymbolClass::isDirectSharedBase(u32 item) const
  {
    SYMBOL_ASSERT(item < m_basestable.size());
    return (getNumberSharingBase(item) > 0);
  }

  u32 SymbolClass::countDirectSharedBases() const
  {
    u32 count = 0;
    for(u32 i = 0; i < m_basestable.size(); i++)
      {
	UTI baseuti = m_basestable[i].m_base;
	if( baseuti != Nouti) //super optional
	  {
	    if(isDirectSharedBase(i))
	      count++;
	  }
      }
    return count;
  } //countDirectSharedBases

  u32 SymbolClass::findDirectSharedBases(std::map<UTI, u32>& svbmapref)
  {
    u32 count = 0;
    for(u32 i = 0; i < m_basestable.size(); i++)
      {
	UTI baseuti = m_basestable[i].m_base;
	if( baseuti != Nouti) //super optional
	  {
	    if(isDirectSharedBase(i))
	      {
		UTI rootuti = baseuti;
		if(!m_state.isARootUTI(baseuti))
		  {
		    AssertBool gotroot = m_state.findRootUTIAlias(baseuti, rootuti); //t3652
		    SYMBOL_ASSERT(gotroot); //note: not mapped in resolver
		  }

		BasesTableTypeMap::iterator it = svbmapref.find(rootuti);
		if(it != svbmapref.end())
		  {
		    it->second++; //increment
		  }
		else
		  {
		    svbmapref.insert(std::pair<UTI, u32>(rootuti, 1));
		  }
		count++;
	      }
	  }
      }
    return count;
  } //findDirectSharedBases

  void SymbolClass::appendBaseClass(UTI baseclass, bool sharedbase)
  {
    UTI rootuti = baseclass;
    if(!m_state.isARootUTI(baseclass))
      {
	AssertBool gotroot = m_state.findRootUTIAlias(baseclass, rootuti); //t3652
	SYMBOL_ASSERT(gotroot); //note: not mapped in resolver
      }
    //note: catch any duplicates, later, during packing bits (20210722 ish)
    BaseClassEntry bentry;
    bentry.m_base = rootuti;
    bentry.m_numbaseshared = (sharedbase ? 1 : 0); //=true, all shared virtual ^base
    bentry.m_basepos = UNKNOWNSIZE; //pos unknown
    bentry.m_dupflag = false; //default, until packbits happens later
    m_basestable.push_back(bentry);
  } //appendBaseClass

  void SymbolClass::updateBaseClass(UTI oldclasstype, u32 item, UTI newbaseclass)
  {
    SYMBOL_ASSERT(m_state.isARootUTI(newbaseclass)); //t3652
    SYMBOL_ASSERT(!m_state.isUrSelf(getUlamTypeIdx()));
    SYMBOL_ASSERT(!isClassTemplateByUTI(getUlamTypeIdx()) || !m_state.isClassAStubCopy(newbaseclass)); //t41448
    SYMBOL_ASSERT(item < m_basestable.size());
    SYMBOL_ASSERT(m_basestable[item].m_base == oldclasstype);
    m_basestable[item].m_base = newbaseclass;

    if(item==0)
      updateSuperTypedef(newbaseclass);
  }

  void SymbolClass::setBaseClass(UTI baseclass, u32 item, bool sharedbase)
  {
    SYMBOL_ASSERT(m_state.isARootUTI(baseclass)); //t3652
    if(item == 0)
      {
	setSuperBaseClass(baseclass); //t3806
      }
    else if(item == m_basestable.size())
      {
	appendBaseClass(baseclass, sharedbase);
      }
    else
      {
	SYMBOL_ASSERT(item < m_basestable.size());
	updateBaseClass(m_basestable[item].m_base, item, baseclass);
      }
  } //setBaseClass

  void SymbolClass::setSuperBaseClass(UTI superuti)
  {
    UTI oldbaseclass = getBaseClass(0); //Super use might have been parsed

    if(!m_state.isUrSelf(getUlamTypeIdx()))
      {
	updateBaseClass(oldbaseclass, 0, superuti);

	if(m_state.isHolder(oldbaseclass))
	  {
	    //mapUTItoUTI(oldbaseclass, superuti); //t3806 redundant
	    m_state.cleanupExistingHolder(oldbaseclass, superuti);
	  }
	updateSuperTypedef(superuti);
      }
  } //setSuperBaseClass

  void SymbolClass::updateSuperTypedef(UTI superuti)
  {
    //may not get to cblock's c&l before fullInstant, stub w default args (t41452)
    NodeBlockClass * cblock = getClassBlockNode();
    SYMBOL_ASSERT(cblock);

    u32 superid = m_state.m_pool.getIndexForDataString("Super");
    Symbol * symtypedef = NULL;
    if(!cblock->isIdInScope(superid, symtypedef))
      {
	Token superTok(TOK_KW_TYPE_SUPER, cblock->getNodeLocation(), 0);
	symtypedef = new SymbolTypedef(superTok, superuti, superuti, m_state);
	SYMBOL_ASSERT(symtypedef);
	symtypedef->setBlockNoOfST(cblock->getNodeNo());
	cblock->addIdToScope(superid,symtypedef);
      }
    else
      symtypedef->resetUlamType(superuti); //any aliasing needed?
  } //updateSuperTypedef

  void SymbolClass::clearBaseAsShared(u32 item)
  {
    SYMBOL_ASSERT(item < m_basestable.size());
    m_basestable[item].m_numbaseshared = 0;
  }

  void SymbolClass::setNumberSharingBase(u32 item, u32 numshared)
  {
    SYMBOL_ASSERT(item < m_basestable.size());
    m_basestable[item].m_numbaseshared = numshared;
  }

  u32 SymbolClass::getNumberSharingBase(u32 item) const
  {
    SYMBOL_ASSERT(item < m_basestable.size());
    return m_basestable[item].m_numbaseshared;
  }

  s32 SymbolClass::getBaseClassRelativePosition(u32 item) const
  {
    SYMBOL_ASSERT(item < m_basestable.size());
    return m_basestable[item].m_basepos;
  }

  void SymbolClass::setBaseClassRelativePosition(u32 item, u32 pos, bool dupflag)
  {
    SYMBOL_ASSERT(item < m_basestable.size());
    m_basestable[item].m_basepos = (s32) pos;
    m_basestable[item].m_dupflag = dupflag;
  }

  bool SymbolClass::isADuplicateBaseClass(u32 item)
  {
    SYMBOL_ASSERT(item < m_basestable.size());
    return m_basestable[item].m_dupflag;
  }

  UTI SymbolClass::getSharedBaseClass(u32 item)
  {
    SYMBOL_ASSERT(item < m_sharedbasestable.size());
    return m_sharedbasestable[item].m_base;
  }

  s32 SymbolClass::isASharedBaseClassItem(UTI buti)
  {
    return isASharedBaseClassItemSearch(buti); //slow, uses compare..
  } //isASharedBaseClassItem

  s32 SymbolClass::isASharedBaseClassItemSearch(UTI buti)
  {
    s32 item = -1; //negative is not found
    for(u32 i = 0; i < m_sharedbasestable.size(); i++)
      {
	UTI baseuti = m_sharedbasestable[i].m_base;
	if( baseuti != Nouti) //super optional (t3102)
	  {
	    if(UlamType::compare(baseuti, buti, m_state) == UTIC_SAME)
	      {
		item = i;
		break;
	      }
	  }
      }
    return item; //includes super
  } //isASharedBaseClassItemSearch

  u32 SymbolClass::getSharedBaseClassCount() const
  {
    return m_sharedbasestable.size();
  } //getSharedBaseClassCount

  u32 SymbolClass::getNumberSharingSharedBase(u32 item) const
  {
    SYMBOL_ASSERT(item < m_sharedbasestable.size());
    return m_sharedbasestable[item].m_numbaseshared;
  }

  void SymbolClass::appendSharedBaseClass(UTI baseclass, u32 numshared)
  {
    UTI rootuti = baseclass;
    if(!m_state.isARootUTI(baseclass))
      {
	AssertBool gotroot = m_state.findRootUTIAlias(baseclass, rootuti); //t3652
	SYMBOL_ASSERT(gotroot); //note: not mapped in resolver
      }

    BaseClassEntry bentry;
    bentry.m_base = rootuti;
    bentry.m_numbaseshared = numshared; //shared virtual ^base
    bentry.m_basepos = UNKNOWNSIZE; //pos unknown
    bentry.m_dupflag = false; //default, until packbits happens later
    m_sharedbasestable.push_back(bentry);
  } //appendSharedBaseClass

  void SymbolClass::updateSharedBaseClass(UTI oldclasstype, u32 item, UTI newbaseclass)
  {
    SYMBOL_ASSERT(m_state.isARootUTI(newbaseclass)); //t3652
    SYMBOL_ASSERT(!m_state.isUrSelf(getUlamTypeIdx()));
    SYMBOL_ASSERT(item < m_sharedbasestable.size());
    SYMBOL_ASSERT(m_sharedbasestable[item].m_base == oldclasstype);
    m_sharedbasestable[item].m_base = newbaseclass;
  }

  s32 SymbolClass::getSharedBaseClassRelativePosition(u32 item) const
  {
    SYMBOL_ASSERT(item < m_sharedbasestable.size());
    return m_sharedbasestable[item].m_basepos;
  }

  void SymbolClass::setSharedBaseClassRelativePosition(u32 item, u32 pos)
  {
    SYMBOL_ASSERT(item < m_sharedbasestable.size());
    m_sharedbasestable[item].m_basepos = (s32) pos;
  }

  const std::string SymbolClass::getMangledPrefix()
  {
    return m_state.getUlamTypeByIndex(getUlamTypeIdx())->getUlamTypeUPrefix();
  }

  ULAMCLASSTYPE SymbolClass::getUlamClass()
  {
    return  m_state.getUlamTypeByIndex(getUlamTypeIdx())->getUlamClassType(); //helper
  }

  void SymbolClass::setQuarkUnion()
  {
    m_quarkunion = true;
  }

  bool SymbolClass::isQuarkUnion()
  {
    return m_quarkunion;
  }

  bool SymbolClass::isStub()
  {
    return m_stub;
  }

  void SymbolClass::unsetStub()
  {
    m_stub = false;
  }

  bool SymbolClass::isStubCopy()
  {
    return m_stubcopy;
  }

  void SymbolClass::unsetStubCopy()
  {
    m_stubcopy = false;
  }

  void SymbolClass::setStubCopy()
  {
    m_stubcopy = true;
  }

  UTI SymbolClass::getStubCopyOf()
  {
    return m_stubcopyOf;
  }

  void SymbolClass::setStubCopyOf(UTI stubuti)
  {
    m_stubcopyOf = stubuti;
  }

  UTI SymbolClass::getCustomArrayType()
  {
    NodeBlockClass * classNode = getClassBlockNode(); //instance
    SYMBOL_ASSERT(classNode);
    return classNode->getCustomArrayTypeFromGetFunction(getUlamTypeIdx()); //returns canonical type
  }

  u32 SymbolClass::getCustomArrayIndexTypeFor(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    NodeBlockClass * classNode = getClassBlockNode(); //instance
    SYMBOL_ASSERT(classNode);
    //returns number of matching types; updates last two args.
    return classNode->getCustomArrayIndexTypeFromGetFunction(getUlamTypeIdx(), rnode, idxuti, hasHazyArgs);
  }

  bool SymbolClass::hasCustomArrayLengthof()
  {
    NodeBlockClass * classNode = getClassBlockNode(); //instance
    SYMBOL_ASSERT(classNode);
    return classNode->hasCustomArrayLengthofFunction(getUlamTypeIdx());
  }

  bool SymbolClass::trySetBitsizeWithUTIValues(s32& basebits, s32& mybits, std::set<UTI>& seensetref)
  {
    NodeBlockClass * classNode = getClassBlockNode(); //instance
    bool aok = true;

    //of course they always aren't! but we know to keep looping..
    UTI suti = getUlamTypeIdx();
    if(! m_state.isComplete(suti))
      aok = false;

    s32 totalbits;
    if(isQuarkUnion())
      totalbits = classNode->getMaxBitSizeOfVariableSymbolsInTable(basebits, mybits, seensetref);
    else
      totalbits = classNode->getBitSizesOfVariableSymbolsInTable(basebits, mybits, seensetref);

    //avoid setting EMPTYSYMBOLTABLE instead of 0 for zero-sized classes
    if(totalbits == CYCLEFLAG)  // was < 0
      {
	std::ostringstream msg;
	msg << "cycle error!! " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	aok = false; //t41427
      }
    else if(totalbits == EMPTYSYMBOLTABLE)
      {
	if(isQuarkUnion())
	  {
	    std::ostringstream msg;
	    msg << "Union " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	    msg << " has no data members; Bitsize is 0";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	    aok = false; //t41426
	  }
	else
	  {
	    basebits = mybits = totalbits = 0;
	    aok = true;
	  }
      }
    else if(totalbits != UNKNOWNSIZE)
      aok = true; //KNOWN
    return aok;
  } //trySetBitSizeWithUTIValues

  bool SymbolClass::determineSharedBasesAndTotalBitsize(s32& sharedbitssaved, s32& sharedbitsize)
  {
    //builds shared base table, for rel pos (later)
    std::map<UTI, u32> svbmap; //shared virtual base map
    UTI suti = getUlamTypeIdx();

    m_state.findTheSharedVirtualBasesInAClassHierarchy(suti, svbmap);

    u32 totalsharedbasebitsize = 0; //bits to add back
    u32 bitssaved = 0; //bits to subtract fm total?

    std::map<UTI, u32>::iterator it = svbmap.begin();
    while(it != svbmap.end())
      {
	UTI baseuti = it->first;
	SYMBOL_ASSERT(m_state.isComplete(baseuti));
	SYMBOL_ASSERT(m_state.isASeenClass(baseuti));
	u32 numshared = it->second;
	SYMBOL_ASSERT(numshared > 0);

	s32 basebitsize = m_state.getBaseClassBitSize(baseuti);
	if(basebitsize == UNKNOWNSIZE) //t3755
	  return false; //wait..

	bitssaved += (basebitsize * numshared);
	totalsharedbasebitsize += basebitsize;

	if(isASharedBaseClassItem(baseuti) < 0)
	  appendSharedBaseClass(baseuti, numshared); //t41423

	s32 bitem = isABaseClassItem(baseuti);
	if(bitem >= 0) //direct shared
	  setNumberSharingBase(bitem, numshared);
	it++;
      } //while

    sharedbitsize = totalsharedbasebitsize; //one of each base, all shared
    sharedbitssaved = bitssaved; //no confidence
    svbmap.clear();
    return true;
  } //determineSharedBasesAndTotalBitsize

  void SymbolClass::printBitSizeOfClass()
  {
    UTI suti = getUlamTypeIdx();
    SYMBOL_ASSERT(m_state.okUTItoContinue(suti));
    u32 total = m_state.getTotalBitSize(suti);
    UlamType * sut = m_state.getUlamTypeByIndex(suti);
    ULAMCLASSTYPE classtype = sut->getUlamClassType();

    std::ostringstream msg;
    msg << "[UTBUA] Total bits used/available by ";
    if(classtype == UC_ELEMENT)
       msg << "element ";
    else if(classtype == UC_QUARK)
       msg << "quark ";
    else if(classtype == UC_TRANSIENT)
      msg << "transient ";
    else
      m_state.abortUndefinedUlamClassType();

    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str() << " : ";

    if(m_state.isComplete(suti))
      {
	s32 remaining = 0;
	if(classtype == UC_ELEMENT)
	  remaining = (MAXSTATEBITS - total);
	else if(classtype == UC_QUARK)
	  remaining = (MAXBITSPERQUARK - total);
	else if(classtype == UC_TRANSIENT)
	  remaining = total;
	else
	  m_state.abortUndefinedUlamClassType();

	msg << total << "/" << remaining;
      }
    else
      {
	total = UNKNOWNSIZE;
	s32 remaining = 0;
	if(classtype == UC_ELEMENT)
	  remaining = (MAXSTATEBITS);
	else if(classtype == UC_QUARK)
	  remaining = (MAXBITSPERQUARK);
	else if(classtype == UC_TRANSIENT)
	  remaining = total;
	else
	  m_state.abortUndefinedUlamClassType();

	msg << "UNKNOWN" << "/" << remaining;
      }
    MSG(m_state.getFullLocationAsString(getLoc()).c_str(), msg.str().c_str(), INFO);
  } //printBitSizeOfClass

  bool SymbolClass::getDefaultQuark(u64& dqref)
  {
    SYMBOL_ASSERT(getUlamClass() == UC_QUARK);

    if(!m_isreadyDefaultValue)
      {
	BV8K dk;
	getDefaultValue(dk);
      }
    u32 len = m_state.getBitSize(getUlamTypeIdx());
    dqref = m_defaultValue.ReadLong(0u, len); //return value
    return m_isreadyDefaultValue;
  } //getDefaultQuark

  bool SymbolClass::getPackedDefaultValue(u64& dpkref)
  {
    if(!m_isreadyDefaultValue)
      {
	BV8K dk;
	getDefaultValue(dk);
      }
    u32 len = m_state.getBitSize(getUlamTypeIdx());
    dpkref = m_defaultValue.ReadLong(0u, len); //return value
    return m_isreadyDefaultValue;
  } //getPackedDefaultValue

  bool SymbolClass::getDefaultValue(BV8K& dvref)
  {
    //could be any length up to 8K..(i.e. transient)
    // element that doesn't fit into a u64
    if(m_isreadyDefaultValue)
      {
	dvref = m_defaultValue;
	return true; //short-circuit, known
      }

    UTI suti = getUlamTypeIdx();
    UlamType * sut = m_state.getUlamTypeByIndex(suti);

    if(!sut->isComplete())
      {
	std::ostringstream msg;
	msg << "Cannot get default value for incomplete class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	MSG(Symbol::getTokPtr(),msg.str().c_str(), WAIT);
	return false; //t3875
      }

    u32 usizeof = sut->getSizeofUlamType(); //t3802
    if(usizeof == 0)
      {
 	m_isreadyDefaultValue = true;
	dvref = m_defaultValue;
	return true; //short-circuit, no data members, nor an element
      }

    if(sut->getUlamClassType() == UC_ELEMENT)
      {
	//prepended before going to class block tp build the rest..
	ELE_TYPE type = getElementType();  //t3173, t3206
	dvref.Write(0,ATOMFIRSTSTATEBITPOS, Parity2D_4x4::Add2DParity(type));
      }

    BV8K basedv; //elements aren't base classes!!
    NodeBlockClass * classblock = getClassBlockNode();
    SYMBOL_ASSERT(classblock);
    m_state.pushClassContext(suti, classblock, classblock, false, NULL); //missing?

    if((m_isreadyDefaultValue = classblock->buildDefaultValue(usizeof, dvref)))
      {
	m_defaultValue = dvref;
      }

    m_state.popClassContext();
    return m_isreadyDefaultValue;
  } //getDefaultValue

  TBOOL SymbolClass::packBitsForClassVariableDataMembers()
  {
    if(m_bitsPacked)
      return TBOOL_TRUE;

    TBOOL rtntb = TBOOL_FALSE;
    UTI suti = getUlamTypeIdx();
    if(m_state.isComplete(suti))
      {
	NodeBlockClass * classblock = getClassBlockNode();
	SYMBOL_ASSERT(classblock);
	m_state.pushClassContext(suti, classblock, classblock, false, NULL);

	rtntb = classblock->packBitsForVariableDataMembers(); //side-effect

	m_state.popClassContext();

	if(rtntb == TBOOL_TRUE)
	  m_bitsPacked = true;
      }
    else
      {
	std::ostringstream msg;
	msg << " Class '";
	msg << m_state.getUlamTypeNameByIndex(suti).c_str();
	msg << "' is still incomplete; No pack bits for data members, error";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), DEBUG); //t3862
	rtntb = TBOOL_HAZY;
      }
    return rtntb;
  } //packBitsForClassVariableDataMembers

  void SymbolClass::testThisClass(File * fp)
  {
    NodeBlockClass * classNode = getClassBlockNode();
    SYMBOL_ASSERT(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    if(classNode->findTestFunctionNode())
      {
	// set up an atom in eventWindow; init m_currentObjPtr to point to it
	// set up STACK since func call not called
	m_state.setupCenterSiteForTesting();

	m_state.m_nodeEvalStack.addFrameSlots(1); //prolog, 1 for return
	s32 rtnValue = 0;
	EvalStatus evs = classNode->eval();
	if(evs == NORMAL)
	  {
	    UlamValue rtnUV = m_state.m_nodeEvalStack.popArg();
	    rtnValue = rtnUV.getImmediateData(32, m_state);
	  }
	else if(evs == BREAK)
	  {
	    UlamValue rtnUV = m_state.m_nodeEvalStack.popArg();
	    rtnValue = rtnUV.getImmediateData(32, m_state); //t41016 (no loop to catch it!)
	  }
	else if(evs == UNEVALUABLE)
	  rtnValue = -11;
	else if(evs == NOTREADY)
	  rtnValue = -12;
	else
	  rtnValue = -1; //error!

	//#define CURIOUS_T3146
#ifdef CURIOUS_T3146
	//curious..
	{
	  UlamValue objUV = m_state.m_eventWindow.loadAtomFromSite(c0.convertCoordToIndex());
	  u32 data = objUV.getData(25,32); //Int f.m_i (t3146)
	  std::ostringstream msg;
	  msg << "Output for m_i = <" << data << "> (expecting 4 for t3146)";
	  MSG(Symbol::getTokPtr(),msg.str().c_str(), INFO);
	}
#endif
	m_state.m_nodeEvalStack.returnFrame(m_state); //epilog

	fp->write("Exit status: " ); //in compared answer
	fp->write_decimal(rtnValue);
	fp->write("\n");
      } //test eval
    m_state.popClassContext(); //missing?
  } //testThisClass

  void SymbolClass::addUnknownTypeTokenToClass(const Token& tok, UTI huti)
  {
    if(!m_resolver)
      m_resolver = new Resolver(getUlamTypeIdx(), m_state);
    SYMBOL_ASSERT(m_resolver);
    m_resolver->addUnknownTypeToken(tok, huti);
    if(tok.m_type == TOK_KW_TYPE_SUPER)
      setBaseClass(huti, 0, true); //t41312??
  } //addUnknownTypeTokenToClass

  Token SymbolClass::removeKnownTypeTokenFromClass(UTI huti)
  {
    SYMBOL_ASSERT(m_resolver);
    return m_resolver->removeKnownTypeToken(huti);
  } //removeKnownTypeTokenFromClass

  bool SymbolClass::hasUnknownTypeInClass(UTI huti)
  {
    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver)
      return false;
    return m_resolver->hasUnknownTypeToken(huti);
  }

  bool SymbolClass::getUnknownTypeTokenInClass(UTI huti, Token& tok)
  {
    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver)
      return false;
    return m_resolver->getUnknownTypeToken(huti, tok);
  }

  bool SymbolClass::statusUnknownTypeInClass(UTI huti)
  {
    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver)
      return false;
    return m_resolver->statusUnknownType(huti, this);
  }

  bool SymbolClass::statusUnknownTypeNamesInClass()
  {
    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver)
      return true;
    return m_resolver->statusAnyUnknownTypeNames();
  }

  u32 SymbolClass::reportUnknownTypeNamesInClass()
  {
    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver)
      return 0;
    return m_resolver->reportAnyUnknownTypeNames();
  }

  bool SymbolClass::reportLongClassName()
  {
    bool istoolong = false;
    UTI suti = getUlamTypeIdx();
    UlamType * sut = m_state.getUlamTypeByIndex(suti);
    std::string classname = sut->getUlamTypeMangledName();
    u32 cnamelen = classname.length();
    if(cnamelen > MAX_FILENAME_LENGTH)
      {
	std::ostringstream msg;
	msg << "Mangled Class Instance Name: <";
	msg << classname.c_str() << ">; ";
	msg << "exceeds the maximum length (" << MAX_FILENAME_LENGTH;
	msg << ") before extensions, length is " << cnamelen;
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	istoolong = true;
      }
    return istoolong;
  } //reportLongClassName

  void SymbolClass::linkConstantExpressionForPendingArg(NodeConstantDef * constNode)
  {
    if(!m_resolver)
      m_resolver = new Resolver(getUlamTypeIdx(), m_state);
    //    SYMBOL_ASSERT(m_stub); //stubs only have pending args (t41441)
    m_resolver->linkConstantExpressionForPendingArg(constNode);

    //new owner of the NodeConstantDef! Sun Aug 21 09:09:57 2016
    NodeBlockClass * cblock = getClassBlockNode();
    SYMBOL_ASSERT(cblock);
    cblock->addArgumentNode(constNode);
  } //linkConstantExpressionForPendingArg

  bool SymbolClass::pendingClassArgumentsForClassInstance()
  {
    //    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver) //stubs only!
      return false; //ok, none pending
    return m_resolver->pendingClassArgumentsForClassInstance();
  }

  void SymbolClass::cloneArgumentNodesForClassInstance(SymbolClass * fmcsym, UTI argvaluecontext, UTI argtypecontext)
  {
    SYMBOL_ASSERT(fmcsym); //from
    //UTI fmuti = fmcsym->getUlamTypeIdx();
    NodeBlockClass * fmclassblock = fmcsym->getClassBlockNode();
    SYMBOL_ASSERT(fmclassblock);
    NodeBlockClass * classblock = getClassBlockNode();
    SYMBOL_ASSERT(classblock);

    u32 numArgs = fmclassblock->getNumberOfArgumentNodes();

    SymbolClassNameTemplate * ptcsym = fmcsym->getParentClassTemplate();
    SYMBOL_ASSERT(ptcsym);
    NodeBlockClass * ptclassblock = ptcsym->getClassBlockNode();
    SYMBOL_ASSERT(ptclassblock);

    for(u32 i = 0; i < numArgs; i++)
      {
	NodeConstantDef * parmNode = (NodeConstantDef *) ptclassblock->getParameterNode(i);

	NodeConstantDef * ceNode = (NodeConstantDef *) fmclassblock->getArgumentNode(i);
	SYMBOL_ASSERT(ceNode);
	ceNode->fixPendingArgumentNode(); //sets m_cid

	NodeConstantDef * cloneNode = NULL;
	if(ceNode->isClassArgumentItsDefaultValue())
	  cloneNode =  (NodeConstantDef *) parmNode->instantiate();
	else
	  cloneNode = (NodeConstantDef *) ceNode->instantiate();
	SYMBOL_ASSERT(cloneNode);
	SYMBOL_ASSERT(ceNode->getNodeNo() == cloneNode->getNodeNo());

	Symbol * cvsym = NULL;
	AssertBool isDefined = classblock->isIdInScope(parmNode->getNameId(), cvsym);
	SYMBOL_ASSERT(isDefined); //t3498,9..
	//if(isDefined)
	  cloneNode->setSymbolPtr((SymbolConstantValue *) cvsym); //sets declnno

	linkConstantExpressionForPendingArg(cloneNode); //resolve later; adds to classblock, etc.
      } //end forloop
  } //cloneArgumentNodesForClassInstance

  void SymbolClass::partialInstantiationOfMemberNodesAndSymbols(NodeBlockClass & fromclassblock)
  {
    NodeBlockClass * cblock = getClassBlockNode();
    UTI suti = getUlamTypeIdx();

    m_state.pushClassContext(suti, cblock, cblock, false, NULL);

    cblock->resetDataMembersParseTree(suti, fromclassblock);
    cblock->updateLineage(0);
    cblock->setDataMembersSymbolTable(suti, fromclassblock);

    m_state.popClassContext(); //restore
  }

  void SymbolClass::cloneResolverUTImap(SymbolClass * csym)
  {
    SYMBOL_ASSERT(csym); //to
    if(m_resolver)
      m_resolver->cloneUTImapForNonclasses(csym);
  } //cloneResolverUTImap

  void SymbolClass::cloneUnknownTypesMapInClass(SymbolClass * to)
  {
    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver)
      return;
    return m_resolver->cloneUnknownTypesTokenMap(to);
  }

  void SymbolClass::setContextForPendingArgValues(UTI context)
  {
    if(!m_resolver)
      m_resolver = new Resolver(getUlamTypeIdx(), m_state); //t41446

    m_resolver->setContextForPendingArgValues(context);
  } //setContextForPendingArgValues

  UTI SymbolClass::getContextForPendingArgValues()
  {
    //SYMBOL_ASSERT(m_resolver); //dangerous! when template has default parameters
    if(m_resolver)
      return m_resolver->getContextForPendingArgValues();

    SYMBOL_ASSERT(!isStub() || (getParentClassTemplate() && ((getParentClassTemplate()->getTotalParametersWithDefaultValues() > 0) || (getParentClassTemplate()->getUlamClass() == UC_UNSEEN))) || !m_state.isComplete(getUlamTypeIdx())); //e.g. not complete if bases have pending arg types (t3981, t41446)
    return Nouti; //getUlamTypeIdx(); //return self UTI, t3981
  } //getContextForPendingArgValues

  void SymbolClass::setContextForPendingArgTypes(UTI context)
  {
    if(!m_resolver)
      m_resolver = new Resolver(getUlamTypeIdx(), m_state); //t41446

    m_resolver->setContextForPendingArgTypes(context);
  } //setContextForPendingArgTypes

  UTI SymbolClass::getContextForPendingArgTypes()
  {
    //SYMBOL_ASSERT(m_resolver); //dangerous! when template has default parameters
    if(m_resolver)
      return m_resolver->getContextForPendingArgTypes();

    SYMBOL_ASSERT(!isStub() || (getParentClassTemplate() && ((getParentClassTemplate()->getTotalParametersWithDefaultValues() > 0) || (getParentClassTemplate()->getUlamClass() == UC_UNSEEN))) || !m_state.isComplete(getUlamTypeIdx())); //e.g. not complete if bases have pending arg types (t3981)

    return Nouti; //getUlamTypeIdx(); //return self UTI, t3981
  } //getContextForPendingArgTypes

  bool SymbolClass::statusNonreadyClassArguments()
  {
    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver) //stubs only!
      return true;
    return m_resolver->statusNonreadyClassArguments(this);
  }

  u32 SymbolClass::countNonreadyClassArguments()
  {
    SYMBOL_ASSERT(!isStubCopy());
    if(!m_resolver)
      return 0; //nothing to do
    return m_resolver->countNonreadyClassArgs();
  }

  bool SymbolClass::mapUTItoUTI(UTI auti, UTI mappedUTI)
  {
    if(!m_resolver)
      m_resolver = new Resolver(getUlamTypeIdx(), m_state);

    return m_resolver->mapUTItoUTI(auti, mappedUTI);
  } //mapUTItoUTI

  bool SymbolClass::hasMappedUTI(UTI auti, UTI& mappedUTI)
  {
    bool rtnb = false;
    BaseclassWalker walker;

    //recursively check class and ancestors for auti
    UTI cuti = getUlamTypeIdx();
    walker.init(cuti);

    UTI baseuti = Nouti;
    while(!rtnb && walker.getNextBase(baseuti, m_state))
      {
	SymbolClass * basecsym = NULL;
	if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    rtnb = basecsym->resolveHasMappedUTI(auti, mappedUTI);

	    if(!rtnb)
	      walker.addAncestorsOf(basecsym);
	  }
      } //end while
    return rtnb;
  } //hasMappedUTI

  bool SymbolClass::hasMappedUTI(UTI auti)
  {
    return resolveHasMappedUTI(auti);
  }

  bool SymbolClass::resolveHasMappedUTI(UTI auti, UTI& mappedUTI)
  {
    bool rtnb = false;
    if(m_resolver)
      rtnb = m_resolver->findMappedUTI(auti, mappedUTI);
    return rtnb;
  } //(helper)

  bool SymbolClass::resolveHasMappedUTI(UTI auti)
  {
    UTI tmpmappedUTI; //helps with gdb debugging
    bool rtnb = false;
    if(m_resolver)
      rtnb = m_resolver->findMappedUTI(auti, tmpmappedUTI);
    return rtnb;
  } //(helper)

  /////////////////////////////////////////////////////////////////////////////////
  // from compileFiles in Compiler.cpp
  /////////////////////////////////////////////////////////////////////////////////
  bool SymbolClass::assignRegistryNumber(u32 n)
  {
    if (n == UNINITTED_REGISTRY_NUMBER)
      {
	std::ostringstream msg;
	msg << "Attempting to assign invalid Registry Number";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    if (m_registryNumber != UNINITTED_REGISTRY_NUMBER)
      {
	std::ostringstream msg;
	msg << "Attempting to assign duplicate Registry Number " << n;
	msg << " to " << m_registryNumber;
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    if(n >= MAX_REGISTRY_NUMBER)
      {
	std::ostringstream msg;
	msg << "Attempting to assign TOO MANY Registry Numbers " << n;
	msg << "; max table size is " << MAX_REGISTRY_NUMBER;
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    m_registryNumber = n;
    return true;
  } //assignRegistryNumber

  bool SymbolClass::assignElementType(ELE_TYPE n)
  {
    if (n == UNDEFINED_ELEMENT_TYPE)
      {
	std::ostringstream msg;
	msg << "Attempting to assign undefined Element Type";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    if (m_elementType != UNDEFINED_ELEMENT_TYPE)
      {
	std::ostringstream msg;
	msg << "Attempting to assign duplicate Element Type " << n;
	msg << " to " << m_elementType;
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    if(n >= MAX_ELEMENT_TYPE)
      {
	std::ostringstream msg;
	msg << "Attempting to assign TOO MANY Element Types " << n;
	msg << "; max table size is " << MAX_ELEMENT_TYPE + 1;
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    m_elementType = n;
    return true;
  } //assignElementType

  bool SymbolClass::assignEmptyElementType()
  {
    if (m_elementType != UNDEFINED_ELEMENT_TYPE)
      {
	std::ostringstream msg;
	msg << "Attempting to assign duplicate Empty Element Types";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }
    m_elementType = EMPTY_ELEMENT_TYPE;
    return true;
  } //assignEmptyElementType

  ELE_TYPE SymbolClass::getElementType()
  {
    UTI cuti = getUlamTypeIdx();
    if(m_elementType == UNDEFINED_ELEMENT_TYPE)
      {
	if(m_state.isEmptyElement(cuti))
	  assignEmptyElementType(); //t3802
	else
	  {
	    ELE_TYPE type = m_state.getNextElementType(cuti);
	    assignElementType(type);
	  }
      }
    return m_elementType;
  }

  u32 SymbolClass::getRegistryNumber()
  {
    if(m_registryNumber == UNINITTED_REGISTRY_NUMBER)
      {
	UTI suti = getUlamTypeIdx();
	if(m_state.okUTItoContinue(suti) && m_state.isComplete(suti))
	  {
	    u32 n = m_state.assignClassId(suti);
	    AssertBool rnset = assignRegistryNumber(n);
	    SYMBOL_ASSERT(rnset);
	  }
      }
    return m_registryNumber;
  }

  void SymbolClass::generateCode(FileManager * fm)
  {
    //class context already pushed..compilingThisIdx is us.
    SYMBOL_ASSERT(m_classBlock);

    ULAMCLASSTYPE classtype = getUlamClass();

    // setup for codeGen
    m_state.m_currentSelfSymbolForCodeGen = NULL; //self set by func defs
    m_state.clearCurrentObjSymbolsForCodeGen();

    m_state.setupCenterSiteForGenCode(); //temporary!!! (t3207, t3714)

    // mangled types and forward class declarations
    genMangledTypesHeaderFile(fm);

    // this class header
    {
      File * fp = fm->open(m_state.getFileNameForThisClassHeader(WSUBDIR).c_str(), WRITE);
      //testable by commenting out reportTooLongClassNamesAcrossTableOfClasses in Compiler.cpp
      // and running error test t3991
      if(!fp)
	{
	  std::ostringstream msg;
	  msg << "System failure: " << strerror(errno) << " to write <";
	  msg << m_state.getFileNameForThisClassHeader(WSUBDIR).c_str() << ">";
	  MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	  return;
	}

      generateHeaderPreamble(fp);
      genIfndefForHeaderFile(fp);
      generateHeaderIncludes(fp);

      UVPass uvpass;
      m_classBlock->genCode(fp, uvpass); //compileThisId only, class block

      // include this .tcc
      m_state.indent(fp);
      fp->write("#include \"");
      fp->write(m_state.getFileNameForThisClassBody().c_str());
      fp->write("\"");
      fp->write("\n");

      // include native .tcc for this class if any declared
      if(m_classBlock->countNativeFuncDecls() > 0)
	{
	  m_state.indent(fp);
	  fp->write("#include \"");
	  fp->write(m_state.getFileNameForThisClassBodyNative().c_str());
	  fp->write("\"");
	  fp->write("\n");
	}
      genEndifForHeaderFile(fp);

      delete fp; //close
    }

    // this class body
    {
      File * fp = fm->open(m_state.getFileNameForThisClassBody(WSUBDIR).c_str(), WRITE);
      if(!fp)
	{
	  std::ostringstream msg;
	  msg << "System failure: " << strerror(errno) << " to write <";
	  msg << m_state.getFileNameForThisClassBody(WSUBDIR).c_str() << ">";
	  MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	  return;
	}

      m_state.m_currentIndentLevel = 0;
      m_state.genCModeForHeaderFile(fp); //needed for .tcc files too

      UVPass uvpass;
      m_classBlock->genCodeBody(fp, uvpass); //compileThisId only, MFM namespace

      delete fp; //close
    }

    // "stub" .cpp includes .h (unlike the .tcc body)
    {
      File * fp = fm->open(m_state.getFileNameForThisClassCPP(WSUBDIR).c_str(), WRITE);
      if(!fp)
	{
	  std::ostringstream msg;
	  msg << "System failure: " << strerror(errno) << " to write <";
	  msg << m_state.getFileNameForThisClassCPP(WSUBDIR).c_str() << ">";
	  MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	  return;
	}

      m_state.m_currentIndentLevel = 0;

      // include .h in the .cpp
      m_state.indent(fp);
      fp->write("#include \"");
      fp->write(m_state.getFileNameForThisClassHeader().c_str());
      fp->write("\"");
      fp->write("\n");

      m_state.indent(fp);
      fp->write("namespace MFM{\n\n");
      fp->write("} //MFM\n\n");

      delete fp; //close
    }

    //separate main.cpp for elements only; that have the test method.
    if(classtype == UC_ELEMENT)
      {
	if(m_classBlock->findTestFunctionNode())
	  generateMain(fm);
      }
  } //generateCode

  void SymbolClass::generateAsOtherInclude(File * fp)
  {
    UTI suti = getUlamTypeIdx();
    //only if used by this class, or its superclass
    if(m_state.isOtherClassInThisContext(suti))
      {
	generateAllIncludesForTestMain(fp);
      }
  } //generateAsOtherInclude

  void SymbolClass::generateAllIncludesForTestMain(File * fp)
  {
    NodeBlockClass * classblock = getClassBlockNode();
    SYMBOL_ASSERT(classblock);
    classblock->generateIncludeTestMain(fp);
  } //generateAllIncludesForTestMain

  void SymbolClass::generateAsOtherForwardDef(File * fp)
  {
    UTI suti = getUlamTypeIdx();
    //only if used by This class, or its superclass
    if(m_state.isOtherClassInThisContext(suti))
      {
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	m_state.indent(fp);
	fp->write("namespace MFM { template ");
	fp->write("<class EC> "); //same for elements and quarks

	fp->write("struct ");
	fp->write(sut->getUlamTypeMangledName().c_str());
	fp->write("; }  //FORWARD\n");
      }
  } //generateAsOtherForwardDef

  void SymbolClass::generateTestInstance(File * fp, bool runtest)
  {
    NodeBlockClass * classblock = getClassBlockNode();
    SYMBOL_ASSERT(classblock);
    classblock->generateTestInstance(fp, runtest);
  } //generateTestInstance

  void SymbolClass::generateHeaderPreamble(File * fp)
  {
    m_state.m_currentIndentLevel = 0;
    m_state.genCModeForHeaderFile(fp);
    fp->write("/***********************         DO NOT EDIT        ******************************\n");
    fp->write("*\n");
    fp->write("* ");
    fp->write(m_state.m_pool.getDataAsString(m_state.getCompileThisId()).c_str());
    fp->write(".h - ");
    ULAMCLASSTYPE classtype = getUlamClass();
    if(classtype == UC_ELEMENT)
      fp->write("Element");
    else if(classtype == UC_QUARK)
      fp->write("Quark");
    else if(classtype == UC_TRANSIENT)
      fp->write("Transient");
    else if(classtype == UC_LOCALSFILESCOPE)
      fp->write("LocalsFilescope");
    else
      m_state.abortUndefinedUlamClassType();

    fp->write(" header for ULAM"); GCNL;

    m_state.genCopyrightAndLicenseForUlamHeader(fp);
  } //generateHeaderPreamble

  void SymbolClass::genIfndefForHeaderFile(File * fp)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(getUlamTypeIdx());
    m_state.indent(fp);
    fp->write("#ifndef ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("_H\n");

    m_state.indent(fp);
    fp->write("#define ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("_H\n\n");
  } //genIfndefForHeaderFile

  void SymbolClass::genEndifForHeaderFile(File * fp)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(getUlamTypeIdx());
    m_state.indent(fp);
    fp->write("#endif //");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("_H\n");
  }

  void SymbolClass::generateHeaderIncludes(File * fp)
  {
    m_state.indent(fp);
    fp->write("#include \"UlamDefs.h\"");
    fp->write("\n");

    //global user string pool (ulam-4)
    m_state.indent(fp);
    fp->write("#include \"");
    fp->write(m_state.getFileNameForUserStringPoolHeader().c_str());
    fp->write("\"\n");

    //using the _Types.h file
    m_state.indent(fp);
    fp->write("#include \"");
    fp->write(m_state.getFileNameForThisTypesHeader().c_str());
    fp->write("\"");
    fp->write("\n");

    if(getUlamClass() == UC_LOCALSFILESCOPE)
      return;

    //generate includes for all the other classes that have appeared
    m_state.m_programDefST.generateForwardDefsForTableOfClasses(fp);

    NodeBlockClass * classblock = getClassBlockNode();
    SYMBOL_ASSERT(classblock);
    UTI locuti = classblock->getLocalsFilescopeType();
    if(locuti != Nouti)
      {
	//locals filescope
	SYMBOL_ASSERT(m_state.okUTItoContinue(locuti));
	u32 mangledclassid = m_state.getMangledClassNameIdForUlamLocalsFilescope(locuti);

	m_state.indent(fp);
	fp->write("namespace MFM { template ");
	fp->write("<class EC> ");
	fp->write("struct ");
	fp->write(m_state.m_pool.getDataAsString(mangledclassid).c_str());
	fp->write("; }  //FORWARD"); GCNL;
      }
  } //generateHeaderIncludes

  // create structs with BV, as storage, and typedef
  // for primitive types; useful as args and local variables;
  // important for overloading functions
  void SymbolClass::genMangledTypesHeaderFile(FileManager * fm)
  {
    File * fp = fm->open(m_state.getFileNameForThisTypesHeader(WSUBDIR).c_str(), WRITE);
    //testable by commenting ouat reportTooLongClassNamesAcrossTableOfClasses in Compiler.cpp
    // and running error test t3991
    if(!fp)
      {
	std::ostringstream msg;
	msg << "System failure: " << strerror(errno) << " to write <";
	msg << m_state.getFileNameForThisTypesHeader(WSUBDIR).c_str() << ">";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return;
      }

    m_state.m_currentIndentLevel = 0;
    m_state.genCModeForHeaderFile(fp);

    m_state.indent(fp);
    //use -I ../../../include in g++ command
    fp->write("//#include \"itype.h\"\n");
    fp->write("//#include \"BitVector.h\"\n");
    fp->write("//#include \"BitField.h\"\n");
    fp->write("\n");

    m_state.indent(fp);
    fp->write("#include \"UlamDefs.h\"\n\n");

    NodeBlockClass * classblock = getClassBlockNode();
    SYMBOL_ASSERT(classblock);

    // define any immediate types needed for this class
    classblock->genUlamTypeImmediateDefinitions(fp);

    // define any model parameter immediate types needed for this class
    classblock->genModelParameterImmediateDefinitions(fp);

    delete fp; //close
  } //genMangledTypesHeaderFile

  // append main to .cpp for debug useage
  // outside the MFM namespace !!!
  void SymbolClass::generateMain(FileManager * fm)
  {
    File * fp = fm->open(m_state.getFileNameForThisClassMain(WSUBDIR).c_str(), WRITE);
    if(!fp)
      {
	std::ostringstream msg;
	msg << "System failure: " << strerror(errno) << " to write <";
	msg << m_state.getFileNameForThisClassMain(WSUBDIR).c_str() << ">";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return;
      }

    UTI suti = getUlamTypeIdx();

    m_state.m_currentIndentLevel = 0;

    m_state.indent(fp);
    fp->write("#include <stdio.h>\n");
    m_state.indent(fp);
    fp->write("#include <iostream>\n"); //to cout/cerr rtn
    m_state.indent(fp);
    fp->write("#include \"itype.h\"\n");
    m_state.indent(fp);
    fp->write("#include \"P3Atom.h\"\n");
    m_state.indent(fp);
    fp->write("#include \"SizedTile.h\"\n");
    m_state.indent(fp);
    fp->write("#include \"DebugTools.h\"\n"); //for LOG
    fp->write("\n");

    m_state.indent(fp);
    fp->write("#include \"UlamDefs.h\"\n\n");

    //include ALL the classes, including this class being compiled
    //t3373,4,5,6,7 t3380,82, t3394, 95, t3622, t3749, t3755, t3804,7, t3958, t41005, t41032,4
    m_state.m_programDefST.generateAllIncludesTestMainForTableOfClasses(fp);

    //include ALL the locals filescopes being compiled (t41345, t3981, t3865)
    m_state.generateAllIncludesTestMainForLocalsFilescopes(fp);

    fp->write("\n");
    m_state.indent(fp);
    fp->write("#ifndef NSEC_PER_SEC\n");
    m_state.indent(fp);
    fp->write("#define NSEC_PER_SEC 1000000000\n");
    m_state.indent(fp);
    fp->write("#endif //NSEC_PER_SEC "); GCNL;

    //namespace MFM
    fp->write("\n");
    m_state.indent(fp);
    fp->write("namespace MFM\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    //For all models
    m_state.indent(fp);
    fp->write("typedef P3Atom OurAtomAll;"); GCNL;
    m_state.indent(fp);
    fp->write("typedef Site<P3AtomConfig> OurSiteAll;"); GCNL;
    m_state.indent(fp);
    fp->write("typedef EventConfig<OurSiteAll,4> OurEventConfigAll;"); GCNL;
    m_state.indent(fp);
    fp->write("typedef SizedTile<OurEventConfigAll, 40, 24, 101> OurTestTile;"); GCNL;
    m_state.indent(fp);
    fp->write("typedef ElementTypeNumberMap<OurEventConfigAll> OurEventTypeNumberMapAll;"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("typedef ElementTable<OurEventConfigAll> TestElementTable;"); GCNL;
    m_state.indent(fp);
    fp->write("typedef EventWindow<OurEventConfigAll> TestEventWindow;"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("typedef UlamContextEvent<OurEventConfigAll> OurUlamContext;"); GCNL;
    fp->write("\n");

    // TEST TILE SETUP
    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("void TestTileSetup(OurTestTile& tile)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("OurEventTypeNumberMapAll etnm;"); GCNL;

    m_state.indent(fp);
    fp->write("Ue_10105Empty10<EC>::THE_INSTANCE.AllocateEmptyType();"); GCNL;
    m_state.indent(fp);
    fp->write("tile.ReplaceEmptyElement(Ue_10105Empty10<EC>::THE_INSTANCE);"); GCNL;

    //registers localsfilescope "classes" for string index corrections (e.g. t3952)
    m_state.generateTestInstancesForLocalsFilescopes(fp);

    //eventually ends up at SC::generateTestInstance()
    m_state.m_programDefST.generateTestInstancesForTableOfClasses(fp);

    m_state.indent(fp);
    fp->write("return;"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //TestTileSetup \n\n");

    //run test once only:
    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("int TestSingleElement()\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("OurTestTile tile;"); GCNL;
    m_state.indent(fp);
    fp->write("TestTileSetup<EC>(tile);"); GCNL;

    m_state.indent(fp);
    fp->write("OurUlamContext uc(tile.GetElementTable());"); GCNL;
    m_state.indent(fp);
    fp->write("const u32 TILE_WIDTH = tile.TILE_WIDTH;"); GCNL;
    m_state.indent(fp);
    fp->write("const u32 TILE_HEIGHT = tile.TILE_HEIGHT;"); GCNL;
    m_state.indent(fp);
    fp->write("SPoint center(TILE_WIDTH/2, TILE_HEIGHT/2);  // Hitting no caches, for starters;\n");
    m_state.indent(fp);
    fp->write("uc.SetTile(tile);"); GCNL;

    //eventually ends up at SC::generateTestInstance()
    m_state.m_programDefST.generateTestInstancesRunForTableOfClasses(fp);

    m_state.indent(fp);
    fp->write("return 0;"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //TestSingleElement\n\n");

    //run test performance in loop, return time in ms:
    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("int TestSingleElementPerformance(unsigned int loops)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("LOG.SetByteSink(STDERR);"); GCNL;
    m_state.indent(fp);
    fp->write("LOG.SetLevel(LOG.MESSAGE);"); GCNL;
    m_state.indent(fp);
    fp->write("OurTestTile tile;"); GCNL;
    m_state.indent(fp);
    fp->write("TestTileSetup<EC>(tile);"); GCNL;

    m_state.indent(fp);
    fp->write("OurUlamContext uc(tile.GetElementTable());"); GCNL;
    m_state.indent(fp);
    fp->write("const u32 TILE_WIDTH = tile.TILE_WIDTH;"); GCNL;
    m_state.indent(fp);
    fp->write("const u32 TILE_HEIGHT = tile.TILE_HEIGHT;"); GCNL;
    m_state.indent(fp);
    fp->write("SPoint center(TILE_WIDTH/2, TILE_HEIGHT/2);  // Hitting no caches, for starters;\n");
    m_state.indent(fp);
    fp->write("uc.SetTile(tile);"); GCNL;
    m_state.indent(fp);
    fp->write("TestEventWindow & ew = tile.GetEventWindow();"); GCNL;
    m_state.indent(fp);
    fp->write("OurAtomAll atom = "); //OurAtomAll
    fp->write(m_state.getTheInstanceMangledNameByIndex(suti).c_str());
    fp->write(".GetDefaultAtom();"); GCNL;
    m_state.indent(fp);
    fp->write("tile.PlaceAtom(atom, center);"); GCNL;

    fp->write("\n");
    m_state.indent(fp);
    fp->write("timespec startts;\n");
    m_state.indent(fp);
    fp->write("if(clock_gettime(CLOCK_REALTIME, &startts)) abort();"); GCNL;

    m_state.indent(fp);
    fp->write("while(loops-- > 0)");
    //eventually ends up at SC::generateTestInstance()
    //m_state.m_programDefST.generateTestInstancesRunForTableOfClasses(fp);
    m_state.indent(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("if(!ew.TryEventAtForProfiling(center)) abort();"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");

    m_state.indent(fp);
    fp->write("timespec endts;\n");
    m_state.indent(fp);
    fp->write("if(clock_gettime(CLOCK_REALTIME, &endts)) abort();"); GCNL;

    m_state.indent(fp);
    fp->write("long deltasecs = (endts.tv_sec - startts.tv_sec);\n");

    m_state.indent(fp);
    fp->write("if(deltasecs < 0) abort();\n");
    m_state.indent(fp);
    fp->write("if(deltasecs > 2)\n"); //more than 2 seconds, and nsec won't fit in 64-bits
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("std::cerr << loops << \" is too many loops\" << std::endl;\n");
    m_state.indent(fp);
    fp->write("abort();\n");
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");

    m_state.indent(fp);
    fp->write("long elapsedns = NSEC_PER_SEC * deltasecs + (endts.tv_nsec - startts.tv_nsec);"); GCNL;

    m_state.indent(fp);
    fp->write("return elapsedns;"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //TestSingleElementPerformance\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //MFM\n\n");

    //////////////////////////////////////////////
    //MAIN STARTS HERE !!!
    GCNL;

    m_state.indent(fp);
    fp->write("int main(int argc, const char** argv)\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("if(argc > 1)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("unsigned int loops = atoi(argv[1]);\n");
    m_state.indent(fp);
    fp->write("loops = (loops == 0 ? 1 : loops);\n");
    m_state.indent(fp);
    fp->write("unsigned int totalnanos = MFM::TestSingleElementPerformance<MFM::OurEventConfigAll>(loops);"); GCNL;
    m_state.indent(fp);
    fp->write("std::cerr << totalnanos/loops << \",\" << loops << std::endl;"); GCNL; //comma-delimited (no labels)
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");

    m_state.indent(fp);
    fp->write("else\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("MFM::TestSingleElement<MFM::OurEventConfigAll>();"); GCNL; //old way
    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("return 0;\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //main\n");

    delete fp; //close
  } //generateMain

  std::string SymbolClass::firstletterTolowercase(const std::string s) //static method
  {
    std::ostringstream up;
    assert(!s.empty());
    std::string c(1,(s[0] >= 'A' && s[0] <= 'Z') ? s[0]-('A'-'a') : s[0]);
    up << c << s.substr(1,s.length());
    return up.str();
  } //firstletterTolowercase

  void SymbolClass::addTargetDescriptionMapEntry(TargetMap& classtargets, u32 scid)
  {
    NodeBlockClass * classNode = getClassBlockNode();
    SYMBOL_ASSERT(classNode);
    classNode->addTargetDescriptionToInfoMap(classtargets, scid);
  } //addTargetDesciptionMapEntry

  void SymbolClass::addClassMemberDescriptionsMapEntry(ClassMemberMap& classmembers)
  {
    NodeBlockClass * classNode = getClassBlockNode();
    SYMBOL_ASSERT(classNode);
    classNode->addMemberDescriptionsToInfoMap(classmembers);
  } //addClassMemberDesciptionsMapEntry

  void SymbolClass::setConcreteClass()
  {
    m_concreteclass = true;
  }

  bool SymbolClass::isConcreteClass()
  {
    return m_concreteclass;
  }

  void SymbolClass::initVTable(s32 initialmax)
  {
    if(initialmax == UNKNOWNSIZE) return; //nothing to initialize
    SYMBOL_ASSERT(initialmax >= 0);
    if(m_vtableinitialized) return; //been here before

    u32 basesmaxes = 0;

    // get each ancestors' originating virtual function max index,
    // (entire tree, no dups) (t41312, t41303, t3602, t41325).
    BaseclassWalker walker;

    UTI cuti = getUlamTypeIdx();
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      walker.addAncestorsOf(csym);

    //ulam-5 supports multiple base classes; superclass optional;
    UTI baseuti;
    while(walker.getNextBase(baseuti, m_state))
      {
	SymbolClass * basecsym = NULL;
	if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    s32 vtsize = basecsym->getOrigVTableSize();
	    SYMBOL_ASSERT(vtsize >= 0); //caller made sure none unknown size

	    bool notdup = setVTstartoffsetOfRelatedOriginatingClass(baseuti, basesmaxes);

	    if(notdup)
	      {
		//copy baseclass' originating VTable entries
		for(s32 i = 0; i < vtsize; i++)
		  {
		    struct VTEntry ve = basecsym->getOrigVTableEntry(i);

		    std::vector<UTI> pTypes;
		    SYMBOL_ASSERT(ve.m_funcPtr);
		    ve.m_funcPtr->getVectorOfParameterTypes(pTypes);

		    // in case a virtual func is overridden by a subclass of baseuti
		    // in cuti ancestory, especially when not in cuti (e.g. t3600)
		    SymbolFunction * tmpfsym = NULL;
		    UTI foundInAncestor = Nouti;
		    if(m_state.findOverrideMatchingVirtualFunctionStrictlyByTypesInAncestorOf(cuti, ve.m_funcPtr->getId(), pTypes, true /* virtualInSub */, tmpfsym, foundInAncestor) && (foundInAncestor != baseuti) && m_state.isClassASubclassOf(foundInAncestor,baseuti))
		      {
			SYMBOL_ASSERT(tmpfsym);
			ve.m_ofClassUTI = foundInAncestor;
			ve.m_funcPtr = tmpfsym;
			ve.m_isPure = tmpfsym->isPureVirtualFunction();
		      }

		    m_vtable.push_back(ve); //not updateVTable
		    basesmaxes++;
		  }
	      }

	    walker.addAncestorsOf(basecsym); // check all bases
	  }
      } //end while

    SYMBOL_ASSERT(m_vtable.size() == (u32) initialmax); //t3639
    SYMBOL_ASSERT(basesmaxes == (u32) initialmax);

    setVTstartoffsetOfRelatedOriginatingClass(cuti, basesmaxes); //start of our originating virtual funcs
    m_vtableinitialized = true;
  } //initVTable

  void SymbolClass::updateVTable(u32 vidx, SymbolFunction * fsym, UTI kinuti, UTI origuti, bool isPure)
  {
    SYMBOL_ASSERT(m_state.okUTItoContinue(origuti));
    u32 startoffset = getVTstartoffsetOfRelatedOriginatingClass(origuti);
    SYMBOL_ASSERT((startoffset < UNRELIABLEPOS));
    u32 idx = startoffset + vidx;
    if(idx < m_vtable.size())
      {
	m_vtable[idx].m_funcPtr = fsym;
	m_vtable[idx].m_ofClassUTI = kinuti;
	SYMBOL_ASSERT(m_vtable[idx].m_origClassUTI == origuti);
	SYMBOL_ASSERT(m_vtable[idx].m_origvidx == vidx);
	m_vtable[idx].m_isPure = isPure;
      }
    else
      {
	SYMBOL_ASSERT(m_vtable.size() == idx); //sanity check, appends at next idx
	SYMBOL_ASSERT(m_vownedVT.size() == vidx); //sanity check, again
	struct VTEntry ve;
	ve.m_funcPtr = fsym;
	ve.m_ofClassUTI = kinuti;
	ve.m_origClassUTI = origuti;
	ve.m_origvidx = vidx;
	ve.m_isPure = isPure;
	m_vtable.push_back(ve);
	m_vownedVT.push_back(ve);
      }
  }//updateVTable

  s32 SymbolClass::getVTableSize()
  {
    if(m_vtableinitialized)
      return m_vtable.size();
    return UNKNOWNSIZE;
  }

  s32 SymbolClass::getOrigVTableSize()
  {
    if(m_vtableinitialized)
      return m_vownedVT.size();
    return UNKNOWNSIZE;
  }

  VT& SymbolClass::getVTableRef()
  {
    return m_vtable;
  }

  u32 SymbolClass::convertVTstartoffsetmap(std::map<u32, u32> & mapbyrnref)
  {
    u32 count = 0;
    BasesTableTypeMap::iterator it;
    for(it = m_basesVTstart.begin(); it != m_basesVTstart.end(); it++)
      {
	UTI nextuti = it->first;
	u32 startoffset = it->second;
	u32 regnum = m_state.getAClassRegistrationNumber(nextuti);
	std::pair<std::map<u32,u32>::iterator, bool> reti;
	reti = mapbyrnref.insert(std::pair<u32,u32>(regnum,startoffset));
	bool notdupi = reti.second; //false if already existed, i.e. not added
	SYMBOL_ASSERT(notdupi);  //sanity
	count++;
      }
    SYMBOL_ASSERT(count == m_basesVTstart.size()); //sanity
    SYMBOL_ASSERT(count == mapbyrnref.size()); //sanity
    return count;
  } //convertVTstartoffsetmap

  u32 SymbolClass::getVTstartoffsetOfRelatedOriginatingClass(UTI origuti)
  {
    SYMBOL_ASSERT(m_state.isARootUTI(origuti)); //t3652
    u32 startoffset = UNRELIABLEPOS;
    BasesTableTypeMap::iterator it = m_basesVTstart.find(origuti);
    if(it != m_basesVTstart.end())
      startoffset = it->second;
    return startoffset;
  }

  bool SymbolClass::setVTstartoffsetOfRelatedOriginatingClass(UTI origuti, u32 startoffset)
  {
    UTI rootuti = origuti;
    if(!m_state.isARootUTI(origuti))
      {
	AssertBool gotroot = m_state.findRootUTIAlias(origuti, rootuti); //t3652
	SYMBOL_ASSERT(gotroot); //note: not mapped in resolver
      }
    std::pair<BasesTableTypeMap::iterator, bool> reti;
    reti = m_basesVTstart.insert(std::pair<UTI,u32>(rootuti, startoffset)); //quick access
    //SYMBOL_ASSERT(reti.second);
    return reti.second;  //false if already existed (e.g. shared base), i.e. not added again
  }

  u32 SymbolClass::getVTableIndexForOriginatingClass(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    return m_vtable[idx].m_origvidx;
  }

  bool SymbolClass::isPureVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    return m_vtable[idx].m_isPure;
  }

  UTI SymbolClass::getClassForVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    return m_vtable[idx].m_ofClassUTI;
  }

  UTI SymbolClass::getOriginatingClassForVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    return m_vtable[idx].m_origClassUTI; //vowned
  }

  void SymbolClass::notePureFunctionSignatures()
  {
    u32 vtsize = m_vtable.size();
    for(u32 i = 0; i < vtsize; i++)
      {
	if(m_vtable[i].m_isPure)
	  notePureFunctionSignature(i);
      }
  } //notePureFunctionSignatures

  void SymbolClass::notePureFunctionSignature(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    SYMBOL_ASSERT(m_vtable[idx].m_isPure);

    Locator floc = m_vtable[idx].m_funcPtr->getLoc();
    std::ostringstream note;
    note << "Still pure in ";
    note << m_state.getUlamTypeNameBriefByIndex(getUlamTypeIdx()).c_str();
    note << ": " << m_vtable[idx].m_funcPtr->getFunctionNameWithTypes().c_str();
    MSG(m_state.getFullLocationAsString(floc).c_str(), note.str().c_str(), NOTE);
  } //notePureFunctionSignature

  std::string SymbolClass::getMangledFunctionNameForVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    return m_vtable[idx].m_funcPtr->getMangledName(); //need to cast overloaded-ness
  }

  std::string SymbolClass::getMangledFunctionNameWithTypesForVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    //SYMBOL_ASSERT(!m_vtable[idx].m_isPure); t3606,8,10, t3774,9, t3788, t3794,5, t3767 ..
    // enum VTABLE_IDX in .h needs func name, even when pure;
    return m_vtable[idx].m_funcPtr->getMangledNameWithTypes();
  }

  u32 SymbolClass::getVFuncIndexForVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    return m_vtable[idx].m_funcPtr->getVirtualMethodIdx();
  }

  u32 SymbolClass::getVFuncNameSignatureIdForVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    return m_vtable[idx].m_funcPtr->getUlamFunctionSignatureId();
  }

  struct VTEntry SymbolClass::getVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vtable.size());
    return m_vtable[idx];
  }

  struct VTEntry SymbolClass::getOrigVTableEntry(u32 idx)
  {
    SYMBOL_ASSERT(idx < m_vownedVT.size());
    return m_vownedVT[idx];
  }

  bool SymbolClass::isAbstract()
  {
    u32 vtsize = m_vtable.size();
    for(u32 i = 0; i < vtsize; i++)
      {
	if(m_vtable[i].m_isPure)
	  return true;
      }
    return false;
  }

  bool SymbolClass::checkAbstractClassError()
  {
    bool aok = true;
    bool isElement = (getUlamClass() == UC_ELEMENT);
    if((isElement || isConcreteClass()) && isAbstract())
      {
	UTI suti = getUlamTypeIdx();
	std::ostringstream msg;
	if(isElement)
	  msg << "Element '";
	else
	  msg << "Concrete Class '";
	msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	msg << "' is abstract due to these pure functions."; //dot dot
	MSG(m_state.getFullLocationAsString(getLoc()).c_str(), msg.str().c_str(), ERR);
	notePureFunctionSignatures();
	aok = false; //t41296,7
      }

    return aok;
  } //checkAbstractClassError

  void SymbolClass::buildIsBitVectorByRegNum(BV8K& bitvecref)
  {
    BaseclassWalker walker;

    //recursively sets the bit located at each ancestor's registration number;
    //gencoded is-method checks for relative by testing if the regnum bit is on.
    UTI cuti = getUlamTypeIdx();
    walker.init(cuti);

    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, m_state))
      {
	SymbolClass * basecsym = NULL;
	if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    u32 regid = basecsym->getRegistryNumber();
	    bitvecref.WriteBit(regid,true);

	    walker.addAncestorsOf(basecsym);
	  }
      } //end while
    return;
  } //buildIsBitVectorByRegNum

} //end MFM
