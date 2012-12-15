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
#ifndef _TYPES_H_
#define _TYPES_H_
 
// Header files
#include <string>
#include <vector>
#include "basictypes.h"

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

	// Script type id definition
	typedef uint32 TTypeId;

	// NIL type id definition
	const TTypeId TYPE_NIL = 0;

	// Global null type instance
	extern CWeakReference NIL;

	/*******************************************************
	* Class   : CWeakReference
	* Purpose : Indirec reference to an arbitrary type
	* Initial : Max Payne on February 18, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CWeakReference
	{
		// Declare the script type factory as friend
		friend class CFactory;

	public:

		// Constructors and destructor
		CWeakReference(CType* pType) : m_Pointer(pType) { assert(pType != NULL); }
		CWeakReference(const CWeakReference& Ref) : m_Pointer(Ref.m_Pointer) {}
		CWeakReference();
		virtual ~CWeakReference() {}

		// Method to nullify the reference
		void Nullify();

		// Method to test if the reference is nil
		bool IsNil() const;

		// Operators to access the referenced object
		const CType* operator -> () const { return m_Pointer; }
		CType* operator -> () { return m_Pointer; }

		// Operator to test for nullity
		operator bool () const { return !IsNil(); }

		// Comparison operators
		bool operator == (const CWeakReference& Ref) const { return (m_Pointer == Ref.m_Pointer); }
		bool operator != (const CWeakReference& Ref) const { return (m_Pointer != Ref.m_Pointer); }

		// Assignment operator
		const CWeakReference& operator = (const CWeakReference& Ref) { m_Pointer = Ref.m_Pointer; return *this; }

		// Conversion operators
		inline operator CReference () const;
		template <class Type> inline operator CWeakRef<Type> () const;
		template <class Type> inline operator CRef<Type> () const;

	protected:

		// Accessor to get the pointer (only called by factory)
		CType* GetPointer() const { return m_Pointer; }

		// Pointer to a script type
		CType* m_Pointer;
	};

	/*******************************************************
	* Class   : CReference
	* Purpose : Direct reference to a script type
	* Initial : Max Payne on February 21, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CReference : public CWeakReference
	{
	public:

		// Constructors and destructor
		CReference(CType* pType);
		CReference(const CReference& Ref);
		CReference();
		virtual ~CReference();

		// Assignment operator
		const CReference& operator = (const CReference& Ref) { BindRef(Ref.m_Pointer); return *this; }

		// Conversion operators
		inline operator CWeakReference () const;

	protected:

		// Method to bind this root reference
		void BindRef(CType* pType);
	};

	/*******************************************************
	* Class   : CRunError
	* Purpose : Runtime error exception
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CRunError
	{
	public:

		// Constructor
		CRunError(const std::string& ErrorText, const CReference& ErrorCode = NIL);

		// Push another level of error trace
		void PushLevel(const std::string& ErrorText, const CReference& ErrorCode = NIL);

		// Accessor to get the error description text
		std::string GetErrorText() const;

	private:

		// Trace level structure
		struct STraceLevel
		{
			// Error description text
			std::string ErrorText;

			// Erroneous code segment
			CReference ErrorCode;
		};

		// Error trace type definition
		typedef std::vector<STraceLevel> TErrorTrace;

		// Error trace
		TErrorTrace m_Trace;
	};

	/*******************************************************
	* Class   : CType
	* Purpose : Base class for all script types
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CType
	{
		// Declare the factory class as a friend
		friend class CFactory;

	public:

		// Constructor and destructor
		CType() {}
		virtual ~CType() {}

		// Method to produce a string representation
		virtual std::string ToString() const = 0;

		// Method to copy this list item
		virtual CReference Copy() const = 0;

		// Method to get the number of outgoing references
		virtual size_t GetNumRefs() const { return 0; }

		// Method to get an outgoing reference
		virtual CWeakReference GetOutRef(size_t Index) const { assert(false); return NIL; }

		// Method to get the type id of this object
		virtual TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_NIL; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "NIL"; }
	};

	/*******************************************************
	* Class   : CNil
	* Purpose : LISP null primitive singleton class
	* Initial : Max Payne on February 13, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CNil : public CType
	{
	public:

		// Method to produce a string representation
		std::string ToString() const { return "()"; }

		// Method to copy this primitive
		CReference Copy() const { return NIL; }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_NIL; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "NIL"; }

		// Comparison operators
		bool operator == (const CWeakReference& Primitive) { return (Primitive->GetTypeId() == TYPE_NIL); }
		bool operator != (const CWeakReference& Primitive) { return (Primitive->GetTypeId() != TYPE_NIL); }

		// Static method to access the NIL instance
		static CNil& GetInstance() { return s_Instance; }

	private:

		// private constructor and destructor
		CNil() {}
		~CNil() {}

		// Static instance of the NIL primitive
		static CNil s_Instance;
	};
};

#endif // #ifndef _TYPES_H_