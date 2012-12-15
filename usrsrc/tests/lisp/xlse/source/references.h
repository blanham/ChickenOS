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
#ifndef _REFERENCES_H_
#define _REFERENCES_H_

// Header files
#include <cstdio>
#include <cassert>
#include "types.h"

// Include this in the XLSE namespace
namespace XLSE
{
	// Forward declarations
	class CType;
	class CFactory;
	class CWeakReference;
	class CReference;
	template <class Type> class CWeakRef;
	template <class Type> class CRef;



	/*******************************************************
	* Class   : CWeakRef<>
	* Purpose : Templated indirect reference to a type
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> class CWeakRef : public CWeakReference
	{
	public:

		// Constructors and destructor
		CWeakRef(Type* Pointer);
		CWeakRef(const CWeakRef<Type>& Ref) { m_Pointer = Ref.m_Pointer; }
		CWeakRef(const CReference& Ref) { *this = ((CWeakReference)Ref); }
		CWeakRef() {}
		~CWeakRef() {}

   		// Operators to access the referenced object
		const Type* operator -> () const;
		Type* operator -> ();

		// Comparison operators
		bool operator == (const CWeakRef<Type>& Ref) const { return (m_Pointer == Ref.m_Pointer); }
		bool operator != (const CWeakRef<Type>& Ref) const { return (m_Pointer != Ref.m_Pointer); }

		// Assignment operators
		const CWeakRef<Type>& operator = (const CWeakRef<Type>& Ref) { m_Pointer = Ref.m_Pointer; return *this; }
	};

	/*******************************************************
	* Class   : CRef<>
	* Purpose : Templated direct reference to a type
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> class CRef : public CReference
	{
	public:

		// Constructors and destructor
		CRef(Type* Pointer);
		CRef(const CRef<Type>& Ref) { BindRef(Ref.m_Pointer); }
		CRef(const CReference& Ref) { *this = ((CWeakReference)Ref); }
		CRef() {}
		~CRef() {}

   		// Operators to access the referenced object
		const Type* operator -> () const;
		Type* operator -> ();

		// Comparison operators
		bool operator == (const CRef<Type>& Ref) const { return (m_Pointer == Ref.m_Pointer); }
		bool operator != (const CRef<Type>& Ref) const { return (m_Pointer != Ref.m_Pointer); }

		// Assignment operators
		const CRef<Type>& operator = (const CRef<Type>& Ref) { BindRef(Ref.m_Pointer); return *this; }
	};

	/*******************************************************
	* Function: CWeakReference::operator CReference()
	* Purpose : Conversion operator for weak references
	* Initial : Max Payne on February 21, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	inline CWeakReference::operator CReference () const
	{
		// Return a root reference to the same object
		return CReference(m_Pointer);
	}

	/*******************************************************
	* Function: CWeakReference::operator CWeakRef<>()
	* Purpose : Conversion operator for weak references
	* Initial : Max Payne on July 9, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> inline CWeakReference::operator CWeakRef<Type> () const
	{
		// Return a specialized reference to the same object
		return CWeakRef<Type>((Type*)m_Pointer);
	}

	/*******************************************************
	* Function: CWeakReference::operator CRef<>()
	* Purpose : Conversion operator for weak references
	* Initial : Max Payne on February 21, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> inline CWeakReference::operator CRef<Type> () const
	{
		// Return a specialized reference to the same object
		return CRef<Type>((Type*)m_Pointer);
	}

	/*******************************************************
	* Function: CReference::operator CWeakReference()
	* Purpose : Conversion operator for root references
	* Initial : Max Payne on February 21, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	inline CReference::operator CWeakReference () const
	{
		// Return a reference to the same object
		return CWeakReference(m_Pointer);
	}

	/*******************************************************
	* Function: CWeakRef::CWeakRef()
	* Purpose : Constructor for typed weak reference type
	* Initial : Max Payne on August 28, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> CWeakRef<Type>::CWeakRef(Type* Pointer)
	{ 
		// Assert the pointer is valid
		assert(Pointer); 

		// Ensure that the type is compatible
		if (Pointer->GetTypeId() != TYPE_NIL && Pointer->GetTypeId() != Type::ClassTypeId())
			throw CRunError("Incompatible reference type: " + Pointer->GetTypeName() + ", expected " + Type::ClassTypeName());
		
		// Assign the pointer
		m_Pointer = (CType*)Pointer;
	}

	/*******************************************************
	* Function: CWeakRef::operator -> () const
	* Purpose : Referencing operator
	* Initial : Max Payne on August 29, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> const Type* CWeakRef<Type>::operator -> () const
	{
		// Ensure that the primitive is not NIL
		if (m_Pointer->GetTypeId() == TYPE_NIL)
			throw CRunError("Cannot reference NIL primitive");

		// Return a const pointer to the primitive
		return (const Type*)m_Pointer;
	}

	/*******************************************************
	* Function: CWeakRef::operator -> ()
	* Purpose : Referencing operator
	* Initial : Max Payne on August 29, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> Type* CWeakRef<Type>::operator -> ()
	{
		// Ensure that the primitive is not NIL
		if (m_Pointer->GetTypeId() == TYPE_NIL)
			throw CRunError("Cannot reference NIL primitive");

		// Return a pointer to the primitive
		return (Type*)m_Pointer;
	}

	/*******************************************************
	* Function: CRef::CRef()
	* Purpose : Constructor for typed reference type
	* Initial : Max Payne on August 28, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> CRef<Type>::CRef(Type* Pointer)
	{
		// Assert the pointer is valid
		assert(Pointer);

		// Ensure that the type is compatible
		if (Pointer->GetTypeId() != TYPE_NIL && Pointer->GetTypeId() != Type::ClassTypeId())
			throw CRunError("Incompatible reference type: " + Pointer->GetTypeName() + ", expected " + Type::ClassTypeName());

		// Bind the reference
		BindRef((CType*)Pointer);
	}

	/*******************************************************
	* Function: CRef::operator -> () const
	* Purpose : Referencing operator
	* Initial : Max Payne on August 29, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> const Type* CRef<Type>::operator -> () const
	{
		// Ensure that the primitive is not NIL
		if (m_Pointer->GetTypeId() == TYPE_NIL)
			throw CRunError("Cannot reference NIL primitive");

		// Return a const pointer to the primitive
		return (const Type*)m_Pointer;
	}

	/*******************************************************
	* Function: CRef::operator -> ()
	* Purpose : Referencing operator
	* Initial : Max Payne on August 29, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	template <class Type> Type* CRef<Type>::operator -> ()
	{
		// Ensure that the primitive is not NIL
		if (m_Pointer->GetTypeId() == TYPE_NIL)
			throw CRunError("Cannot reference NIL primitive");

		// Return a pointer to the primitive
		return (Type*)m_Pointer;
	}
};

#endif // #ifndef _REFERENCES_H_