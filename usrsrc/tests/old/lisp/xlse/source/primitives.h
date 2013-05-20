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
#ifndef _PRIMITIVES_H_
#define _PRIMITIVES_H_

// Header files
#include <vector>
#include <string>
#include <sstream>
#include "basictypes.h"
#include "references.h"
#include "factory.h"
#include "allocator.h"
#include "environment.h"
#include "script.h"

// Include this in the XLSE namespace
namespace XLSE
{
	// Forward declarations
	class CEnvironment;
	class CInterpreter;
	class CScript;
	class CFactory;
	class CType;
	class CNil;
	class CQuote;
	class CPair;
	class CBool;
	class CInt;
	class CFloat;
	class CString;
	class CSymbol;
	class CVector;
	class CMacro;
	class CFunction;
	class CExtMacro;
	class CExtFunction;
	class CModule;

	// Primitive type id definitions
	const TTypeId TYPE_QUOTE		= 1;
	const TTypeId TYPE_PAIR			= 2;
	const TTypeId TYPE_SYMBOL		= 3;
	const TTypeId TYPE_BOOL			= 4;
	const TTypeId TYPE_INT			= 5;
	const TTypeId TYPE_FLOAT		= 6;
	const TTypeId TYPE_STRING		= 7;
	const TTypeId TYPE_VECTOR		= 8;
	const TTypeId TYPE_MACRO		= 9;
	const TTypeId TYPE_FUNCTION		= 10;
	const TTypeId TYPE_EXTMACRO		= 11;
	const TTypeId TYPE_EXTFUNCTION	= 12;
	const TTypeId TYPE_MODULE		= 13;

	// Host macro type definition
	typedef CReference (*THostMacro)(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);

	// Host function type definition
	typedef CReference (*THostFunction)(const CRef<CPair>& Args, CInterpreter& Interpreter);

