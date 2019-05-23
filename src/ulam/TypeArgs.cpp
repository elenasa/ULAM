#include "TypeArgs.h"

namespace MFM {

  TypeArgs::TypeArgs() : m_bitsize(UNKNOWNSIZE), m_arraysize(NONARRAYSIZE), m_classInstanceIdx(Nouti), m_anothertduti(Nouti), m_declListOrTypedefScalarType(Nouti), m_assignOK(true), m_isStmt(true), m_declRef(ALT_NOT), m_referencedUTI(Nouti), m_hasConstantTypeModifier(false), m_forMemberSelect(false) {}

  TypeArgs::TypeArgs(const TypeArgs& tref) :
    m_typeTok(tref.m_typeTok),
    m_bitsize(tref.m_bitsize),
    m_arraysize(tref.m_arraysize),
    m_classInstanceIdx(tref.m_classInstanceIdx),
    m_anothertduti(tref.m_anothertduti),
    m_declListOrTypedefScalarType(tref.m_declListOrTypedefScalarType),
    m_assignOK(tref.m_assignOK),
    m_isStmt(tref.m_isStmt),
    m_declRef(tref.m_declRef),
    m_referencedUTI(tref.m_referencedUTI),
    m_hasConstantTypeModifier(tref.m_hasConstantTypeModifier),
    m_forMemberSelect(tref.m_forMemberSelect)
  {}

  TypeArgs::~TypeArgs() {}

  void TypeArgs::init(const Token& typetoken)
  {
    m_typeTok = typetoken;
  }

  TypeArgs& TypeArgs::operator=(const TypeArgs& tref)
  {
    m_typeTok = tref.m_typeTok;
    m_bitsize = tref.m_bitsize;
    m_arraysize = tref.m_arraysize;
    m_classInstanceIdx = tref.m_classInstanceIdx;
    m_anothertduti = tref.m_anothertduti;
    m_declListOrTypedefScalarType = tref.m_declListOrTypedefScalarType;
    m_assignOK = tref.m_assignOK;
    m_isStmt = tref.m_isStmt;
    m_declRef = tref.m_declRef;
    m_referencedUTI = tref.m_referencedUTI;
    m_hasConstantTypeModifier = tref.m_hasConstantTypeModifier;
    m_forMemberSelect = tref.m_forMemberSelect;
    return *this;
  }

  void TypeArgs::setdeclref(const Token& ftoken, UTI referencedType)
  {
    switch(ftoken.m_type)
      {
      case TOK_KW_AS:
	m_declRef = ALT_AS;
	m_referencedUTI = referencedType;
	break;
      case TOK_AMP:
	m_declRef = (m_hasConstantTypeModifier ? ALT_CONSTREF : ALT_REF);
	m_referencedUTI = referencedType;
	break;
      case TOK_KW_IS:
      default:
	m_declRef = ALT_NOT;
	break;
      };
    return;
  }//setdelref
} //MFM
