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
#include <iostream>
#include <inttypes.h>
#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <vector>
#include <list>
#include <utility>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <IfaceCellML_APISPEC.hxx>
#include <IfaceCCGS.hxx>
#include <CeVASBootstrap.hpp>
#include <MaLaESBootstrap.hpp>
#include <CCGSBootstrap.hpp>
#include <CellMLBootstrap.hpp>

#include "utils.hxx"
#include "version.hpp"

typedef std::pair<std::wstring,std::wstring> StringPair;
typedef std::vector<StringPair> StringPairList;
class ConnectionDescription
{
public:
  StringPair components;
  StringPairList variables;
};
typedef std::vector<ConnectionDescription> ConnectionList;
typedef std::vector< ObjRef<iface::cellml_api::CellMLVariable> > VariableList;
typedef std::vector< ObjRef<iface::cellml_api::Model> > ModelList;
typedef std::pair<std::wstring,
                  ObjRef<iface::cellml_api::CellMLVariable> > NameMap;
typedef std::vector<NameMap> NameMapList;
typedef std::vector< std::wstring > StringList;

char* wstring2string(const wchar_t* str)
{
  if (str)
  {
    size_t len = wcsrtombs(NULL,&str,0,NULL);
    if (len > 0)
    {
      len++;
      char* s = (char*)malloc(len);
      wcsrtombs(s,&str,len,NULL);
      return(s);
    }
  }
  return((char*)NULL);
}

bool variableInList(iface::cellml_api::CellMLVariable* v,
  const VariableList& list)
{
  VariableList::const_iterator i = list.begin();
  for (;i!=list.end();++i) if (v == *i) return true;
  return false;
}

bool stringInList(std::wstring& string,const StringList& list)
{
  StringList::const_iterator i = list.begin();
  for (;i!=list.end();++i) if (string == *i) return true;
  return false;
}

bool nameInMap(std::wstring& name,const NameMapList& list)
{
  NameMapList::const_iterator i = list.begin();
  for(;i!=list.end();++i) if (name == i->first) return true;
  return false;
}

void libxml2WriteXMLDocument(const char* file,const char* str)
{
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION;
  // first parse the string into an xmlDoc
  xmlDocPtr doc = xmlReadMemory(str,strlen(str),
    /*for use as xml:base*/"noname.xml",NULL,0);
  if (doc == NULL)
  {
    std::cerr << "ERROR parsing document string!" << std::endl;
  }
  // write the file to disk
  xmlSaveFormatFile(file,doc,1);
  // and free memory
  xmlFreeDoc(doc);
  /*
   * Cleanup function for the XML library.
   */
  xmlCleanupParser();
}

void fixupNamespaces(std::wstring& str)
{
  std::wstring one = L"http://www.cellml.org/cellml/1.0";
  std::wstring two = L"http://www.cellml.org/cellml/1.1";
  size_t pos = 0;
  while (true)
  {
    pos = str.find(one,pos);
    if (pos == std::wstring::npos) break;
    str.replace(pos,one.length(),two);
  }
}

/* for dumping an XML document string to a file */
void dumpDocumentString(std::wstring& dir,std::wstring& filename,
  std::wstring& str)
{
  static std::vector<std::wstring> files;
  wchar_t tmp[5];
  std::wstring file = dir + L"/" + filename + L".xml";
  int i=0;
  while (stringInList(file,files))
  {
    swprintf(tmp,5,L"%03d",++i);
    file = dir + L"/" + filename + L"_" + tmp + L".xml";
  }
  files.push_back(file);
  std::wcout << L"Writing to file: " << file << std::endl;

  /* FIXME: dodgy hack to get all the DOM nodes that we imported into the
            CellML 1.1 namespace */
  fixupNamespaces(str);
  // convert the wchar_t document string and file name into a char string
  // for use with libxml2
  char* cstr = wstring2string(str.c_str());
  char* cfilename = wstring2string(file.c_str());
  //and then write the XML file using libxml2
  libxml2WriteXMLDocument(cfilename,cstr);
  free(cstr);
  free(cfilename);
}

void addElement(iface::cellml_api::CellMLElement* parent,
  iface::cellml_api::CellMLElement* child)
{
  try
  {
    parent->addElement(child);
  }
  catch (iface::cellml_api::CellMLException& ce)
  {
    std::cerr << "Caught a CellMLException while trying to add an element"
              << std::endl;
  }
  catch (...)
  {
    // no need for this?
    std::cerr << "Unexpected error caught adding an element"
              << std::endl;
  }
}

