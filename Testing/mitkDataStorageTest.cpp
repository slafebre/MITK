/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <fstream>
#include <algorithm>

#include "mitkImage.h"
#include "mitkSurface.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"
#include "mitkGroupTagProperty.h"
#include "mitkDataTreeNode.h"
#include "mitkReferenceCountWatcher.h"
#include "mitkDataTreeHelper.h"

#include "mitkDataStorage.h"
#include "mitkDataTreeStorage.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNOT.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateOR.h"
#include "mitkNodePredicateSource.h"
#include "mitkMessage.h"

#include "mitkTestingMacros.h"


void TestDataStorage(mitk::DataStorage* ds);
void TestDataTreeStorage(mitk::DataTreeStorage* ds, mitk::DataTree* tree);


class DSEventReceiver // Helper class for event testing
{
public:
  const mitk::DataTreeNode* m_NodeAdded;
  const mitk::DataTreeNode* m_NodeRemoved;

  DSEventReceiver()
    : m_NodeAdded(NULL), m_NodeRemoved(NULL)
  {
  }

  void OnAdd(const mitk::DataTreeNode* node)
  {
    m_NodeAdded = node;
  }

  void OnRemove(const mitk::DataTreeNode* node)
  {
    m_NodeRemoved = node;
  }
};


//## Documentation
//## main testing method
//## NOTE: the current Singleton implementation of DataTreeStorage will lead to crashes if a testcase fails 
//##       and therefore mitk::DataStorage::ShutdownSingleton() is not called.
int mitkDataStorageTest(int /*argc*/, char* /*argv[]*/)
{
  MITK_TEST_BEGIN("DataStorageTest");

  /* Create DataTreeStorage */
  MITK_TEST_OUTPUT( << "Testing DataTreeStorage : ");
  try
  {
    mitk::DataTreeStorage::Pointer dts;
    mitk::DataTree::Pointer tree = mitk::DataTree::New();
    try
    {
      dts = dynamic_cast<mitk::DataTreeStorage*>(mitk::DataStorage::CreateInstance(tree));
      MITK_TEST_CONDITION_REQUIRED(dts.IsNotNull(), "Creating DataTreeStorage");
    }
    catch (...)
    {
  	  MITK_TEST_FAILED_MSG( << "Exception during creation of DataTreeStorage");
    }  

    MITK_TEST_OUTPUT( << "Testing DataTreeStorage in 'Only manage added nodes' mode.");
    dts->SetManageCompleteTree(false);
    TestDataStorage(dts); // test the data storage in both operation modes
    /* Cleanup, get new ds */
    tree = NULL;
    dts = NULL;
    mitk::DataStorage::ShutdownSingleton();
    tree = mitk::DataTree::New();
    dts = dynamic_cast<mitk::DataTreeStorage*>(mitk::DataStorage::CreateInstance(tree));

    MITK_TEST_OUTPUT( << "Specific tests for DataTreeStorage");
    TestDataTreeStorage(dts, tree);
    
    /* Cleanup */
    dts = NULL;
    mitk::DataStorage::ShutdownSingleton();
  }
  catch (...)
  {
    mitk::DataStorage::ShutdownSingleton(); 
  }

  /* Create StandaloneDataStorage */
  MITK_TEST_OUTPUT( << "Create StandaloneDataStorage : ");
  mitk::StandaloneDataStorage::Pointer sds;
  try
  {
    sds = mitk::StandaloneDataStorage::New();
    MITK_TEST_CONDITION_REQUIRED(sds.IsNotNull(), "Testing Instatiation");
  }
  catch (...)
  {
    MITK_TEST_FAILED_MSG( << "Exception during creation of StandaloneDataStorage");
  }  
  
  MITK_TEST_OUTPUT( << "Testing DataTreeStorage: ");
  TestDataStorage(sds);
  // TODO: Add specific StandaloneDataStorage Tests here
  sds = NULL;

  MITK_TEST_END();
}

