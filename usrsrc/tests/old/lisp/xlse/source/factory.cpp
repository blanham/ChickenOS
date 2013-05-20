/*****************************************************************************
*
* Copyright © 2006-2008, Maxime Chevalier-Boisvert
*
* Contact information:
* e-mail: mcheva@cs.mcgill.ca
* phone : 1-514-935-2809
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* 
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*****************************************************************************/

// Header files
#include <cassert>
#include "factory.h"
#include "references.h"
#include "primitives.h"

// Include the XLSE namespace
using namespace XLSE;

// Static factory singleton instance
CFactory CFactory::s_Instance;

/*******************************************************
* Function: CFactory::CFactory()
* Purpose : Constructor for factory class
* Initial : Max Payne on February 16, 2006
********************************************************
Revisions and bug fixes:
*/
CFactory::CFactory()
: m_NumAllocations(0),
  m_GCCycleLength(FACTORY_DEFAULT_GC_CYCLE),
  m_GCActive(true)
{
	// Register allocators for the primitive types
	RegisterType(TYPE_QUOTE      , new CAllocator<CQuote>      );
	RegisterType(TYPE_PAIR       , new CAllocator<CPair>       );
	RegisterType(TYPE_SYMBOL     , new CAllocator<CSymbol>     );
	RegisterType(TYPE_BOOL       , new CAllocator<CBool>       );
	RegisterType(TYPE_INT        , new CAllocator<CInt>        );
	RegisterType(TYPE_FLOAT      , new CAllocator<CFloat>      );
	RegisterType(TYPE_STRING     , new CAllocator<CString>     );
	RegisterType(TYPE_VECTOR	 , new CAllocator<CVector>     );
	RegisterType(TYPE_MACRO      , new CAllocator<CMacro>      );
	RegisterType(TYPE_FUNCTION   , new CAllocator<CFunction>   );
	RegisterType(TYPE_EXTMACRO   , new CAllocator<CExtMacro>   );
	RegisterType(TYPE_EXTFUNCTION, new CAllocator<CExtFunction>);
}

/*******************************************************
* Function: CFactory::~CFactory()
* Purpose : Constructor for factory class
* Initial : Max Payne on February 16, 2006
********************************************************
Revisions and bug fixes:
*/
CFactory::~CFactory()
{
	// Delete script type allocators
	for (TAllocatorMap::iterator AllocItr = m_Allocators.begin(); AllocItr != m_Allocators.end(); ++AllocItr)
		delete (AllocItr->second);
}

/*******************************************************
* Function: CFactory::RegisterType()
* Purpose : Register a new script type
* Initial : Max Payne on February 19, 2006
********************************************************
Revisions and bug fixes:
*/
void CFactory::RegisterType(TTypeId TypeId, IAllocator* pAllocator)
{
	// Insure that the allocator is valid
	assert(pAllocator != NULL);

	// Insure the type is not already registered
	assert(m_Allocators.find(TypeId) == m_Allocators.end());

	// Set the allocator for the specified type id
	m_Allocators[TypeId] = pAllocator;
}

/*******************************************************
* Function: CFactory::CreateType()
* Purpose : Create a script object of a specific type
* Initial : Max Payne on February 19, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CFactory::CreateType(TTypeId TypeId)
{
	// Increment the number of allocations made
	++m_NumAllocations;

	// If an allocation cycle complete, perform garbage collection
	if (m_NumAllocations % m_GCCycleLength == 0)
		PerformGC();

	// Attempt to find the allocator for this type id
	TAllocatorMap::iterator AllocatorItr = m_Allocators.find(TypeId);

	// Insure that the allocator was found
	assert(AllocatorItr != m_Allocators.end());

	// Allocate the script type object
	CType* pObject = (CType*)AllocatorItr->second->Allocate();

	// Add the object to the set of all allocated objects
	m_Allocated.insert(pObject);

	// Return a reference to the object
	return CReference(pObject);
}

/*******************************************************
* Function: CFactory::RegisterRef()
* Purpose : Register a root reference
* Initial : Max Payne on February 19, 2006
********************************************************
Revisions and bug fixes:
*/
void CFactory::RegisterRef(CReference* pRootRef)
{
	// Insure that the reference is valid
	assert(pRootRef != NULL);

	// Insure this reference points to an object we have allocated
	assert(m_Allocated.find(pRootRef->GetPointer()) != m_Allocated.end());

	// Add the reference to the set
	m_RootRefs.insert(pRootRef);
}

/*******************************************************
* Function: CFactory::ReleaseRef()
* Purpose : Release a root reference
* Initial : Max Payne on February 19, 2006
********************************************************
Revisions and bug fixes:
*/
void CFactory::ReleaseRef(CReference* pRootRef)
{
	// Insure that the reference is valid
	assert(pRootRef != NULL);

	// Attempt to find the reference
	TRootRefSet::iterator RefItr = m_RootRefs.find(pRootRef);

	// If the reference was found
	if (RefItr != m_RootRefs.end())
	{
		// Remove the reference from the root reference set
		m_RootRefs.erase(RefItr);
	}
}

/*******************************************************
* Function: CFactory::PerformGC()
* Purpose : Perform a garbage collection cycle
* Initial : Max Payne on February 19, 2006
********************************************************
Revisions and bug fixes:
*/
void CFactory::PerformGC()
{
	// If GC is not active, exit the function
	if (!m_GCActive)
		return;

	// Create a set of all unvisited objects
	TObjectSet Unvisited(m_Allocated.begin(), m_Allocated.end());

	// For each root reference
	for (TRootRefSet::iterator RefItr = m_RootRefs.begin(); RefItr != m_RootRefs.end(); ++RefItr)
	{
		// Visit this reference branch
		VisitRef(*(*RefItr), Unvisited);
	}

	// For each unvisited (unreachable) object
	for (TObjectSet::iterator ObjItr = Unvisited.begin(); ObjItr != Unvisited.end(); ++ObjItr)
	{
		// Attempt to find the allocator for this object
		TAllocatorMap::iterator AllocItr = m_Allocators.find((*ObjItr)->GetTypeId());

		// Release the memory for this object
		AllocItr->second->Release(*ObjItr);

		// Remove this object from the set of allocated objects
		m_Allocated.erase(*ObjItr);
	}
}

/*******************************************************
* Function: CFactory::VisitRef()
* Purpose : Visit a reference branch during GC (DFS)
* Initial : Max Payne on February 19, 2006
********************************************************
Revisions and bug fixes:
*/
void CFactory::VisitRef(const CWeakReference& Reference, TObjectSet& Unvisited)
{
	// Get a pointer to the referenced object
	CType* pObject = Reference.GetPointer();

	// Attempt to find this object in the unvisited object set
	TObjectSet::iterator ObjectItr = Unvisited.find(pObject);

	// If the object was not visited
	if (ObjectItr != Unvisited.end())
	{
		// Get a pointer to the current object
		CType* pCurrentObj = *ObjectItr;

		// Remove the object from the set (it is now visited)
		Unvisited.erase(ObjectItr);

		// For each outgoing reference
		for (size_t i = 0; i < pCurrentObj->GetNumRefs(); ++i)
		{
			// Visit this outgoing reference branch recursively
			VisitRef(pCurrentObj->GetOutRef(i), Unvisited);
		}
	}
}