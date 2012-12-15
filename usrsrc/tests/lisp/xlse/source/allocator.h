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
#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

// Header files
#include <cassert>
#include <vector>
#include "basictypes.h"

// Include this in the XLSE namespace
namespace XLSE
{
	// Forward declarations
	class CFactory;

	// Constants
	const size_t ALLOCATOR_RESERVE_DEFAULT = 8192;

	/*******************************************************
	* Class   : IAllocator
	* Purpose : Interface class for allocators
	* Initial : Max Payne on February 17, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class IAllocator
	{
	public:

		// Constructor and destructor
		IAllocator() {}
		virtual ~IAllocator() {}

		// Method to allocate a single unit
		virtual void* Allocate() = 0;

		// Method to release a specific unit
		virtual void Release(void* pUnit) = 0;

		// Method to release all allocated units
		virtual void ReleaseAll() = 0;
	};

	/*******************************************************
	* Class   : CAllocator
	* Purpose : Allocator with pool reserve
	* Initial : Max Payne on February 17, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type>
	class CAllocator : public IAllocator
	{
	public:

		// Constructor and destructor
		CAllocator(size_t Reserve = ALLOCATOR_RESERVE_DEFAULT);
		virtual ~CAllocator();

		// Method to allocate a single unit
		void* Allocate();

		// Method to extend the allocation pool
		void ExtendPool();

		// Method to release a specific unit
		void Release(void* pUnit);

		// Method to release all allocated units
		void ReleaseAll();

	private:

		// Vector of pre-allocated elements
		std::vector<Type*> m_AllocPool;

		// Vector of free elements
		std::vector<Type*> m_FreeUnits;
	};

	/*******************************************************
	* Function: CAllocator::CAllocator()
	* Purpose : Constructor for pool allocator
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type>
	CAllocator<Type>::CAllocator(size_t Reserve)
	{
		// Reserve enough space for all allocation units
		m_AllocPool.resize(Reserve);

		// Pre-allocate all allocation units
		for (size_t i = 0; i < Reserve; ++i)
			m_AllocPool[i] = (Type*)malloc(sizeof(Type));

		// Reserve enough space for all free units
		m_FreeUnits.resize(Reserve);

		// Store the memory address of each allocation unit
		for (size_t i = 0; i < Reserve; ++i)
			m_FreeUnits[i] = m_AllocPool[i];
	}

	/*******************************************************
	* Function: CAllocator::~CAllocator()
	* Purpose : Destructor for pool allocator
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type>
	CAllocator<Type>::~CAllocator()
	{
		// Delete each allocation unit
		for (size_t i = 0; i < m_AllocPool.size(); ++i)
			free(m_AllocPool[i]);
	}

	/*******************************************************
	* Function: CAllocator::Allocate()
	* Purpose : Allocate memory for one unit
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type>
	void* CAllocator<Type>::Allocate()
	{
		// If there are no free units, extend the pool
		if (m_FreeUnits.empty())
			ExtendPool();

		// Get the address of the last allocation unit
		Type* pUnit = m_FreeUnits.back();

		// Remove the unit from the free vector
		m_FreeUnits.pop_back();

		// Reset the unit by calling its constructor
		new(pUnit) Type();

		// Return the pointer to the unit
		return pUnit;
	}

	/*******************************************************
	* Function: CAllocator::ExtendPool()
	* Purpose : Extend the allocation pool size
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type>
	void CAllocator<Type>::ExtendPool()
	{
		// Obtain the current allocation pool size
		size_t CurrentSize = m_AllocPool.size();

		// Compute the new pool size (double it)
		size_t NewSize = CurrentSize * 2;

		// Resize the pool to fit the new units
		m_AllocPool.resize(NewSize);

		// Pre-allocate the new alllocation units
		for (size_t i = CurrentSize; i < NewSize; ++i)
			m_AllocPool[i] = (Type*)malloc(sizeof(Type));
		
		// Store the number of free units available
		size_t CurrentFree = m_FreeUnits.size();

		// Compute the number of new units
		size_t NumNew = NewSize - CurrentSize;

		// Compute the new size of the free unit pool
		size_t NewFreeSize = CurrentFree + NumNew;

		// Resize the free unit pool to fit all units
		m_FreeUnits.resize(NewFreeSize);

		// Register all new free units
		for (size_t i = 0; i < NumNew; ++i)
			m_FreeUnits[CurrentFree + i] = m_AllocPool[CurrentSize + i];
	}

	/*******************************************************
	* Function: CAllocator::Release()
	* Purpose : Release memory for one unit
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type>
	void CAllocator<Type>::Release(void* pUnit)
	{
		// Assert the validity of the unit address
		assert(pUnit != NULL);

		// Make sure there are allocated units
		assert(m_FreeUnits.size() < m_AllocPool.size());

		// Create a typed pointer to the unit
		Type* pTypedUnit = (Type*)pUnit;

		// Explicitly call the unit's destructor
		pTypedUnit->~Type();

		// Add the free unit back to the free unit vector
		m_FreeUnits.push_back(pTypedUnit);
	}

	/*******************************************************
	* Function: CAllocator::ReleaseAll()
	* Purpose : Release memory for all allocated units
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type>
	void CAllocator<Type>::ReleaseAll()
	{
		// Resize the free unit pool to fit all allocation units
		m_FreeUnits.resize(m_AllocPool.size());

		// Store the memory address of each allocation unit
		for (size_t i = 0; i < m_AllocPool.size(); ++i)
			m_FreeUnits[i] = m_AllocPool[i];
	}
};

#endif // #ifndef _ALLOCATOR_H_