class DecomposedModel
{
public:
  DecomposedModel(iface::cellml_api::CellMLBootstrap* cb,
    std::wstring& baseName,iface::cellml_services::CeVAS* cevas) :
    mCB(cb),
    mBCs(mCB->createModel(L"1.1")),
    mUnits(mCB->createModel(L"1.1")),
    mInterface(mCB->createModel(L"1.1")),
    mExperiment(mCB->createModel(L"1.1")),
    mCeVAS(cevas)
  {
    /*
     * create a model for storing all the boundary and initial conditions
     */
    std::wstring name = baseName + L"_variable_values_model";
    mBCs->name(name.c_str());
    // create a parameter component for BCs
    iface::cellml_api::CellMLComponent* c = mBCs->createComponent();
    name = L"parameters";
    c->name(name.c_str());
    addElement(mBCs,c);
    mParameters = c;
    // and a initial_value component for all the differential equations
    c = mBCs->createComponent();
    name = L"initial_values";
    c->name(name.c_str());
    addElement(mBCs,c);
    mInitialValues = c;
    /*
     * create a model for storing all the units defined in the model
     */
    name = baseName + L"_units_model";
    mUnits->name(name.c_str());
    /*
     * create a model in which we will define the interface to the entire
     * decomposed model
     */
    name = baseName + L"_interface_model";
    mInterface->name(name.c_str());
    c = mInterface->createComponent();
    mInterfaceComponentName = baseName + L"_interface_component";
    c->name(mInterfaceComponentName.c_str());
    addElement(mInterface,c);
    mInterfaceComponent = c;
    // create an encapsulation hierarchy
    RETURN_INTO_OBJREF(g,iface::cellml_api::Group,mInterface->createGroup());
    addElement(mInterface,g);
    RETURN_INTO_OBJREF(rr,iface::cellml_api::RelationshipRef,
      mInterface->createRelationshipRef());
    rr->setRelationshipName(L"",L"encapsulation");
    addElement(g,rr);
    RETURN_INTO_OBJREF(ref,iface::cellml_api::ComponentRef,
      mInterface->createComponentRef());
    ref->componentName(mInterfaceComponentName.c_str());
    addElement(g,ref);
    mEncapsInterface = ref;
    /*
     * create a model in which we will create an example experiment using the
     * decomposed model - this should reflect the original 1.0 model
     */
    name = baseName + L"_experiment_model";
    mExperiment->name(name.c_str());
    // add the import for the parameters and initial conditions
    RETURN_INTO_OBJREF(impBCs,iface::cellml_api::CellMLImport,
      mExperiment->createCellMLImport());
    RETURN_INTO_OBJREF(uri,iface::cellml_api::URI,impBCs->xlinkHref());
    std::wstring u = baseName + L"_variable_values_model.xml";
    uri->asText(u.c_str());
    addElement(mExperiment,impBCs);
    RETURN_INTO_OBJREF(impParametersC,iface::cellml_api::ImportComponent,
      mExperiment->createImportComponent());
    impParametersC->name(L"parameters");
    impParametersC->componentRef(L"parameters");
    addElement(impBCs,impParametersC);
    RETURN_INTO_OBJREF(impIVC,iface::cellml_api::ImportComponent,
      mExperiment->createImportComponent());
    impIVC->name(L"initial_values");
    impIVC->componentRef(L"initial_values");
    addElement(impBCs,impIVC);
    // and the import for the model interface
    RETURN_INTO_OBJREF(impInterface,iface::cellml_api::CellMLImport,
      mExperiment->createCellMLImport());
    RETURN_INTO_OBJREF(uri2,iface::cellml_api::URI,impInterface->xlinkHref());
    u = baseName + L"_interface_model.xml";
    uri2->asText(u.c_str());
    addElement(mExperiment,impInterface);
    RETURN_INTO_OBJREF(impInterfaceC,iface::cellml_api::ImportComponent,
      mExperiment->createImportComponent());
    impInterfaceC->name(mInterfaceComponentName.c_str());
    impInterfaceC->componentRef(mInterfaceComponentName.c_str());
    addElement(impInterface,impInterfaceC);
  }
  ~DecomposedModel()
  {
    /* nothing to do? */
  }
  /* dump out the decomposed model */
  void dump(std::wstring dir)
  {
    std::wstring str;
    std::wstring filename;
    GET_SET_WSTRING(mBCs->serialisedText(),str);
    GET_SET_WSTRING(mBCs->name(),filename);
    dumpDocumentString(dir,filename,str);
    GET_SET_WSTRING(mUnits->serialisedText(),str);
    GET_SET_WSTRING(mUnits->name(),filename);
    dumpDocumentString(dir,filename,str);
    GET_SET_WSTRING(mInterface->serialisedText(),str);
    GET_SET_WSTRING(mInterface->name(),filename);
    dumpDocumentString(dir,filename,str);
    GET_SET_WSTRING(mExperiment->serialisedText(),str);
    GET_SET_WSTRING(mExperiment->name(),filename);
    dumpDocumentString(dir,filename,str);
    ModelList::const_iterator i = mModels.begin();
    for (;i!=mModels.end();++i)
    {
      GET_SET_WSTRING((*i)->serialisedText(),str);
      GET_SET_WSTRING((*i)->name(),filename);
      dumpDocumentString(dir,filename,str);
    }
  }
  /* Create a new model for the given source component and add a clone of the
   * component to it
   */
  iface::cellml_api::CellMLComponent*
  addComponent(iface::cellml_api::CellMLComponent* src)
  {
    /* FIXME: we're assuming all component names are unique, which should be
              safe since we should be decomposing CellML 1.0 models...
    */
    // make the new model document
    RETURN_INTO_OBJREF(model,iface::cellml_api::Model,
      mCB->createModel(L"1.1"));
    RETURN_INTO_WSTRING(cname,src->name());
    std::wstring name = cname + L"_model";
    model->name(name.c_str());
    // and add it to the list of generated models
    mModels.push_back(model);
    // then create a component in the new model
    /****
      This don't work cause the parent model doesn't match the new model. But
      really need to iterate over the contents of the component in order to
      work out what units are required and sorting out the variables and
      stuff. Just need to find an easy way to copy over the math...
    RETURN_INTO_OBJREF(clone,iface::cellml_api::CellMLElement,
    src->clone(___deep___ true));
    addElement(model,clone);
    */
    iface::cellml_api::CellMLComponent* c = model->createComponent();
    c->name(cname.c_str());
    addElement(model,c);
    // and make an import for it in the interface component
    RETURN_INTO_OBJREF(imp,iface::cellml_api::CellMLImport,
      mInterface->createCellMLImport());
    RETURN_INTO_OBJREF(uri,iface::cellml_api::URI,imp->xlinkHref());
    // FIXME: assume files all in one directory and names unique
    std::wstring u = name + L".xml";
    uri->asText(u.c_str());
    addElement(mInterface,imp);
    // and add the component import statement
    RETURN_INTO_OBJREF(impC,iface::cellml_api::ImportComponent,
      mInterface->createImportComponent());
    impC->name(cname.c_str());
    impC->componentRef(cname.c_str());
    addElement(imp,impC);
    // and add the imported component to the encapsulation hierarchy
    RETURN_INTO_OBJREF(ref,iface::cellml_api::ComponentRef,
      mInterface->createComponentRef());
    ref->componentName(cname.c_str());
    addElement(mEncapsInterface,ref);
    return(c);
  }
  void storeConnection(ConnectionList& connections,
    std::wstring component_1,std::wstring variable_1,
    std::wstring component_2,std::wstring variable_2)
  {
    std::wstring v1 = L"";
    std::wstring v2 = L"";
    /* first look for existing connections in the stored list */
    ConnectionList::iterator i = connections.begin();
    for (;i!=connections.end();++i)
    {
      if ((i->components.first == component_1) &&
        (i->components.second == component_2))
      {
        v1 = variable_1;
        v2 = variable_2;
      }
      else if ((i->components.first == component_2) &&
        (i->components.second == component_1))
      {
        v1 = variable_2;
        v2 = variable_1;
      }
      if ((v1 != L"") && (v2 != L""))
      {
        StringPairList::const_iterator j = i->variables.begin();
        for (;j!=i->variables.end();++j)
        {
          if ((j->first == v1) && (j->second == v2))
          {
            /* nothing to do, connection already in list */
            return;
          }
        }
        /* connection between these two variables not found in list
           so add it */
        i->variables.push_back(StringPair(v1,v2));
        return;
      }
    }
    /* existing connection between components not found so make a new one */
    ConnectionDescription con;
    con.components = StringPair(component_1,component_2);
    con.variables.push_back(StringPair(variable_1,variable_2));
    connections.push_back(con);
    return;
  }
  void makeInterfaceConnections(iface::cellml_api::CellMLVariable* src)
  {
    RETURN_INTO_WSTRING(srcName,src->name());
    RETURN_INTO_WSTRING(srcCName,src->componentName());
    // grab all the connected variables
    RETURN_INTO_OBJREF(cvs,iface::cellml_services::ConnectedVariableSet,
      mCeVAS->findVariableSet(src));
    int i,l=(int)cvs->length();
    for (i=0;i<l;++i)
    {
      RETURN_INTO_OBJREF(v,iface::cellml_api::CellMLVariable,
        cvs->getVariable(i));
      RETURN_INTO_WSTRING(vname,v->name());
      RETURN_INTO_WSTRING(cname,v->componentName());
      storeConnection(mInterfaceConnections,mInterfaceComponentName,
        srcName,cname,vname);
    }
  }
  void makeInterfaceConnectionsIV(iface::cellml_api::CellMLVariable* src)
  {
    RETURN_INTO_WSTRING(srcName,src->name());
    RETURN_INTO_WSTRING(srcCName,src->componentName());
    /* store the connection to the source variable from the interface */
    storeConnection(mInterfaceConnections,mInterfaceComponentName,srcName,
      srcCName,srcName);
    /* and the initial value connection */
    std::wstring srcNameIV = srcName + L"_initial";
    storeConnection(mInterfaceConnections,mInterfaceComponentName,srcNameIV,
      srcCName,srcNameIV);
    /* and then all other connections between components? */
    // grab all the connected variables
    RETURN_INTO_OBJREF(cvs,iface::cellml_services::ConnectedVariableSet,
      mCeVAS->findVariableSet(src));
    int i,l=(int)cvs->length();
    for (i=0;i<l;++i)
    {
      RETURN_INTO_OBJREF(v,iface::cellml_api::CellMLVariable,
        cvs->getVariable(i));
      if (v != src)
      {
        RETURN_INTO_WSTRING(vname,v->name());
        RETURN_INTO_WSTRING(cname,v->componentName());
        storeConnection(mInterfaceConnections,srcCName,srcName,cname,vname);
      }
    }
  }
  void addParameterVariable(iface::cellml_api::CellMLVariable* src)
  {
    RETURN_INTO_WSTRING(name,src->name());
    RETURN_INTO_WSTRING(iv,src->initialValue());
    RETURN_INTO_WSTRING(units,src->unitsName());
    /* add the variable to the parameters component in the BCs model */
    RETURN_INTO_OBJREF(v,iface::cellml_api::CellMLVariable,
      mBCs->createCellMLVariable());
    v->name(name.c_str());
    v->initialValue(iv.c_str());
    v->unitsName(units.c_str());
    v->publicInterface(iface::cellml_api::INTERFACE_OUT);
    v->privateInterface(iface::cellml_api::INTERFACE_OUT);
    addElement(mParameters,v);
    /* add the variable to the interface component in the interface model */
    /* FIXME: this assumes model parameters are always uniquely named */
    RETURN_INTO_OBJREF(vInt,iface::cellml_api::CellMLVariable,
      mInterface->createCellMLVariable());
    vInt->name(name.c_str());
    vInt->unitsName(units.c_str());
    vInt->publicInterface(iface::cellml_api::INTERFACE_IN);
    vInt->privateInterface(iface::cellml_api::INTERFACE_OUT);
    addElement(mInterfaceComponent,vInt);
    /* and create any connections to anywhere the parameter is used */
    makeInterfaceConnections(src);
    /* add add the variable to the list of variables that will be connected
       in the example experiment */
    mExperimentParameters.push_back(name.c_str());
  }
  void addInitialValueVariable(iface::cellml_api::CellMLVariable* src)
  {
    RETURN_INTO_WSTRING(name,src->name());
    name += L"_initial";
    RETURN_INTO_WSTRING(iv,src->initialValue());
    RETURN_INTO_WSTRING(units,src->unitsName());
    /* add the variable to the initial_value component in the BCs model */
    RETURN_INTO_OBJREF(v,iface::cellml_api::CellMLVariable,
      mBCs->createCellMLVariable());
    v->name(name.c_str());
    v->initialValue(iv.c_str());
    v->unitsName(units.c_str());
    v->publicInterface(iface::cellml_api::INTERFACE_OUT);
    v->privateInterface(iface::cellml_api::INTERFACE_OUT);
    addElement(mInitialValues,v);
    /* add the variable to the interface component in the interface model */
    RETURN_INTO_OBJREF(vInt,iface::cellml_api::CellMLVariable,
      mInterface->createCellMLVariable());
    vInt->name(name.c_str());
    vInt->unitsName(units.c_str());
    vInt->publicInterface(iface::cellml_api::INTERFACE_IN);
    vInt->privateInterface(iface::cellml_api::INTERFACE_OUT);
    addElement(mInterfaceComponent,vInt);
    /* and create any connections to anywhere the variable is used */
    makeInterfaceConnectionsIV(src);
    /* add add the variable to the list of variables that will be connected
       in the example experiment */
    mExperimentInitialValues.push_back(name.c_str());
  }
  void addCalculatedVariable(iface::cellml_api::CellMLVariable* src)
  {
    /* FIXME: assuming the same variable is never going to be added more than
       once, probably ok since the source model should be valid...
    */
    RETURN_INTO_WSTRING(name,src->name());
    RETURN_INTO_WSTRING(srcCName,src->componentName());
    std::wstring localName = name;
    wchar_t tmp[5];
    int i=0;
    while (nameInMap(localName,mInterfaceNameMap))
    {
      swprintf(tmp,5,L"%03d",++i);
      localName = name + L"_" + tmp;
    }
    mInterfaceNameMap.push_back(NameMap(localName,src));
    RETURN_INTO_WSTRING(units,src->unitsName());
    /* add the variable to the interface component in the interface model */
    RETURN_INTO_OBJREF(vInt,iface::cellml_api::CellMLVariable,
      mInterface->createCellMLVariable());
    vInt->name(localName.c_str());
    vInt->unitsName(units.c_str());
    vInt->publicInterface(iface::cellml_api::INTERFACE_OUT);
    vInt->privateInterface(iface::cellml_api::INTERFACE_IN);
    addElement(mInterfaceComponent,vInt);
    /* store the connection to the source variable from the interface */
    storeConnection(mInterfaceConnections,mInterfaceComponentName,localName,
      srcCName,name);
    /* and add the connections to other components */
    // grab all the connected variables
    RETURN_INTO_OBJREF(cvs,iface::cellml_services::ConnectedVariableSet,
      mCeVAS->findVariableSet(src));
    int l=(int)cvs->length();
    for (i=0;i<l;++i)
    {
      RETURN_INTO_OBJREF(v,iface::cellml_api::CellMLVariable,
        cvs->getVariable(i));
      if (v != src)
      {
        RETURN_INTO_WSTRING(vname,v->name());
        RETURN_INTO_WSTRING(cname,v->componentName());
        storeConnection(mInterfaceConnections,srcCName,name,cname,vname);
      }
    }
  }
  void addBoundVariable(iface::cellml_api::CellMLVariable* src)
  {
    /* we only want to add the source bound variable, not all the occurances */
    RETURN_INTO_OBJREF(sv,iface::cellml_api::CellMLVariable,
      src->sourceVariable());
    RETURN_INTO_WSTRING(name,src->name());
    RETURN_INTO_WSTRING(srcCName,src->componentName());
    RETURN_INTO_WSTRING(svname,sv->name());
    RETURN_INTO_WSTRING(svCName,sv->componentName());
    RETURN_INTO_WSTRING(units,sv->unitsName());
    if (src == sv)
    {
      /* add the source variable to the interface component in the interface
         model */
      RETURN_INTO_OBJREF(vInt,iface::cellml_api::CellMLVariable,
        mInterface->createCellMLVariable());
      vInt->name(svname.c_str());
      vInt->unitsName(units.c_str());
      vInt->publicInterface(iface::cellml_api::INTERFACE_NONE);
      vInt->privateInterface(iface::cellml_api::INTERFACE_OUT);
      addElement(mInterfaceComponent,vInt);
    }
    /* store the connection to the source variable from the interface */
    storeConnection(mInterfaceConnections,mInterfaceComponentName,svname,
      srcCName,name);
  }
  void createConnection(iface::cellml_api::Model* model,
    const ConnectionDescription& desc)
  {
    RETURN_INTO_OBJREF(con,iface::cellml_api::Connection,
      model->createConnection());
    addElement(model,con);
    RETURN_INTO_OBJREF(mc,iface::cellml_api::MapComponents,
      con->componentMapping());
    mc->firstComponentName(desc.components.first.c_str());
    mc->secondComponentName(desc.components.second.c_str());
    StringPairList::const_iterator i = desc.variables.begin();
    for (;i!=desc.variables.end();++i)
    {
      RETURN_INTO_OBJREF(mv,iface::cellml_api::MapVariables,
        model->createMapVariables());
      mv->firstVariableName(i->first.c_str());
      mv->secondVariableName(i->second.c_str());
      addElement(con,mv);
    }
  }
  void createConnections()
  {
    ConnectionList::const_iterator i = mInterfaceConnections.begin();
    for (;i!=mInterfaceConnections.end();++i)
    {
      createConnection(mInterface,*i);
    }
    /* make the connection description for the example experiment model */
    // first parameters
    ConnectionDescription cd;
    cd.components = StringPair(mInterfaceComponentName,L"parameters");
    StringList::const_iterator p = mExperimentParameters.begin();
    for (;p!=mExperimentParameters.end();++p)
    {
      cd.variables.push_back(StringPair(*p,*p));
    }
    createConnection(mExperiment,cd);
    // and then the initial values
    cd.components = StringPair(mInterfaceComponentName,L"initial_values");
    cd.variables.clear();
    p = mExperimentInitialValues.begin();
    for (;p!=mExperimentInitialValues.end();++p)
    {
      cd.variables.push_back(StringPair(*p,*p));
    }
    createConnection(mExperiment,cd);
  }
  void addUnits(iface::cellml_api::Units* src)
  {
    /* save the units name for the later imports */
    RETURN_INTO_WSTRING(name,src->name());
    if (!stringInList(name,mUnitsNames)) mUnitsNames.push_back(name);
    /* add a copy of the units element into the units model using straight
       dom methods */
    DECLARE_QUERY_INTERFACE(modelCDE,mUnits,cellml_api::CellMLDOMElement);
    RETURN_INTO_OBJREF(modelElement,iface::dom::Element,
      modelCDE->domElement());
    RETURN_INTO_OBJREF(domDoc,iface::dom::Document,
      modelElement->ownerDocument());
    DECLARE_QUERY_INTERFACE(srcCDE,src,cellml_api::CellMLDOMElement);
    RETURN_INTO_OBJREF(srcElement,iface::dom::Element,srcCDE->domElement());
    RETURN_INTO_OBJREF(importedNode,iface::dom::Node,
      domDoc->importNode(srcElement,/*deep*/true));
    modelElement->appendChild(importedNode);
  }
  void createUnitsImportsForModel(iface::cellml_api::Model* model)
  {
    // work out the uri for the units model
    RETURN_INTO_WSTRING(unitsFile,mUnits->name());
    unitsFile += L".xml";
    // create the model import to import the units model
    RETURN_INTO_OBJREF(imp,iface::cellml_api::CellMLImport,
      model->createCellMLImport());
    RETURN_INTO_OBJREF(uri,iface::cellml_api::URI,imp->xlinkHref());
    uri->asText(unitsFile.c_str());
    addElement(model,imp);
    // and then add a units import for all units in the list
    StringList::const_iterator i = mUnitsNames.begin();
    for (;i!=mUnitsNames.end();++i)
    {
      RETURN_INTO_OBJREF(impU,iface::cellml_api::ImportUnits,
        model->createImportUnits());
      impU->name(i->c_str());
      impU->unitsRef(i->c_str());
      addElement(imp,impU);
    }
  }
  void createUnitsImports()
  {
    /* add the full set of units to all models */
    createUnitsImportsForModel(mInterface);
    createUnitsImportsForModel(mBCs);
    ModelList::const_iterator i = mModels.begin();
    for (;i!=mModels.end();++i)
    {
      createUnitsImportsForModel(*i);
    }
  }
private:
  ObjRef<iface::cellml_api::CellMLBootstrap> mCB;
  ObjRef<iface::cellml_api::Model> mBCs;
  ObjRef<iface::cellml_api::CellMLComponent> mParameters;
  ObjRef<iface::cellml_api::CellMLComponent> mInitialValues;
  ObjRef<iface::cellml_api::Model> mUnits;
  ObjRef<iface::cellml_api::Model> mInterface;
  ObjRef<iface::cellml_api::CellMLComponent> mInterfaceComponent;
  std::wstring mInterfaceComponentName;
  ObjRef<iface::cellml_api::ComponentRef> mEncapsInterface;
  ObjRef<iface::cellml_api::Model> mExperiment;
  StringList mExperimentParameters;
  StringList mExperimentInitialValues;
  ModelList mModels;
  NameMapList mInterfaceNameMap;
  NameMapList mVOINameMap;
  ObjRef<iface::cellml_services::CeVAS> mCeVAS;
  ConnectionList mInterfaceConnections;
  StringList mUnitsNames;
};

