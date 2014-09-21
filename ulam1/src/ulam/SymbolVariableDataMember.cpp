#include "SymbolVariableDataMember.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableDataMember::SymbolVariableDataMember(u32 id, UTI utype, PACKFIT packed, u32 slot, CompilerState& state) : SymbolVariable(id, utype, packed, state), m_dataMemberUnpackedSlotIndex(slot)
  {
    setDataMember();
  }


  SymbolVariableDataMember::~SymbolVariableDataMember()
  {
    //   m_static.clearAllocatedMemory(); //clean up arrays,etc.
  }


  u32  SymbolVariableDataMember::getDataMemberUnpackedSlotIndex()
  {
    return m_dataMemberUnpackedSlotIndex;
  }


  s32 SymbolVariableDataMember::getBaseArrayIndex()
  {
    return (s32) getDataMemberUnpackedSlotIndex();
  }


  const std::string SymbolVariableDataMember::getMangledPrefix()
  {
    return "Um_"; 
  }


  // replaced by NodeVarDecl:genCode to leverage the declaration order preserved by the parse tree.
  void SymbolVariableDataMember::generateCodedVariableDeclarations(File * fp, ULAMCLASSTYPE classtype)
  {
    UTI vuti = getUlamTypeIdx(); 
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    m_state.indent(fp);
    fp->write(vut->getUlamTypeMangledName(&m_state).c_str()); //for C++
    
    if(vclasstype == UC_QUARK)       // called on classtype elements only
      {
	fp->write("<");
	fp->write_decimal(getPosOffset());
	fp->write(">");
      }    
    fp->write(" ");
    fp->write(getMangledName().c_str());

#if 0
    s32 arraysize = vut->getArraySize();
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
#endif
    fp->write(";\n");  
  }


  // replaces NodeVarDecl:printPostfix to learn the values of Class' storage in center site
  void SymbolVariableDataMember::printPostfixValuesOfVariableDeclarations(File * fp, ULAMCLASSTYPE classtype)
  {
    UTI vuti = getUlamTypeIdx(); 
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(vuti).c_str()); //short type name
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(getId()).c_str());

    s32 arraysize = m_state.getArraySize(vuti);
    //scalar has 'size=1'; empty array [0] is still '0'.
    s32 size = (arraysize > NONARRAYSIZE ? arraysize : 1); 

    PACKFIT packFit = m_state.determinePackable(vuti);
    assert(WritePacked(packFit));   //has to be to fit in an atom/site; 

    char * valstr = new char[size * 8 + 32];

    if(size > 0)
      {
	//simplifying assumption for testing purposes: center site
	Coord c0(0,0);
	u32 slot = c0.convertCoordToIndex();
	
	//build the string of values (for both scalar and packed array)
	UlamValue arrayPtr = UlamValue::makePtr(slot, EVENTWINDOW, vuti, packFit, m_state, ATOMFIRSTSTATEBITPOS + getPosOffset());
	UlamValue nextPtr = UlamValue::makeScalarPtr(arrayPtr, m_state);
	
	UlamValue atval = m_state.getPtrTarget(nextPtr);
	u32 data = atval.getDataFromAtom(nextPtr, m_state);
	vut->getDataAsString(data, valstr, 'z', m_state); //'z' -> no preceeding ','
	
	for(s32 i = 1; i < size; i++)
	  {
	    char tmpstr[8];
	    nextPtr.incrementPtr(m_state);
	    atval = m_state.getPtrTarget(nextPtr);
	    data = atval.getDataFromAtom(nextPtr, m_state);
	    vut->getDataAsString(data, tmpstr, ',', m_state);
	    strcat(valstr,tmpstr);
	  }
      } //end arrays > 0, and scalar
    else
      {
	sprintf(valstr," ");
      }

    //output the arraysize (optional), and results
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }

    fp->write("(");
    fp->write(valstr);  //results out here!
    fp->write("); ");

    delete [] valstr;
  }

} //end MFM