	/*******************************************************
	* Class   : CQuote
	* Purpose : LISP quote primitive (prevents evaluation)
	* Initial : Max Payne on February 13, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CQuote : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CQuote>;

	public:

		// Method to create a quote object
		static CReference Create(const CWeakReference& Item);

		// Method to produce a string representation
		std::string ToString() const { return "\'" + m_Item->ToString(); }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Item->Copy()); }

		// Method to get the number of outgoing references
		size_t GetNumRefs() const { return 1; }

		// Method to get an outgoing reference
		CWeakReference GetOutRef(size_t Index) const { assert(Index == 0); return m_Item; }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_QUOTE; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "quote"; }

		// Mutator to set the stored item reference
		void SetItem(const CWeakReference& Ref) { m_Item = Ref; }

		// Accessor to get the stored item
		const CWeakReference& GetItem() const { return m_Item; }
		CWeakReference GetItem() { return m_Item; }

	private:

		// Private constructor and destructor
		CQuote() {}
		~CQuote() {}

		// Reference to stored item
		CWeakReference m_Item;
	};

	/*******************************************************
	* Class   : CPair
	* Purpose : Standard LISP pair type
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CPair : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CPair>;

	public:

		// Method to create a pair object
		static CReference Create(const CWeakReference& Car, const CWeakReference& Cdr);

		// Method to produce a string representation
		std::string ToString() const;

		// Method to copy this pair
		CReference Copy() const { return Create(m_Car->Copy(), m_Cdr->Copy()); }

		// Method to get the number of outgoing references
		size_t GetNumRefs() const { return 2; }

		// Method to get an outgoing reference
		CWeakReference GetOutRef(size_t Index) const { assert(Index < 2); if (Index == 0) return m_Car; else return m_Cdr; }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_PAIR; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "pair"; }

		// Predicate to test if this is the empty list
		bool IsEmpty() const { return m_Car.IsNil(); }

		// Mutator to set the car object
		void SetCar(const CWeakReference& Ref) { m_Car = Ref; }

		// Mutator to set the cdr object
		void SetCdr(const CWeakReference& Ref) { m_Cdr = Ref; }

		// Accessor to get the car object
		const CWeakReference& GetCar() const { return m_Car; }
		CWeakReference GetCar() { return m_Car; }

		// Accessor to get the cdr object
		const CWeakReference& GetCdr() const { return m_Cdr; }
		CWeakReference GetCdr() { return m_Cdr; }

	private:

		// Private constructor and destructor
		CPair() {}
		~CPair() {}

		// Car object reference 
		CWeakReference m_Car;

		// Cdr object reference
		CWeakReference m_Cdr;
	};

	/*******************************************************
	* Class   : CSymbol
	* Purpose : Symbol primitive
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CSymbol : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CSymbol>;

	public:

		// Method to create a symbol object
		static CReference Create(const std::string& Value);

		// Method to produce a string representation
		std::string ToString() const { return m_Value; }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Value); }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_SYMBOL; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "symbol"; }

		// Mutator to set the value
		void SetValue(const std::string& Value) { m_Value = Value; }

		// Accessor to get the value
		const std::string& GetValue() const { return m_Value; }

	private:

		// Private constructor and destructor
		CSymbol() {}
		~CSymbol() {}

		// Value of this object
		std::string m_Value;
	};

	/*******************************************************
	* Class   : CBool
	* Purpose : Boolean primitive
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CBool : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CBool>;

	public:

		// Method to create a boolean object
		static CReference Create(bool Value);

		// Method to produce a string representation
		std::string ToString() const { return m_Value? "#T":"#F"; }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Value); }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_BOOL; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "boolean"; }

		// Mutator to set the value
		void SetValue(bool Value) { m_Value = Value; }

		// Accessor to get the value
		bool GetValue() const { return m_Value; }

	private:

		// Private constructor and destructor
		CBool() : m_Value(false) {}
		~CBool() {}

		// Value of this object
		bool m_Value;
	};

	/*******************************************************
	* Class   : CInt
	* Purpose : Integer primitive
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CInt : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CInt>;

	public:

		// Method to create an integer object
		static CReference Create(int32 Value);

		// Method to produce a string representation
		std::string ToString() const { std::stringstream SStream; SStream << m_Value; return SStream.str(); }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Value); }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_INT; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "integer"; }

		// Mutator to set the value
		void SetValue(int32 Value) { m_Value = Value; }

		// Accessor to get the value
		int32 GetValue() const { return m_Value; }

	private:

		// Private constructor and destructor
		CInt() : m_Value(0) {}
		~CInt() {}

		// Value of this object
		int32 m_Value;
	};

	/*******************************************************
	* Class   : CFloat
	* Purpose : Floating-point primitive
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CFloat : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CFloat>;

	public:

		// Method to create a float object
		static CReference Create(float32 Value);

		// Method to produce a string representation
		std::string ToString() const { std::stringstream SStream; SStream << m_Value; return SStream.str(); }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Value); }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_FLOAT; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "float"; }

		// Mutator to set the value
		void SetValue(float32 Value) { m_Value = Value; }

		// Accessor to get the value
		float32 GetValue() const { return m_Value; }

	private:

		// Private constructor and destructor
		CFloat() : m_Value(0.0f) {}
		~CFloat() {}

		// Value of this object
		float32 m_Value;
	};

	/*******************************************************
	* Class   : CSring
	* Purpose : String primitive
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CString : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CString>;

	public:

		// Method to create an integer object
		static CReference Create(const std::string& Value);

		// Method to produce a string representation
		std::string ToString() const { return "\"" + m_Value + "\""; }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Value); }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_STRING; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "string"; }

		// Mutator to set the value
		void SetValue(const std::string& Value) { m_Value = Value; }

		// Accessor to get the value
		const std::string& GetValue() const { return m_Value; }

	private:

		// Private constructor and destructor
		CString() {}
		~CString() {}

		// Value of this object
		std::string m_Value;
	};

	/*******************************************************
	* Class   : CVector
	* Purpose : Vector primitive
	* Initial : Max Payne on February 13, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CVector : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CVector>;

	public:

		// Method to create a vector object
		static CReference Create();

		// Method to produce a string representation
		std::string ToString() const;

		// Method to copy this primitive
		CReference Copy() const;

		// Method to get the number of outgoing references
		size_t GetNumRefs() const { return m_Vector.size(); }

		// Method to get an outgoing reference
		CWeakReference GetOutRef(size_t Index) const { assert(Index < m_Vector.size()); return m_Vector[Index]; }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_VECTOR; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "vector"; }

		// Method to add an item to the vector
		void AddItem(const CWeakReference& Item) { m_Vector.push_back(Item); }

		// Mutator to set the member vector
		void SetVector(const std::vector<CWeakReference>& Vector) { m_Vector = Vector; }

		// Accessor to get a reference to the vector
		const std::vector<CWeakReference>& GetVector() const { return m_Vector; }

	private:

		// Private constructor and destructor
		CVector() {}
		~CVector() {}

		// Vector of list items
		std::vector<CWeakReference> m_Vector;
	};

	/*******************************************************
	* Class   : CMacro
	* Purpose : Macro primitive
	* Initial : Max Payne on September 17, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CMacro : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CMacro>;

	public:

		// Method to create an external macro object
		static CReference Create(const CEnvironment& Env, const std::vector<std::string>& ArgNames, const std::vector<CWeakReference>& Body, bool VarArgs);

		// Method to produce a string representation
		std::string ToString() const { return "<MACRO>"; }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Environment, m_ArgNames, m_Body, m_VariableArgs); }

		// Method to get the number of outgoing references
		size_t GetNumRefs() const { return m_Body.size(); }

		// Method to get an outgoing reference
		CWeakReference GetOutRef(size_t Index) const { assert(Index < m_Body.size()); return m_Body[Index]; }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_MACRO; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "macro"; }

		// Method to call the macro
		CReference Call(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);

		// Mutator to set the environment
		void SetEnvironment(const CEnvironment& Env) { m_Environment = Env; }

		// Mutator to set the argument names
		void SetArguments(const std::vector<std::string>& ArgNames) { m_ArgNames = ArgNames; }

		// Mutator to set the macro body
		void SetBody(const std::vector<CWeakReference>& Body) { m_Body = Body; }

		// Mutator to set the variable argument flag
		void SetVarArgs(bool VarArgs) { m_VariableArgs = VarArgs; }

	private:

		// Private constructor and destructor
		CMacro() {}
		~CMacro() {}

		// Method to create a thunk closure object
		CReference CreateThunk(CEnvironment& Env, const CReference& Item);

		// Environment in which to evaluate the macro
		CEnvironment m_Environment;

		// Vector of argument names
		std::vector<std::string> m_ArgNames;

		// Body to be evaluated
		std::vector<CWeakReference> m_Body;

		// Variable number of argument flag
		bool m_VariableArgs;
	};

	/*******************************************************
	* Class   : CFunction
	* Purpose : Function primitive (lambda expression)
	* Initial : Max Payne on August 29, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CFunction : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CFunction>;

	public:

		// Method to create an external macro object
		static CReference Create(const CEnvironment& Env, const std::vector<std::string>& ArgNames, const std::vector<CWeakReference>& Body, bool VarArgs);

		// Method to produce a string representation
		std::string ToString() const { return "<FUNCTION>"; }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Environment, m_ArgNames, m_Body, m_VariableArgs); }

		// Method to get the number of outgoing references
		size_t GetNumRefs() const { return m_Body.size(); }

		// Method to get an outgoing reference
		CWeakReference GetOutRef(size_t Index) const { assert(Index < m_Body.size()); return m_Body[Index]; }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_FUNCTION; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "function"; }

		// Method to call the function
		CReference Call(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);

		// Mutator to set the environment
		void SetEnvironment(const CEnvironment& Env) { m_Environment = Env; }

		// Mutator to set the argument names
		void SetArguments(const std::vector<std::string>& ArgNames) { m_ArgNames = ArgNames; }

		// Mutator to set the function body
		void SetBody(const std::vector<CWeakReference>& Body) { m_Body = Body; }

		// Mutator to set the variable argument flag
		void SetVarArgs(bool VarArgs) { m_VariableArgs = VarArgs; }

	private:

		// Private constructor and destructor
		CFunction() {}
		~CFunction() {}

		// Environment in which to evaluate the function
		CEnvironment m_Environment;

		// Vector of argument names
		std::vector<std::string> m_ArgNames;

		// Body to be evaluated
		std::vector<CWeakReference> m_Body;

		// Variable number of argument flag
		bool m_VariableArgs;
	};

	/*******************************************************
	* Class   : CExtMacro
	* Purpose : External macro primitive
	* Initial : Max Payne on February 15, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CExtMacro : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CExtMacro>;

	public:

		// Method to create an external macro object
		static CReference Create(THostMacro pFunction);

		// Method to produce a string representation
		std::string ToString() const { return "<EXTERN_MACRO>"; }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_pFunction); }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_EXTMACRO; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "extern_macro"; }

		// Method to call the host function
		CReference Call(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);

		// Mutator to set the function
		void SetFunction(THostMacro pFunction) { m_pFunction = pFunction; }

	private:

		// Private constructor and destructor
		CExtMacro() {}
		~CExtMacro() {}

		// Pointer to host function
		THostMacro m_pFunction;
	};

	/*******************************************************
	* Class   : CExtFunction
	* Purpose : External function primitive
	* Initial : Max Payne on February 15, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CExtFunction : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CExtFunction>;

	public:

		// Method to create an external function object
		static CReference Create(THostFunction pFunction);

		// Method to produce a string representation
		std::string ToString() const { return "<EXTERN_FUNCTION>"; }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_pFunction); }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_EXTFUNCTION; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "extern_function"; }

		// Method to call the host function
		CReference Call(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);

		// Mutator to set the function
		void SetFunction(THostFunction pFunction) { m_pFunction = pFunction; }

	private:

		// Private constructor and destructor
		CExtFunction() {}
		~CExtFunction() {}

		// Pointer to host function
		THostFunction m_pFunction;
	};

	/*******************************************************
	* Class   : CModule
	* Purpose : Module first-class primitive
	* Initial : Max Payne on September 30, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CModule : public CType
	{
		// Declare the allocator specific to this type as friend
		friend class CAllocator<CModule>;

	public:

		// Method to create a module object
		static CReference Create(const CScript& Script);

		// Method to produce a string representation
		std::string ToString() const { return "<MODULE>"; }

		// Method to copy this primitive
		CReference Copy() const { return Create(m_Script); }

		// Method to get the type id of this object
		TTypeId GetTypeId() const { return ClassTypeId(); }

		// Method to get the type id of this class
		static TTypeId ClassTypeId() { return TYPE_MODULE; }

		// Method to get the type name of this object
		virtual std::string GetTypeName() const { return ClassTypeName(); }

		// Method to get the type name of this class
		static std::string ClassTypeName() { return "module"; }

		// Method to change the internal script
		void SetScript(const CScript& Script) { m_Script = Script; }

	private:

		// Private constructor and destructor
		CModule() {}
		~CModule() {}

		// Script object
		CScript m_Script;
	};

	/*******************************************************
	* Class   : CSListIterator
	* Purpose : Iterate over the elements of an s-list
	* Initial : Max Payne on July 4, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CSListIterator
	{
	public:

		// Constructors
		CSListIterator(const CWeakReference& FirstPair);
		CSListIterator() {}

		// Incrementation operator
		CSListIterator& operator ++ ();

		// Post-incrementation operator
		CSListIterator operator ++ (int);

		// Operator to access the current object
		const CWeakReference& operator -> () const { return m_Reference->GetCar(); }

		// Dereferencing operator
		const CWeakReference& operator * () const { return m_Reference->GetCar(); }

		// Iterator comparison operator
		bool operator == (const CSListIterator& Itr) const { return (m_Reference == Itr.m_Reference); }

		// Accessor to test if this iterator is valid
		bool IsValid() const { return !m_Reference.IsNil(); }

		// Accessor to test if there is a next element
		bool HasNext() const { return !m_Reference->GetCdr().IsNil(); }

	private:

		// Reference to the first pair in the list
		CRef<CPair> m_FirstPair;

		// Reference to the current pair in the list
		CWeakRef<CPair> m_Reference;
	};
};

#endif // #ifndef _PRIMITIVES_H_