int main(int argc,char** argv)
{
  std::string versionString = getVersion();
  std::cout << versionString << std::endl;
  // Get the URL from which to load the model...
  if (argc < 3)
  {
    printf("Usage: %s modelURL outputDir\n",argv[0]);
    return -1;
  }

  wchar_t* URL;
  size_t l = strlen(argv[1]);
  URL = new wchar_t[l + 1];
  memset(URL, 0, (l + 1) * sizeof(wchar_t));
  const char* mbrurl = argv[1];
  mbsrtowcs(URL, &mbrurl, l, NULL);

  wchar_t* baseDir;
  l = strlen(argv[2]);
  baseDir = new wchar_t[l + 1];
  memset(baseDir, 0, (l + 1) * sizeof(wchar_t));
  const char* mbrBaseDir = argv[2];
  mbsrtowcs(baseDir, &mbrBaseDir, l, NULL);

  RETURN_INTO_OBJREF(cb,iface::cellml_api::CellMLBootstrap,
    CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(ml,iface::cellml_api::ModelLoader,cb->modelLoader());
  iface::cellml_api::Model* mod;
  try
  {
    mod = ml->loadFromURL(URL);
    mod->fullyInstantiateImports(); // just in case
  }
  catch (...)
  {
    printf("Error loading model URL.\n");
    delete [] URL;
    return -1;
  }
  delete [] URL;

  // create a CeVAS so we can navigate variable connections
  RETURN_INTO_OBJREF(cbs,iface::cellml_services::CeVASBootstrap,
    CreateCeVASBootstrap());
  RETURN_INTO_OBJREF(cevas,iface::cellml_services::CeVAS,
    cbs->createCeVASForModel(mod));

  /*
   * create the object to hold the decomposed model documents
   */
  RETURN_INTO_WSTRING(modelName,mod->name());
  DecomposedModel* dm = new DecomposedModel(cb,modelName,cevas);
  
  // we need to create a list of state variables so we can distinguish initial
  // conditions from model parameters ??? FIXME: really? 
  VariableList stateVariables;
  VariableList boundVariables;
  RETURN_INTO_OBJREF(cgb,iface::cellml_services::CodeGeneratorBootstrap,
    CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg,iface::cellml_services::CodeGenerator,
    cgb->createCodeGenerator());
  cg->useCeVAS(cevas);
  try
  {
    RETURN_INTO_OBJREF(cci,iface::cellml_services::CodeInformation,
      cg->generateCode(mod));
    RETURN_INTO_OBJREF(cti,iface::cellml_services::ComputationTargetIterator,
      cci->iterateTargets());
    while(true)
    {
      RETURN_INTO_OBJREF(ct,iface::cellml_services::ComputationTarget,
        cti->nextComputationTarget());
      if (ct == NULL) break;
      if (ct->type() == iface::cellml_services::STATE_VARIABLE)
      {
        RETURN_INTO_OBJREF(v,iface::cellml_api::CellMLVariable,ct->variable());
        stateVariables.push_back(v);
      }
      if (ct->type() == iface::cellml_services::VARIABLE_OF_INTEGRATION)
      {
        RETURN_INTO_OBJREF(v,iface::cellml_api::CellMLVariable,ct->variable());
        boundVariables.push_back(v);
      }
    }
  }
  catch (iface::cellml_api::CellMLException& ce)
  {
    printf("Caught a CellMLException while generating code.\n");
    mod->release_ref();
    return -1;
  }
  catch (...)
  {
    printf("Unexpected exception calling generateCode!\n");
    // this is a leak, but it should also never happen :)
    return -1;
  }

  // iterate over all relevant components in the model
  RETURN_INTO_OBJREF(ci,iface::cellml_api::CellMLComponentIterator,
    cevas->iterateRelevantComponents());
  while(true)
  {
    RETURN_INTO_OBJREF(c,iface::cellml_api::CellMLComponent,
      ci->nextComponent());
    if (c == NULL) break;
    // create the component's own model and component within that model
    RETURN_INTO_OBJREF(nc,iface::cellml_api::CellMLComponent,
      dm->addComponent(c));
    RETURN_INTO_OBJREF(ncModel,iface::cellml_api::Model,nc->modelElement());
    // iterate over all variables in the component
    RETURN_INTO_OBJREF(vs,iface::cellml_api::CellMLVariableSet,c->variables());
    RETURN_INTO_OBJREF(vsi,iface::cellml_api::CellMLVariableIterator,
      vs->iterateVariables());
    while (true)
    {
      RETURN_INTO_OBJREF(v,iface::cellml_api::CellMLVariable,
        vsi->nextVariable());
      if (v == NULL) break;
      RETURN_INTO_WSTRING(vname,v->name());
      RETURN_INTO_WSTRING(vunits,v->unitsName());
      // grab the source variable
      RETURN_INTO_OBJREF(sv,iface::cellml_api::CellMLVariable,
        v->sourceVariable());
      if (variableInList(v,boundVariables) ||
        variableInList(sv,boundVariables))
      {
        /* we have a variable of integration special case
           create the variable in the new component but ensure it gets
           connected directly to the interface component.
           FIXME: ignoring any initial value attribute that might be specified.
         */
        RETURN_INTO_OBJREF(nv,iface::cellml_api::CellMLVariable,
          ncModel->createCellMLVariable());
        nv->name(vname.c_str());
        nv->publicInterface(iface::cellml_api::INTERFACE_IN);
        nv->privateInterface(iface::cellml_api::INTERFACE_OUT);
        nv->unitsName(vunits.c_str());
        addElement(nc,nv);
        dm->addBoundVariable(v);
      }
      else if (v == sv)
      {
        // check for an initial value on the variable
        RETURN_INTO_WSTRING(iv,v->initialValue());
        if (iv != L"")
        {
          if (variableInList(v,stateVariables))
          {
            /* we have a state variable, so add its initial value to the BC
               model and add the initial value variable and the original state
               variable to the new component */
            std::wstring ivName = vname + L"_initial";
            RETURN_INTO_OBJREF(nv,iface::cellml_api::CellMLVariable,
              ncModel->createCellMLVariable());
            nv->name(vname.c_str());
            nv->publicInterface(iface::cellml_api::INTERFACE_OUT);
            nv->privateInterface(iface::cellml_api::INTERFACE_OUT);
            nv->unitsName(vunits.c_str());
            nv->initialValue(ivName.c_str());
            addElement(nc,nv);
            dm->addCalculatedVariable(v);
            RETURN_INTO_OBJREF(niv,iface::cellml_api::CellMLVariable,
              ncModel->createCellMLVariable());
            niv->name(ivName.c_str());
            niv->publicInterface(iface::cellml_api::INTERFACE_IN);
            niv->privateInterface(iface::cellml_api::INTERFACE_NONE);
            niv->unitsName(vunits.c_str());
            addElement(nc,niv);
            dm->addInitialValueVariable(v);
          }
          else
          {
            /* we have a parameter (FIXME: do we?) so add it to the BC model
               and the new component without the initial value */
            RETURN_INTO_OBJREF(nv,iface::cellml_api::CellMLVariable,
              ncModel->createCellMLVariable());
            nv->name(vname.c_str());
            nv->publicInterface(iface::cellml_api::INTERFACE_IN);
            nv->privateInterface(iface::cellml_api::INTERFACE_OUT);
            nv->unitsName(vunits.c_str());
            addElement(nc,nv);
            dm->addParameterVariable(v);
          }
        }
        else
        {
          /* we have a locally computed variable so add it straight in */
          RETURN_INTO_OBJREF(nv,iface::cellml_api::CellMLVariable,
            ncModel->createCellMLVariable());
          nv->name(vname.c_str());
          nv->publicInterface(iface::cellml_api::INTERFACE_OUT);
          nv->privateInterface(iface::cellml_api::INTERFACE_OUT);
          nv->unitsName(vunits.c_str());
          addElement(nc,nv);
          /* FIXME: variables with locally defined units probably shouldn't be
             exposed, and if they are then the units need to be bubbled up
             also. */
          RETURN_INTO_OBJREF(unitsSet,iface::cellml_api::UnitsSet,c->units());
          RETURN_INTO_OBJREF(units,iface::cellml_api::Units,
            unitsSet->getUnits(vunits.c_str()));
          if (units == NULL) dm->addCalculatedVariable(v);
        }
      }
      else
      {
        /* FIXME: a variable coming from somewhere else? */
        RETURN_INTO_OBJREF(nv,iface::cellml_api::CellMLVariable,
          ncModel->createCellMLVariable());
        nv->name(vname.c_str());
        nv->publicInterface(iface::cellml_api::INTERFACE_IN);
        nv->privateInterface(iface::cellml_api::INTERFACE_OUT);
        nv->unitsName(vunits.c_str());
        addElement(nc,nv);
      }
    }
    /*
     * Now add all the math in the component to the new component
     */
    RETURN_INTO_OBJREF(math,iface::cellml_api::MathList,c->math());
    RETURN_INTO_OBJREF(mathIt,iface::cellml_api::MathMLElementIterator,
      math->iterate());
    while (true)
    {
      RETURN_INTO_OBJREF(m,iface::mathml_dom::MathMLElement,mathIt->next());
      if (m == NULL) break;
      // shift to working in the DOM
      DECLARE_QUERY_INTERFACE(componentDE,nc,cellml_api::CellMLDOMElement);
      RETURN_INTO_OBJREF(componentElement,iface::dom::Element,
        componentDE->domElement());
      RETURN_INTO_OBJREF(domDoc,iface::dom::Document,
        componentElement->ownerDocument());
      RETURN_INTO_OBJREF(importedMathNode,iface::dom::Node,
        domDoc->importNode(m,/*deep*/true));
      componentElement->appendChild(importedMathNode);
    }
    /*
     * and any locally defined units
     */
    RETURN_INTO_OBJREF(units,iface::cellml_api::UnitsSet,c->units());
    RETURN_INTO_OBJREF(ui,iface::cellml_api::UnitsIterator,
      units->iterateUnits());
    while (true)
    {
      RETURN_INTO_OBJREF(u,iface::cellml_api::Units,ui->nextUnits());
      if (u == NULL) break;
      // shift to working in the DOM
      // get the dom element for the old units element
      DECLARE_QUERY_INTERFACE(unitsDE,u,cellml_api::CellMLDOMElement);
      RETURN_INTO_OBJREF(uElement,iface::dom::Element,unitsDE->domElement());
      // and the dom element of the new component
      DECLARE_QUERY_INTERFACE(componentDE,nc,cellml_api::CellMLDOMElement);
      RETURN_INTO_OBJREF(componentElement,iface::dom::Element,
        componentDE->domElement());
      // the dom document of the new component
      RETURN_INTO_OBJREF(domDoc,iface::dom::Document,
        componentElement->ownerDocument());
      // and import the old units node into the new dom document
      RETURN_INTO_OBJREF(importedNode,iface::dom::Node,
        domDoc->importNode(uElement,/*deep*/true));
      // finally, append the units node to the new component's child list
      componentElement->appendChild(importedNode);
    }
  } // component loop

  /*
   * now sort out the units
   */
  // first add all the units the units model
  RETURN_INTO_OBJREF(units,iface::cellml_api::UnitsSet,mod->allUnits());
  RETURN_INTO_OBJREF(unitsI,iface::cellml_api::UnitsIterator,
    units->iterateUnits());
  while (true)
  {
    RETURN_INTO_OBJREF(u,iface::cellml_api::Units,unitsI->nextUnits());
    if (u == NULL) break;
    dm->addUnits(u);
  }
  // and then create all the units imports
  dm->createUnitsImports();

  /* instantiate all the connections */
  dm->createConnections();

  dm->dump(baseDir);
  delete [] baseDir;
  delete dm;
  
  mod->release_ref();

  return 0;
}