//##Documentation
//## @brief Test for the DataStorage class and its associated classes (e.g. the predicate classes)
//## This method will be called once for each subclass of DataStorage
void TestDataStorage( mitk::DataStorage* ds )
{
  /* DataStorage valid? */
  MITK_TEST_CONDITION_REQUIRED(ds != NULL, "DataStorage valid?");

  // create some DataTreeNodes to fill the ds
  mitk::DataTreeNode::Pointer n1 = mitk::DataTreeNode::New();   // node with image and name property
  mitk::Image::Pointer image = mitk::Image::New();
  unsigned int imageDimensions[] = { 10, 10, 10 };
  mitk::PixelType pt(typeid(int));
  image->Initialize( pt, 3, imageDimensions );
  n1->SetData(image);
  n1->SetProperty("name", mitk::StringProperty::New("Node 1 - Image Node"));
  mitk::DataStorage::SetOfObjects::Pointer parents1 = mitk::DataStorage::SetOfObjects::New();

  mitk::DataTreeNode::Pointer n2 = mitk::DataTreeNode::New();   // node with surface and name and color properties
  mitk::Surface::Pointer surface = mitk::Surface::New();
  n2->SetData(surface);
  n2->SetProperty("name", mitk::StringProperty::New("Node 2 - Surface Node"));
  mitk::Color color;  color.Set(1.0f, 0.0f, 0.0f);
  n2->SetColor(color);
  n2->SetProperty("Resection Proposal 1", mitk::GroupTagProperty::New());
  mitk::DataStorage::SetOfObjects::Pointer parents2 = mitk::DataStorage::SetOfObjects::New();
  parents2->InsertElement(0, n1);  // n1 (image node) is source of n2 (surface node)

  mitk::DataTreeNode::Pointer n3 = mitk::DataTreeNode::New();   // node without data but with name property
  n3->SetProperty("name", mitk::StringProperty::New("Node 3 - Empty Node"));
  n3->SetProperty("Resection Proposal 1", mitk::GroupTagProperty::New());
  n3->SetProperty("Resection Proposal 2", mitk::GroupTagProperty::New());
  mitk::DataStorage::SetOfObjects::Pointer parents3 = mitk::DataStorage::SetOfObjects::New();
  parents3->InsertElement(0, n2);  // n2 is source of n3 

  mitk::DataTreeNode::Pointer n4 = mitk::DataTreeNode::New();   // node without data but with color property
  n4->SetColor(color);
  n4->SetProperty("Resection Proposal 2", mitk::GroupTagProperty::New());
  mitk::DataStorage::SetOfObjects::Pointer parents4 = mitk::DataStorage::SetOfObjects::New();
  parents4->InsertElement(0, n2); 
  parents4->InsertElement(1, n3);  // n2 and n3 are sources of n4 

  mitk::DataTreeNode::Pointer n5 = mitk::DataTreeNode::New();   // extra node
  n5->SetProperty("name", mitk::StringProperty::New("Node 5"));

  try /* adding objects */
  {
    /* Add an object */
    ds->Add(n1, parents1);
    MITK_TEST_CONDITION_REQUIRED((ds->GetAll()->Size() == 1) && (ds->GetAll()->GetElement(0) == n1), "Testing Adding a new object");

    /* Check exception on adding the same object again */
    MITK_TEST_OUTPUT( << "Check exception on adding the same object again: ");
    MITK_TEST_FOR_EXCEPTION(..., ds->Add(n1, parents1)); 
    MITK_TEST_CONDITION(ds->GetAll()->Size() == 1, "Test if object count is correct after exception");

    /* Add an object that has a source object */
    ds->Add(n2, parents2);
    MITK_TEST_CONDITION_REQUIRED(ds->GetAll()->Size() == 2, "Testing Adding an object that has a source object");

    /* Add some more objects needed for further tests */
    ds->Add(n3, parents3);   // n3 object that has name property and one parent
    ds->Add(n4, parents4);   // n4 object that has color property
    ds->Add(n5);             // n5 has no parents
    MITK_TEST_CONDITION_REQUIRED(ds->GetAll()->Size() == 5, "Adding some more objects needed for further tests");
  } 
  catch(...)
  {
    MITK_TEST_FAILED_MSG( << "Exeption during object creation");
  }

  try  /* object retrieval methods */
  { 
  /* Requesting all Objects */
    {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetAll();
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();

    MITK_TEST_CONDITION(
         (stlAll.size() == 5)  // check if all tree nodes are in resultset
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n5) != stlAll.end()),
      "Testing GetAll()"      
      );
    }
    /* Requesting a named object */
    {
    mitk::NodePredicateProperty predicate("name", mitk::StringProperty::New("Node 2 - Surface Node"));
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n2), "Requesting a named object"); 
    }
  
  /* Requesting objects of specific data type */
    {   
    mitk::NodePredicateDataType predicate("Image");
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n1), "Requesting objects of specific data type")
  }
  /* Requesting objects of specific dimension */
    {
    mitk::NodePredicateDimension predicate( 3 );
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n1), "Requesting objects of specific dimension")
  }
  /* Requesting objects with specific data object */
    {   
  mitk::NodePredicateData predicate(image);
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n1), "Requesting objects with specific data object")
  }
  /* Requesting objects with NULL data */
    {
    mitk::NodePredicateData predicate(NULL);
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    MITK_TEST_CONDITION(
         (all->Size() == 3)
      && (std::find(all->begin(), all->end(), n3) != all->end()) 
      && (std::find(all->begin(), all->end(), n4) != all->end())
      && (std::find(all->begin(), all->end(), n5) != all->end())
      , "Requesting objects with NULL data");
  }
  /* Requesting objects that meet a conjunction criteria */
    {
    mitk::NodePredicateDataType p1("Surface");
    mitk::NodePredicateProperty p2("color", mitk::ColorProperty::New(color));
    mitk::NodePredicateAND predicate;
    predicate.AddPredicate(p1);
    predicate.AddPredicate(p2);  // objects must be of datatype "Surface" and have red color (= n2)
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n2), "Requesting objects that meet a conjunction criteria");
    }
  /* Requesting objects that meet a disjunction criteria */
  {
    mitk::NodePredicateDataType p1("Image");
    mitk::NodePredicateProperty p2("color", mitk::ColorProperty::New(color));
    mitk::NodePredicateOR predicate;
    predicate.AddPredicate(p1);
    predicate.AddPredicate(p2);  // objects must be of datatype "Surface" or have red color (= n1, n2, n4)
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    MITK_TEST_CONDITION(
        (all->Size() == 3) 
        && (std::find(all->begin(), all->end(), n1) != all->end()) 
        && (std::find(all->begin(), all->end(), n2) != all->end())
        && (std::find(all->begin(), all->end(), n4) != all->end()),
        "Requesting objects that meet a disjunction criteria");
  }
  /* Requesting objects that do not meet a criteria */
  {
    mitk::ColorProperty::Pointer cp = mitk::ColorProperty::New(color);
    mitk::NodePredicateProperty proppred("color", cp);
    mitk::NodePredicateNOT predicate(proppred);

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    MITK_TEST_CONDITION(
           (all->Size() == 3) // check if correct objects are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) 
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n5) != stlAll.end()), "Requesting objects that do not meet a criteria");
  }

  /* Requesting *direct* source objects */
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n3, NULL, true); // Get direct parents of n3 (=n2)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    MITK_TEST_CONDITION(
      (all->Size() == 1) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()),
      "Requesting *direct* source objects");
  }

  /* Requesting *all* source objects */
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n3, NULL, false); // Get all parents of n3 (= n1 + n2)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    MITK_TEST_CONDITION(
      (all->Size() == 2) 
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()),
      "Requesting *all* source objects"); // check if n1 and n2 are the resultset
  }

  /* Requesting *all* sources of object with multiple parents */
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, NULL, false); // Get all parents of n4 (= n1 + n2 + n3)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    MITK_TEST_CONDITION(  
      (all->Size() == 3)
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) // check if n1 and n2 and n3 are the resultset
      , "Requesting *all* sources of object with multiple parents");
  }  

  /* Requesting *direct* derived objects */
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, NULL, true); // Get direct childs of n1 (=n2)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    MITK_TEST_CONDITION(
      (all->Size() == 1) 
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())// check if n1 is the resultset
      , "Requesting *direct* derived objects");
   
  }

  ///* Requesting *direct* derived objects with multiple parents/derivations */
  
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n2, NULL, true); // Get direct childs of n2 (=n3 + n4)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    MITK_TEST_CONDITION(
      (all->Size() == 2) 
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) // check if n3 is the resultset
      && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()) // check if n4 is the resultset
      , "Requesting *direct* derived objects with multiple parents/derivations");
  }

  //* Requesting *all* derived objects */
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, NULL, false); // Get all childs of n1 (=n2, n3, n4)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    MITK_TEST_CONDITION(
      (all->Size() == 3) 
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
      , "Requesting *all* derived objects");
  }

  /* Checking for circular source relationships */
  {
    parents1->InsertElement(0, n4);   // make n1 derived from n4 (which is derived from n2, which is derived from n1)
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, NULL, false); // Get all parents of n4 (= n1 + n2 + n3, not n4 itself and not multiple versions of the nodes!)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    MITK_TEST_CONDITION(
      (all->Size() == 3)
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) // check if n1 and n2 and n3 are the resultset
      , "Checking for circular source relationships");
  }

  ///* Checking for circular derivation relationships can not be performed, because the internal derivations datastructure 
  //   can not be accessed from the outside. (Therefore it should not be possible to create these circular relations */

  //* Checking GroupTagProperty */
  {
    mitk::GroupTagProperty::Pointer tp = mitk::GroupTagProperty::New();
    mitk::NodePredicateProperty pred("Resection Proposal 1", tp);
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(pred);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    MITK_TEST_CONDITION(
        (all->Size() == 2) // check if n2 and n3 are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()) 
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
        , "Checking GroupTagProperty");
  }

  /* Checking GroupTagProperty 2 */
  {
    mitk::GroupTagProperty::Pointer tp = mitk::GroupTagProperty::New();
    mitk::NodePredicateProperty pred("Resection Proposal 2", tp);
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(pred);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    MITK_TEST_CONDITION(
      (all->Size() == 2) // check if n3 and n4 are in resultset
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) 
      && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
      , "Checking GroupTagProperty 2");

  }

  /* Checking direct sources with condition */
  {
    mitk::NodePredicateDataType pred("Surface");
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, &pred, true);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    MITK_TEST_CONDITION(
      (all->Size() == 1) // check if n2 is in resultset
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      , "checking direct sources with condition");
  }
  
  /* Checking all sources with condition */
  { 
    mitk::NodePredicateDataType pred("Image");
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, &pred, false);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    MITK_TEST_CONDITION(
      (all->Size() == 1) // check if n1 is in resultset
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
      , "Checking all sources with condition");
  }

  /* Checking all sources with condition with empty resultset */
  { 
    mitk::NodePredicateDataType pred("VesselTree");
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, &pred, false);
    MITK_TEST_CONDITION(all->Size() == 0 , "Checking all sources with condition with empty resultset"); // check if resultset is empty
  }

  /* Checking direct derivations with condition */
  {
    mitk::NodePredicateProperty pred("color");
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, &pred, true);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    MITK_TEST_CONDITION(
      (all->Size() == 1) // check if n2 is in resultset
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      , "Checking direct derivations with condition");
  }

  /* Checking all derivations with condition */
  {
    mitk::NodePredicateProperty pred("color");

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, &pred, false);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    MITK_TEST_CONDITION(
      (all->Size() == 2) // check if n2 and n4 are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
        , "Checking direct derivations with condition");
  }

  /* Checking named node method */
  MITK_TEST_CONDITION(ds->GetNamedNode("Node 2 - Surface Node") == n2, "Checking named node method");

  /* Checking named node method with wrong name */
  MITK_TEST_CONDITION(ds->GetNamedNode("This name does not exist") == NULL, "Checking named node method with wrong name");

  /* Checking named object method */
  MITK_TEST_CONDITION(ds->GetNamedObject<mitk::Image>("Node 1 - Image Node") == image, "Checking named object method");

  /* Checking named object method with wrong DataType */
  MITK_TEST_CONDITION(ds->GetNamedObject<mitk::Surface>("Node 1 - Image Node") == NULL, "Checking named object method with wrong DataType");    
  
  /* Checking named object method with wrong name */
  MITK_TEST_CONDITION(ds->GetNamedObject<mitk::Image>("This name does not exist") == NULL, "Checking named object method with wrong name");  

  /* Checking GetNamedDerivedNode with valid name and direct derivation only */
  MITK_TEST_CONDITION(ds->GetNamedDerivedNode("Node 2 - Surface Node", n1, true) == n2, "Checking GetNamedDerivedNode with valid name & direct derivation only");

  /* Checking GetNamedDerivedNode with invalid Name and direct derivation only */
  MITK_TEST_CONDITION(ds->GetNamedDerivedNode("wrong name", n1, true) == NULL, "Checking GetNamedDerivedNode with invalid name & direct derivation only");

  /* Checking GetNamedDerivedNode with invalid Name and direct derivation only */
  MITK_TEST_CONDITION(ds->GetNamedDerivedNode("Node 3 - Empty Node", n1, false) == n3, "Checking GetNamedDerivedNode with invalid name & direct derivation only");

    
  /* Checking GetNamedDerivedNode with valid Name but direct derivation only */
  MITK_TEST_CONDITION(ds->GetNamedDerivedNode("Node 3 - Empty Node", n1, true) == NULL, "Checking GetNamedDerivedNode with valid Name but direct derivation only");
  
  /* Checking GetNode with valid predicate */
  {
    mitk::NodePredicateDataType p("Image");
    MITK_TEST_CONDITION(ds->GetNode(&p) == n1, "Checking GetNode with valid predicate");
  }  
  /* Checking GetNode with invalid predicate */
  {
    mitk::NodePredicateDataType p("PointSet");
    MITK_TEST_CONDITION(ds->GetNode(&p) == NULL, "Checking GetNode with invalid predicate");
  }

  } // object retrieval methods
  catch(...)
  {
    MITK_TEST_FAILED_MSG( << "Exeption during object retrieval (GetXXX() Methods)");
  }

  try  /* object removal methods */
  {
 
  /* Checking removal of a node without relations */
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    int refCountbeforeDS = watcher->GetReferenceCount();
    ds->Add(extra);
    MITK_TEST_CONDITION(ds->GetNamedNode("extra") == extra, "Adding extra node");
    ds->Remove(extra);
    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == NULL) 
      && (refCountbeforeDS == watcher->GetReferenceCount())
      , "Checking removal of a node without relations");
    extra = NULL;
  }

  /* Checking removal of a node with a parent */
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
   
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    int refCountbeforeDS = watcher->GetReferenceCount();
    ds->Add(extra, n1);   // n1 is parent of extra

    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == extra)
      && (ds->GetDerivations(n1)->Size() == 2)   // n2 and extra should be derived from n1
      , "Adding extra node");
    ds->Remove(extra);
    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == NULL)
      && (refCountbeforeDS == watcher->GetReferenceCount())
      && (ds->GetDerivations(n1)->Size() == 1)
      , "Checking removal of a node with a parent");  
    extra = NULL;
  }      

  /* Checking removal of a node with two parents */
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
   
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    int refCountbeforeDS = watcher->GetReferenceCount();
    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(n2);
    ds->Add(extra, p);   // n1 and n2 are parents of extra
    

    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == extra)
      && (ds->GetDerivations(n1)->Size() == 2)    // n2 and extra should be derived from n1
      && (ds->GetDerivations(n2)->Size() == 3)
      , "add extra node");
    
    ds->Remove(extra);
    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == NULL)
      && (refCountbeforeDS == watcher->GetReferenceCount())
      && (ds->GetDerivations(n1)->Size() == 1)   // after remove, only n2 should be derived from n1
      && (ds->GetDerivations(n2)->Size() == 2)   // after remove, only n3 and n4 should be derived from n2
      , "Checking removal of a node with two parents");
    extra = NULL;
  }  

  /* Checking removal of a node with two derived nodes */
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    int refCountbeforeDS = watcher->GetReferenceCount();
    ds->Add(extra);
    mitk::DataTreeNode::Pointer d1 = mitk::DataTreeNode::New();
    d1->SetProperty("name", mitk::StringProperty::New("d1"));
    ds->Add(d1, extra);
    mitk::DataTreeNode::Pointer d2 = mitk::DataTreeNode::New();
    d2->SetProperty("name", mitk::StringProperty::New("d2"));    
    ds->Add(d2, extra);

    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == extra)
      && (ds->GetNamedNode("d1") == d1)
      && (ds->GetNamedNode("d2") == d2)
      && (ds->GetSources(d1)->Size() == 1)    // extra should be source of d1
      && (ds->GetSources(d2)->Size() == 1)    // extra should be source of d2
      && (ds->GetDerivations(extra)->Size() == 2)    // d1 and d2 should be derived from extra
      , "add extra node");

    ds->Remove(extra);
    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == NULL)
      && (ds->GetNamedNode("d1") == d1)
      && (ds->GetNamedNode("d2") == d2)
      && (refCountbeforeDS == watcher->GetReferenceCount())
      && (ds->GetSources(d1)->Size() == 0)   // after remove, d1 should not have a source anymore
      && (ds->GetSources(d2)->Size() == 0)   // after remove, d2 should not have a source anymore
      , "Checking removal of a node with two derived nodes");
    extra = NULL;
  }  

  /* Checking removal of a node with two parents and two derived nodes */
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    mitk::ReferenceCountWatcher::Pointer n1watcher = new mitk::ReferenceCountWatcher(n1);
    int refCountbeforeDS = watcher->GetReferenceCount();
    
    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(n2);
    ds->Add(extra, p);   // n1 and n2 are parents of extra

    mitk::DataTreeNode::Pointer d1 = mitk::DataTreeNode::New();
    d1->SetProperty("name", mitk::StringProperty::New("d1x"));
    ds->Add(d1, extra);
    mitk::DataTreeNode::Pointer d2 = mitk::DataTreeNode::New();
    d2->SetProperty("name", mitk::StringProperty::New("d2x"));    
    ds->Add(d2, extra);

    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == extra)
      && (ds->GetNamedNode("d1x") == d1)
      && (ds->GetNamedNode("d2x") == d2)
      && (ds->GetSources(d1)->Size() == 1)    // extra should be source of d1
      && (ds->GetSources(d2)->Size() == 1)    // extra should be source of d2
      && (ds->GetDerivations(n1)->Size() == 2)    // n2 and extra should be derived from n1
      && (ds->GetDerivations(n2)->Size() == 3)   // n3, n4 and extra should be derived from n2
      && (ds->GetDerivations(extra)->Size() == 2)    // d1 and d2 should be derived from extra
      , "add extra node");

      ds->Remove(extra);
      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == NULL)
        && (ds->GetNamedNode("d1x") == d1)
        && (ds->GetNamedNode("d2x") == d2)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetDerivations(n1)->Size() == 1)    // after remove, only n2 should be derived from n1
        && (ds->GetDerivations(n2)->Size() == 2)    // after remove, only n3 and n4 should be derived from n2
        && (ds->GetSources(d1)->Size() == 0)        // after remove, d1 should not have a source anymore
        && (ds->GetSources(d2)->Size() == 0)       // after remove, d2 should not have a source anymore
        , "Checking removal of a node with two parents and two derived nodes");
      extra = NULL;
    }
  }
  catch(...)
  {
    MITK_TEST_FAILED_MSG( << "Exeption during object removal methods");
  }


 /* Checking for node is it's own parent exception */
  {
  MITK_TEST_FOR_EXCEPTION_BEGIN(...);
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(extra); // extra is parent of extra!!!
    ds->Add(extra, p); 
  MITK_TEST_FOR_EXCEPTION_END(...);
  }


 /* Checking reference count of node after add and remove */
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(n3); 
    ds->Add(extra, p); 
    extra = NULL;
    ds->Remove(ds->GetNamedNode("extra"));
    MITK_TEST_CONDITION(watcher->GetReferenceCount() == 0, "Checking reference count of node after add and remove");
  } 

  /* Checking GetGrouptags() */
  {
    const std::set<std::string> groupTags = ds->GetGroupTags();
    MITK_TEST_CONDITION(
      (groupTags.size() == 2)
      && (std::find(groupTags.begin(), groupTags.end(), "Resection Proposal 1") != groupTags.end())
      && (std::find(groupTags.begin(), groupTags.end(), "Resection Proposal 2") != groupTags.end())
      , "Checking GetGrouptags()");
  }  


  /* Checking Event handling */  
  DSEventReceiver listener;
  try
  {
    ds->AddNodeEvent.AddListener(&listener, &DSEventReceiver::OnAdd);
    ds->RemoveNodeEvent.AddListener(&listener, &DSEventReceiver::OnRemove);
       
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    ds->Add(extra); 

    MITK_TEST_CONDITION(listener.m_NodeAdded == extra.GetPointer(), "Checking AddEvent");

    ds->Remove(extra); 
    MITK_TEST_CONDITION(listener.m_NodeRemoved == extra.GetPointer(), "Checking RemoveEvent");
    
    /* RemoveListener */
    ds->AddNodeEvent.RemoveListener(&listener, &DSEventReceiver::OnAdd);
    ds->RemoveNodeEvent.RemoveListener(&listener, &DSEventReceiver::OnRemove);
    listener.m_NodeAdded = NULL;
    listener.m_NodeRemoved = NULL;
    ds->Add(extra);
    ds->Remove(extra);
    MITK_TEST_CONDITION((listener.m_NodeRemoved == NULL) && (listener.m_NodeAdded == NULL), "Checking RemoveListener");
    

    std::cout << "Pointer handling after event handling: " << std::flush;
    extra = NULL; // delete reference to the node. its memory should be freed now
    MITK_TEST_CONDITION(watcher->GetReferenceCount() == 0, "Pointer handling after event handling");
  }
  catch(...)
  {
    /* cleanup */
    ds->AddNodeEvent.RemoveListener(&listener, &DSEventReceiver::OnAdd);
    ds->RemoveNodeEvent.RemoveListener(&listener, &DSEventReceiver::OnRemove);
    MITK_TEST_FAILED_MSG( << "Exception during object removal methods");
  }

  /* Clear DataStorage */
  ds->Remove(ds->GetAll());
  MITK_TEST_CONDITION(ds->GetAll()->Size() == 0, "Checking Clear DataStorage");
}


