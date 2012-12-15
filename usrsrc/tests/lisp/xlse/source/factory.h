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

// Definitions
#ifndef _FACTORY_H_
#define _FACTORY_H_

// Header files
#include <map>
#include <set>
#include "basictypes.h"
#include "allocator.h"

// Include this in the XLSE namespace
namespace XLSE
{
	// Forward declarations
	class CType;
	class CReference;
	class CWeakReference;
	class CPair;

	// Script type id definition
	typedef uint32 TTypeId;

	// Constants
	const size_t FACTORY_DEFAULT_GC_CYCLE = 4096;

	/*******************************************************
	* Class   : CFactory
	* Purpose : Manage allocation of script primitives
	* Initial : Max Payne on February 16, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CFactory
	{
		// Make the direct reference class a friend
		friend class CReference;

	public:

		// Public destructor
		~CFactory();

		// Method to register a type
		void RegisterType(TTypeId TypeId, IAllocator* pAllocator);

		// Method to create a script object of a specific type
		CReference CreateType(TTypeId TypeId);

		// Mutator to set whether GC is active or not
		void SetGCActive(bool GCActive) { m_GCActive = GCActive; }

		// Method to get the singleton instance
		static CFactory& GetInstance() { return s_Instance; }

	private:

		// Private constructor
		CFactory();

		// Define an allocator map type
		typedef std::map<TTypeId, IAllocator*> TAllocatorMap;

		// Define a root reference set type
		typedef std::set<CReference*> TRootRefSet;

		// Define an object set type
		typedef std::set<CType*> TObjectSet;

		// Method to create a root reference to a script type
		void RegisterRef(CReference* pRootRef);

		// Method to release a root reference
		void ReleaseRef(CReference* pRootRef);

		// Method to perform a garbage collection cycle
		void PerformGC();
	           
		// Method to visit a reference branch during GC (DFS)
		void VisitRef(const CWeakReference& Reference, TObjectSet& Unvisited);

		// Map of script type ids to allocators
		TAllocatorMap m_Allocators;

		// Set of root references
		TRootRefSet m_RootRefs;

		// Set of all allocated objects
		TObjectSet m_Allocated;

		// Number of allocations performed
		size_t m_NumAllocations;

		// Garbage collection cycle length
		size_t m_GCCycleLength;

		// Whether to perform GC or not
		bool m_GCActive;

		// Static factory singleton instance
		static CFactory s_Instance;
	};
};

#endif // #ifndef _FACTORY_H_