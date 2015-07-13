/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is decompose.
 *
 * The Initial Developer of the Original Code is
 * David Nickerson <nickerso@users.sourceforge.net>.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef _UTILS_HXX_
#define _UTILS_HXX_

#define RETURN_INTO_WSTRING(lhs, rhs) \
  wchar_t* tmp_##lhs = rhs;           \
  std::wstring lhs;                   \
  if (tmp_##lhs)                      \
  {                                   \
    lhs = std::wstring(tmp_##lhs);    \
    free(tmp_##lhs);                  \
  }                                 

#define RETURN_INTO_STRING(lhs, rhs)  \
  char* tmp_##lhs = rhs;              \
  std::string lhs;                    \
  if (tmp_##lhs)                      \
  {                                   \
    lhs = std::string(tmp_##lhs);     \
    free(tmp_##lhs);                  \
  }

#define GET_SET_WSTRING( GET , SET )  \
  {                                   \
    wchar_t* _str = GET;              \
    if (_str)                         \
    {                                 \
      SET = std::wstring(_str);       \
      free(_str);                     \
    }                                 \
  }

#define GET_SET_URI( GET , SET )          \
  {                                       \
    iface::cellml_api::URI* uri = GET;    \
    if (uri)                              \
    {                                     \
      GET_SET_WSTRING(uri->asText(),SET); \
      uri->release_ref();                 \
    }                                     \
  }

#define QUERY_INTERFACE(lhs, rhs, type)                               \
  if (rhs != NULL)                                                    \
  {                                                                   \
    void* _qicast_obj = rhs->query_interface(#type);                  \
    if (_qicast_obj != NULL)                                          \
    {                                                                 \
      lhs = already_AddRefd<iface::type>(reinterpret_cast<iface::type*>(_qicast_obj)); \
    }                                                                   \
    else                                                                \
      lhs = NULL;                                                       \
  }                                                                     \
  else lhs = NULL;

#define QUERY_INTERFACE_REPLACE(lhs, rhs, type) \
  QUERY_INTERFACE(lhs, rhs, type);              \
  if (rhs != NULL)                              \
  {                                             \
    rhs->release_ref();                         \
    rhs = NULL;                                 \
  }

#define DECLARE_QUERY_INTERFACE(lhs, rhs, type) \
  iface::type* lhs;                             \
  QUERY_INTERFACE(lhs, rhs, type)

#define DECLARE_QUERY_INTERFACE_REPLACE(lhs, rhs, type) \
  iface::type* lhs;                                     \
  QUERY_INTERFACE_REPLACE(lhs, rhs, type)

#define CELLML_TO_VARIABLE_INTERFACE( cellml, variable)                 \
  if (cellml == iface::cellml_api::INTERFACE_IN) variable(Variable::IN); \
  else if (cellml == iface::cellml_api::INTERFACE_OUT)                  \
    variable(Variable::OUT);                                              \
  else if (cellml == iface::cellml_api::INTERFACE_NONE)                 \
    variable(Variable::NONE);                                             \
  else ERROR("CELLML_TO_VARIABLE_INTERFACE","Invalid variable interface\n")

template<class T>
class already_AddRefd
{
public:
  already_AddRefd(T* aPtr)
    : mPtr(aPtr)
  {
  }

  ~already_AddRefd()
  {
  }

  operator T*() const
  {
    return mPtr;
  }

  T* getPointer() const
  {
    return mPtr;
  }
private:
  T* mPtr;
};

template<class T>
class ObjRef
{
public:
  ObjRef()
    : mPtr(NULL)
  {
  }

  ObjRef(const ObjRef<T>& aPtr)
  {
    mPtr = aPtr.getPointer();
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  ObjRef(T* aPtr)
    : mPtr(aPtr)
  {
    mPtr->add_ref();
  }

  ObjRef(const already_AddRefd<T> aar)
  {
    mPtr = aar.getPointer();
  }

  ~ObjRef()
  {
    if (mPtr != NULL)
      mPtr->release_ref();
  }

  T* operator-> () const
  {
    return mPtr;
  }

  T* getPointer() const
  {
    return mPtr;
  }

  operator T* () const
  {
    return mPtr;
  }

  void operator= (T* newAssign)
  {
    if (mPtr == newAssign)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = newAssign;
    if (newAssign != NULL)
      mPtr->add_ref();
  }

  // We need these explicit forms or the default overloads the templates below.
  void operator= (const already_AddRefd<T>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  void operator= (const ObjRef<T>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  template<class U>
  void operator= (const already_AddRefd<U>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  template<class U>
  void operator= (const ObjRef<U>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    if (mPtr != NULL)
      mPtr->add_ref();
  }

private:
  T* mPtr;
};

template<class T, class U> bool
operator==(const ObjRef<T>& lhs, const ObjRef<U>& rhs)
{
  return (lhs.getPointer() == rhs.getPointer());
}

template<class T, class U> bool
operator!=(const ObjRef<T>& lhs, const ObjRef<U>& rhs)
{
  return (lhs.getPointer() != rhs.getPointer());
}

#define RETURN_INTO_OBJREF(lhs, type, rhs)  \
  ObjRef<type> lhs                          \
  (                                         \
    already_AddRefd<type>                   \
    (                                       \
      static_cast<type*>                    \
      (                                     \
        rhs                                 \
      )                                     \
    )                                       \
  )

#define RETURN_INTO_OBJREFD(lhs, type, rhs) \
  ObjRef<type> lhs \
  ( \
    already_AddRefd<type> \
    ( \
      dynamic_cast<type*> \
      ( \
        rhs \
      ) \
    )\
  )

#endif /* _UTILS_HXX_ */
