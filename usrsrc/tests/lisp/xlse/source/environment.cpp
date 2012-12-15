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
#include "environment.h"
#include "primitives.h"

// Include the XLSE namespace
using namespace XLSE;

// Environment data allocator
CAllocator<CEnvironment::SEnvData> CEnvironment::s_Allocator;

/*******************************************************
* Function: CEnvironment::CEnvironment()
* Purpose : Constructor for environment class
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
CEnvironment::CEnvironment()
{
	// Allocate environment data
	m_pEnvData = (SEnvData*)s_Allocator.Allocate();

	// Initially one reference
	m_pEnvData->RefCount = 1;

	// No parent initially
	m_pEnvData->pParent = NULL;
}

/*******************************************************
* Function: CEnvironment::CEnvironment()
* Purpose : Copy constructor for environment class
* Initial : Max Payne on September 2, 2006
********************************************************
Revisions and bug fixes:
*/
CEnvironment::CEnvironment(const CEnvironment& Env)
{
	// Allocate environment data
	m_pEnvData = (SEnvData*)s_Allocator.Allocate();

	// Initially one reference
	m_pEnvData->RefCount = 1;

	// Copy the other environment's bindings
	m_pEnvData->Bindings = Env.m_pEnvData->Bindings;

	// Copy the other environment's parent
	m_pEnvData->pParent = Env.m_pEnvData->pParent;

	// If this environment has a parent
	if (m_pEnvData->pParent)
	{
		// Increment the parent's reference count
		++m_pEnvData->pParent->RefCount;
	}
}

/*******************************************************
* Function: CEnvironment::~CEnvironment()
* Purpose : Destructor for environment class
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
CEnvironment::~CEnvironment()
{
	// For each level of the environment hierarchy
	for (SEnvData* pEnvData = m_pEnvData; pEnvData != NULL; pEnvData = pEnvData->pParent)
	{
		// Decrement the reference count of the environment data
		--pEnvData->RefCount;

		// If there are no references to this environment data anymore
		if (pEnvData->RefCount == 0)
		{
			// Release the environment data
			s_Allocator.Release(pEnvData);
		}
		else
		{
			// Don't continue to the parent environment
			break;
		}
	}
}

/*******************************************************
* Function: CEnvironment::Clear()
* Purpose : Clear all environment bindings
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
void CEnvironment::Clear()
{
	// Clear all bindings
	m_pEnvData->Bindings.clear();
}

/*******************************************************
* Function: CEnvironment::Bind()
* Purpose : Create a new binding in the environment
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
bool CEnvironment::Bind(const std::string& Name, const CReference& Item)
{
	// Set the binding
	m_pEnvData->Bindings[Name] = Item;

	// Nothing went wrong
	return true;
}

/*******************************************************
* Function: CEnvironment::Set()
* Purpose : Set an existing binding in the environment
* Initial : Max Payne on September 20, 2006
********************************************************
Revisions and bug fixes:
*/
bool CEnvironment::Set(const std::string& Name, const CReference& Item)
{
	// For each level of the environment hierarchy
	for (SEnvData* pEnvData = m_pEnvData; pEnvData; pEnvData = pEnvData->pParent)
	{
		// Attempt to find the binding
		TItemMap::iterator Itr = pEnvData->Bindings.find(Name);

		// If the binding was found
		if (Itr != pEnvData->Bindings.end())
		{
			// Modify the binding
			Itr->second = Item;

			// Set operation successful
			return true;
		}
	}

	// Binding not found
	return false;
}

/*******************************************************
* Function: CEnvironment::Find()
* Purpose : Find a bound item in the environment
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CEnvironment::Find(const std::string& Name) const
{
	// For each level of the environment hierarchy
	for (const SEnvData* pEnvData = m_pEnvData; pEnvData; pEnvData = pEnvData->pParent)
	{
		// Attempt to find the binding
		TItemMap::const_iterator Itr = pEnvData->Bindings.find(Name);

		// If the binding was found
		if (Itr != pEnvData->Bindings.end())
		{
			// Return the item address
			return Itr->second;
		}
	}

	// Binding not found
	return NIL;
}

/*******************************************************
* Function: CEnvironment::Extend()
* Purpose : Create an extended environment
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
CEnvironment CEnvironment::Extend()
{
	// Create a new environment
	CEnvironment Environment;

	// Set the parent pointer of the extended environment
	Environment.m_pEnvData->pParent = m_pEnvData;

	// Increment the reference count of the local environment data
	++m_pEnvData->RefCount;

	// Return the new environment
	return Environment;
}

/*******************************************************
* Function: CEnvironment::operator = ()
* Purpose : Assignment operator
* Initial : Max Payne on September 2, 2006
********************************************************
Revisions and bug fixes:
*/
CEnvironment& CEnvironment::operator = (const CEnvironment& Env)
{	
	// Copy the other environment's bindings
	m_pEnvData->Bindings = Env.m_pEnvData->Bindings;

	// Copy the other environment's parent
	m_pEnvData->pParent = Env.m_pEnvData->pParent;

	// If this environment has a parent
	if (m_pEnvData->pParent)
	{
		// Increment the parent's reference count
		++m_pEnvData->pParent->RefCount;
	}

	// Return a reference to this environment
	return *this;
}