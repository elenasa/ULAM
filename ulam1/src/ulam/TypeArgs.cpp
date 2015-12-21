#include "TypeArgs.h"

namespace MFM {

  TypeArgs::TypeArgs() {}

  TypeArgs::TypeArgs(const TypeArgs& tref) :
      m_typeTok(tref.m_typeTok),
      m_bitsize(tref.m_bitsize),
      m_arraysize(tref.m_arraysize),
      m_classInstanceIdx(tref.m_classInstanceIdx),
      m_anothertduti(tref.m_anothertduti),
      m_declListOrTypedefScalarType(tref.m_declListOrTypedefScalarType),
      m_assignOK(tref.m_assignOK),
      m_isStmt(tref.m_isStmt),
      m_declRef(tref.m_declRef)
    {}

  TypeArgs::~TypeArgs() {}

  void TypeArgs::init(Token typetoken)
  {
    m_typeTok = typetoken;
    m_bitsize = UNKNOWNSIZE;
    m_arraysize = NONARRAYSIZE;
    m_classInstanceIdx = Nav;
    m_anothertduti = Nav;
    m_declListOrTypedefScalarType = Nav;
    m_assignOK = true;
    m_isStmt = true;
    m_declRef = ALT_NOT;
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
    return *this;
  }

} //MFM
