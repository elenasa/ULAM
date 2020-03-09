namespace MFM {

//! ClassUtils.ulam:52:   Bool isBase(ClassId classId, ClassId baseId) native;
  template<class EC>

  Ui_Ut_10111b<EC> Uq_10109210ClassUtils10<EC>::Uf_6isBase(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_7classId, Ui_Ut_102321u<EC>& Uv_6baseId) const //native
  { 
    Ui_Ut_10111b<EC> rtn;

    u32 classId = Uv_7classId.read();
    u32 baseId = Uv_6baseId.read();

    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    const UlamClass<EC>* theClass = ucr.GetUlamClassOrNullByIndex(classId);
    const UlamClass<EC>* theBase = ucr.GetUlamClassOrNullByIndex(baseId);

    if (theClass && theBase && theClass->internalCMethodImplementingIs(theBase))
      rtn.write(1);

    return rtn;
  } //Uf_6isBase


  template<class EC>
  Ui_Ut_10111b<EC> Uq_10109210ClassUtils10<EC>::Uf_9212isDirectBase(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_7classId, Ui_Ut_102321u<EC>& Uv_9212directBaseId) const
  {
    u32 classId = Uv_7classId.read();
    u32 baseId = Uv_9212directBaseId.read();

    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    const UlamClass<EC>* theClass = ucr.GetUlamClassOrNullByIndex(classId);

    Ui_Ut_10111b<EC> rtn;
    u32 directBases = theClass->GetDirectBaseClassCount();
    for (u32 idx = 1; idx < directBases; ++idx) {
      const UlamClass<EC>* aBase = theClass->GetOrderedBaseClassAsUlamClass(idx);
      u32 aBaseId = aBase->GetRegistrationNumber();
      if (aBaseId == baseId) {
        rtn.write(1);
        break;
      }
    }
    return rtn;
  }


  template<class EC>
  Ui_Ut_102321u<EC> Uq_10109210ClassUtils10<EC>::Uf_9217getBaseClassCount(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_7classId) const
  {
    u32 classId = Uv_7classId.read();
    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    const UlamClass<EC>* theClass = ucr.GetUlamClassOrNullByIndex(classId);
    MFM_API_ASSERT_NONNULL(theClass);
    Ui_Ut_102321u<EC> ret(theClass->GetBaseClassCount()); 
    return ret;
  } // Uf_9217getBaseClassCount

    template<class EC>
  Ui_Ut_102321u<EC> Uq_10109210ClassUtils10<EC>::Uf_9223getDirectBaseClassCount(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_7classId) const
  {
    u32 classId = Uv_7classId.read();
    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    const UlamClass<EC>* theClass = ucr.GetUlamClassOrNullByIndex(classId);
    MFM_API_ASSERT_NONNULL(theClass);
    Ui_Ut_102321u<EC> ret(theClass->GetDirectBaseClassCount()); 
    return ret;
  } // Uf_9217getBaseClassCount

  template<class EC>
  Ui_Ut_102321u<EC> Uq_10109210ClassUtils10<EC>::Uf_9210getClassId(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_r10106UrSelf10<EC>& Ur_2ur) const
  {
    Ui_Ut_102321u<EC> rtn(U32_MAX); // U32_MAX is the illegal classid value
    const UlamClass<EC> * effSelf = Ur_2ur.GetEffectiveSelf();
    if (effSelf != NULL)
      rtn.write(effSelf->GetRegistrationNumber());
    return rtn;
  } // Uf_9210getClassId

  template<class EC>
  Ui_Ut_102321u<EC> Uq_10109210ClassUtils10<EC>::Uf_9210getClassId(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_r102961a<EC>& Ur_4atom) const
  {
    u32 atomType = Ur_4atom.GetType();
    Ui_Ut_102321u<EC> rtn; 
    const Element<EC> * eltptr = uc.LookupElementTypeFromContext(atomType);
    if (eltptr) {
      const UlamElement<EC> * ue = eltptr->AsUlamElement();
      if (ue) {
        rtn.write(ue->GetRegistrationNumber());
      }
    }
    return rtn;


//! ClassUtils.ulam:54:   ClassId getClassId(Atom& atom) { return 0; }
    const u32 Uh_5tlreg3126 = 0; //gcnl:NodeTerminal.cpp:657
    const u32 Uh_5tlreg3127 = _Int32ToUnsigned32(Uh_5tlreg3126, 2, 32); //gcnl:NodeCast.cpp:756
    Ui_Ut_102321u<EC> Uh_5tlval3128(Uh_5tlreg3127); //gcnl:Node.cpp:1466
    return (Uh_5tlval3128); //gcnl:NodeReturnStatement.cpp:392
  } // Uf_9210getClassId


//! ClassUtils.ulam:59:   ClassId getBaseClassId(ClassId ur, Unsigned baseIdx) { return 0; }
  template<class EC>
  Ui_Ut_102321u<EC> Uq_10109210ClassUtils10<EC>::Uf_9214getBaseClassId(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_2id, Ui_Ut_102321u<EC>& Uv_7baseIdx) const
  {
    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    u32 classId = Uv_2id.read();
    const UlamClass<EC> * ulamClass = ucr.GetUlamClassOrNullByIndex(classId);
    MFM_API_ASSERT_NONNULL(ulamClass);

    u32 baseIdx = Uv_7baseIdx.read();
    u32 baseClassCount = ulamClass->GetBaseClassCount();

    Ui_Ut_102321u<EC> rtn;  // default to zero (illegal class id) return
    if (baseIdx < baseClassCount) {
      const UlamClass<EC> * baseClass = ulamClass->GetOrderedBaseClassAsUlamClass(baseIdx);
      MFM_API_ASSERT_NONNULL(baseClass);
      rtn.write(baseClass->GetRegistrationNumber());
    }

    return rtn;
  } // Uf_9214getBaseClassId

  template<class EC>
  Ui_Ut_102321s<EC> Uq_10109210ClassUtils10<EC>::Uf_9219getMangledClassName(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& ci) const
  {
    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    u32 classId = ci.read();
    const UlamClass<EC> * ulamClass = ucr.GetUlamClassOrNullByIndex(classId);
    MFM_API_ASSERT_NONNULL(ulamClass);

    u32 idx = ulamClass->GetMangledClassNameAsStringIndex();
    Ui_Ut_102321s<EC> ret(idx);

    return ret;
  } // Uf_9219getMangledClassName

  template<class EC>
  Ui_Ut_102321s<EC> Uq_10109210ClassUtils10<EC>::Uf_9212getClassName(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& ci, Ui_Ut_10111b<EC>& tmplParms, Ui_Ut_10111b<EC>& tmplVals) const

  {
    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    u32 classId = ci.read();
    const UlamClass<EC> * ulamClass = ucr.GetUlamClassOrNullByIndex(classId);
    MFM_API_ASSERT_NONNULL(ulamClass);

    bool parms = _Bool32ToCbool(tmplParms.read(), 1);
    bool vals = _Bool32ToCbool(tmplVals.read(), 1);

    u32 idx = ulamClass->GetUlamClassNameAsStringIndex(parms,vals);

    Ui_Ut_102321s<EC> ret(idx);
    return ret;
  }


#if 0
//! ClassUtils.ulam:62:   Bool getClassNamePretty(ClassId classId, ByteStreamWriter & to) { return false; }
  template<class EC>
  Ui_Ut_10111b<EC> Uq_10109210ClassUtils10<EC>::Uf_9218getClassNamePretty(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_7classId, Ui_Uq_r10109216ByteStreamWriter10<EC>& Ur_2to) const
  {

    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    u32 classId = Uv_7classId.read();
    const UlamClass<EC> * ulamClass = ucr.GetUlamClassOrNullByIndex(classId);
    Ui_Ut_10111b<EC> rtn;
    if (ulamClass) {
      const char * mangledName = ulamClass->GetMangledClassName();
      UlamTypeInfo utin;
      if (!utin.InitFrom(mangledName))
        FAIL(ILLEGAL_STATE);
      OString128 className;
      utin.PrintPretty(className,false);

      const char * name = className.GetZString();

      VfuncPtr writebyteptr;
      // Build the appropriate ref for a virtual function call
      UlamRef<EC> vfuw(Ur_2to,                                                       // The ref we're calling a virtual function on
                       Uq_10109216ByteStreamWriter10<EC>::VOWNED_IDX_Uf_919writeByte1110181u, // The index of the vfunc we're calling
                       Uq_10109216ByteStreamWriter10<EC>::THE_INSTANCE,              // The class that originated that vfunc
                       writebyteptr);                                                // Where to stick the resulting vfuncptr

      for (u8 ch = *name; ch; ch = *++name) {
        Ui_Ut_10181u<EC> Uv_2ch(ch);
        ((typename Uq_10109216ByteStreamWriter10<EC>::Uf_919writeByte1110181u) (writebyteptr)) (uc, vfuw, Uv_2ch); // Make the call
      }

      rtn.write(1);
    }
    return rtn;
  } // Uf_9218getClassNamePretty
#endif

#if 0
//! ClassUtils.ulam:54:   ClassId getBaseClassId(UrSelf& ur, Unsigned baseIdx) native;
  template<class EC>
  Ui_Ut_102321u<EC> Uq_10109210ClassUtils10<EC>::Uf_9214getBaseClassId(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_r10106UrSelf10<EC>& Ur_2ur, Ui_Ut_102321u<EC>& Uv_7baseIdx) const //native
  { 
    const UlamClassRegistry<EC> & ucr = uc.GetUlamClassRegistry();
    const UlamClass<EC> * effSelf = Ur_2ur.GetEffectiveSelf();
    MFM_API_ASSERT_NONNULL(effSelf);
    u32 baseClassCount = effSelf->GetBaseClassCount();
    u32 baseIdx = Uv_7baseIdx.read();
    Ui_Ut_102321u<EC> rtn;  // default to zero (illegal class id) return

    if (baseIdx < baseClassCount) {
      const UlamClass<EC> * baseClass = effSelf->GetOrderedBaseClassAsUlamClass(baseIdx);
      MFM_API_ASSERT_NONNULL(baseClass);
      rtn.write(baseClass->GetRegistrationNumber());
    }

    return rtn;
  } //Uf_9214getBaseClassId 
#endif

  template<class EC>
  Ui_Ut_10111b<EC> Uq_10109210ClassUtils10<EC>::Uf_9212getClassInfo(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_5idnum, Ui_Un_r1047136919ClassInfo10<EC>& Ur_4info) const //native
  { 
    Ui_Ut_10111b<EC> rtn;
    return rtn;
  } //Uf_9212getClassInfo 

}