//## Documentation
//## Test DataTreeStorage specific behavior
void TestDataTreeStorage(mitk::DataTreeStorage* ds, mitk::DataTree* tree)
{
  int objectsInTree = tree->Count();
  
  /* Adding a node directly to the tree to test if the DataStorage can handle that */
  mitk::DataTreePreOrderIterator it(tree);
  mitk::DataTreeNode::Pointer treeNode = mitk::DataTreeNode::New();   // node with image and name property
  treeNode->SetProperty("name", mitk::StringProperty::New("TreeNode - not added by DataStorage"));  
  it.Add(treeNode);
  MITK_TEST_CONDITION(ds->GetNamedNode("TreeNode - not added by DataStorage") == treeNode, "Adding a node directly to the tree");

  mitk::DataTreeNode::Pointer n1 = mitk::DataTreeNode::New();   // node with image and name property
  n1->SetName("n1");
  ds->Add(n1);
  ds->SetManageCompleteTree(true);
  MITK_TEST_CONDITION(ds->GetAll()->Size() == static_cast<unsigned int>(tree->Count()), "Testing SetManageCompleteTree(true): same number of objects in tree and dts");
  
  ds->SetManageCompleteTree(false);
  MITK_TEST_CONDITION(ds->GetAll()->Size() == 1, "Testing SetManageCompleteTree(false) different number of objects in tree and dts");

  /* Checking DataTree Delete Observer functionality */
  {
    
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    // Strange syntax, but VS2008 compiler could not resolve the overloaded method otherwise:
    ds->DataStorage::Add(extra, n1); // add extra to DataStorage. Reference count should be 5 (extra smartpointer, tree, sources map, derivations map, derivations list of n1)
    mitk::DataTreeIteratorClone it = mitk::DataTreeHelper::FindIteratorToNode(tree, extra); // remove extra directly from tree
    it->Disconnect(); // delete node directly from tree. the observer mechanism should delete it from the internal relations too
    extra = NULL; // delete last reference to the node. its memory should be freed now

    MITK_TEST_CONDITION(watcher->GetReferenceCount() == 0, "Checking DataTree Delete Observer functionality");
  }
  /* Checking RemoveEvent on delete in DataTree */
  {
    DSEventReceiver listener;
    ds->RemoveNodeEvent.AddListener(&listener, &DSEventReceiver::OnRemove);

    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();    
    ds->Add(extra); 
    mitk::DataTreeIteratorClone it = mitk::DataTreeHelper::FindIteratorToNode(tree, extra); // remove extra directly from tree
    it->Disconnect(); // delete node directly from tree. the observer mechanism should delete it from the internal relations too
    MITK_TEST_CONDITION((listener.m_NodeRemoved == extra.GetPointer()), "Checking RemoveEvent on delete in DataTree");
  }
}
