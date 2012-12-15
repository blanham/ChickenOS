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
#include "references.h"
#include "primitives.h"
#include "factory.h"

// Include the XLSE namespace
using namespace XLSE;

/*******************************************************
* Function: CWeakReference::CWeakReference()
* Purpose : Constructor for weak reference class
* Initial : Max Payne on August 28, 2006
********************************************************
Revisions and bug fixes:
*/
CWeakReference::CWeakReference()
: m_Pointer(NIL.GetPointer())
{
}

/*******************************************************
* Function: CWeakReference::Nullify()
* Purpose : Nullify a reference
* Initial : Max Payne on August 28, 2006
********************************************************
Revisions and bug fixes:
*/
void CWeakReference::Nullify()
{
	// Point to the global NIL object instance
	m_Pointer = NIL.GetPointer();
}

/*******************************************************
* Function: CWeakReference::IsNil()
* Purpose : Verify the validity of a reference
* Initial : Max Payne on August 28, 2006
********************************************************
Revisions and bug fixes:
*/
bool CWeakReference::IsNil() const
{
	// Test whether we point to a NIL object or not
	return (m_Pointer->GetTypeId() == TYPE_NIL);
}

/*******************************************************
* Function: CReference::CReference()
* Purpose : Constructor for reference class
* Initial : Max Payne on September 1, 2006
********************************************************
Revisions and bug fixes:
*/
CReference::CReference(CType* pType)
{
	// Bind this reference
	BindRef(pType);
}

/*******************************************************
* Function: CReference::CReference()
* Purpose : Copy constructor for reference class
* Initial : Max Payne on September 1, 2006
********************************************************
Revisions and bug fixes:
*/
CReference::CReference(const CReference& Ref)
{
	// Bind this reference
	BindRef(Ref.m_Pointer);
}

/*******************************************************
* Function: CReference::CReference()
* Purpose : Default constructor for reference class
* Initial : Max Payne on September 1, 2006
********************************************************
Revisions and bug fixes:
*/
CReference::CReference()
{
}

/*******************************************************
* Function: CReference::~CReference()
* Purpose : Destructor for reference class
* Initial : Max Payne on September 1, 2006
********************************************************
Revisions and bug fixes:
*/
CReference::~CReference()
{ 
	// Release this reference
	CFactory::GetInstance().ReleaseRef(this);
}

/*******************************************************
* Function: CReference::BindRef()
* Purpose : Bind a root reference
* Initial : Max Payne on February 21, 2006
********************************************************
Revisions and bug fixes:
*/
void CReference::BindRef(CType* pType)
{
	// Ensure that the pointer is not NULL
	assert(pType != NULL);

	// Copy the pointer
	m_Pointer = pType;

	// If the pointer is not null
	if (m_Pointer->GetTypeId() != TYPE_NIL)
	{
		// Register this reference in the factory
		CFactory::GetInstance().RegisterRef(this);
	}
}