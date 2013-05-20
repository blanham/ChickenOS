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
#include "primitives.h"
#include "allocator.h"
#include "references.h"
#include "interpreter.h"

// Include the XLSE namespace
using namespace XLSE;

// Global null type instance
namespace XLSE { CWeakReference NIL = &CNil::GetInstance(); };

// Static NIL singleton instance
CNil CNil::s_Instance;

/*******************************************************
* Function: CRunError::CRunError()
* Purpose : Constructor for runtime error class
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CRunError::CRunError(const std::string& ErrorText, const CReference& ErrorCode)
{
	// Add the first level to the error trace
	PushLevel(ErrorText, ErrorCode);
}

/*******************************************************
* Function: CRunError::PushLevel()
* Purpose : Add another level to the error trace
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
void CRunError::PushLevel(const std::string& ErrorText, const CReference& ErrorCode)
{
	// Create a new trace level object
	STraceLevel NewLevel;

	// Set the error text and code
	NewLevel.ErrorText = ErrorText;
	NewLevel.ErrorCode = ErrorCode;

	// Add a new level to the error trace
	m_Trace.push_back(NewLevel);
}

/*******************************************************
* Function: CRunError::GetErrorText()
* Purpose : Produce a text representation of the error
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
std::string CRunError::GetErrorText() const
{
	// Declare a string for the error representation
	std::string ErrorText;

	// For each trace level
	for (TErrorTrace::const_reverse_iterator TraceItr = m_Trace.rbegin(); TraceItr != m_Trace.rend(); ++TraceItr)
	{
		// Add the error text to the error description
		ErrorText += TraceItr->ErrorText;

		// If error code was provided
		if (TraceItr->ErrorCode)
		{
			// Add the error code to the error description
			ErrorText += ":\n\t" + TraceItr->ErrorCode->ToString();
		}

		// Separate from the next entry
		ErrorText += "\n";
	}

	// Return the error text
	return ErrorText;
}

/*******************************************************
* Function: CQuote::Create()
* Purpose : Static method to create a quote object
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CQuote::Create(const CWeakReference& Item)
{
	// Create a new pair object
	CRef<CQuote> NewQuote = CFactory::GetInstance().CreateType(TYPE_QUOTE);

	// Set the item reference
	NewQuote->SetItem(Item);

	// Return a reference to the quote object
	return NewQuote;
}

/*******************************************************
* Function: CPair::Create()
* Purpose : Static method to create a pair object
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CPair::Create(const CWeakReference& Car, const CWeakReference& Cdr)
{
	// Create a new pair object
	CRef<CPair> NewPair = CFactory::GetInstance().CreateType(TYPE_PAIR);

	// Set the car and cdr item references
	NewPair->SetCar(Car);
	NewPair->SetCdr(Cdr);

	// Return a reference to the new pair object
	return NewPair;
}

/*******************************************************
* Function: CPair::ToString()
* Purpose : Create a string representation of a list
* Initial : Max Payne on February 10, 2006
********************************************************
Revisions and bug fixes:
*/
std::string CPair::ToString() const
{
	// Declare a string to store the list
	std::string ListString = "(";

	// For each list element
	for (CRef<const CPair> Current(this); Current; Current = Current->m_Cdr)
	{
		// If this is not the starting pair
		if (Current != this)
		{
			// Add a space separator
			ListString += ' ';
		}

		// Add this list item to the string
		ListString += Current->m_Car->ToString();

		// If there is a next object and it is not a pair
		if (Current->m_Cdr && Current->m_Cdr->GetTypeId() != TYPE_PAIR)
		{
			// Add a dot separator
			ListString += " . ";

			// Add the object to the list
			ListString += Current->m_Cdr->ToString();

			// This is the end of the list
			break;
		}
	}

	// End the list and return it
	return ListString + ")";
}

/*******************************************************
* Function: CSymbol::Create()
* Purpose : Static method to create a symbol object
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CSymbol::Create(const std::string& Value)
{
	// Create a new symbol object
	CRef<CSymbol> NewSymbol = CFactory::GetInstance().CreateType(TYPE_SYMBOL);

	// Set the symbol value
	NewSymbol->SetValue(Value);

	// Return a reference to the new symbol object
	return NewSymbol;
}

/*******************************************************
* Function: CBool::Create()
* Purpose : Static method to create a boolean object
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CBool::Create(bool Value)
{
	// Create a new boolean object
	CRef<CBool> NewBool = CFactory::GetInstance().CreateType(TYPE_BOOL);

	// Set the boolean value
	NewBool->SetValue(Value);

	// Return a reference to the new object
	return NewBool;
}

/*******************************************************
* Function: CInt::Create()
* Purpose : Static method to create an integer object
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CInt::Create(int32 Value)
{
	// Create a new integer object
	CRef<CInt> NewInt = CFactory::GetInstance().CreateType(TYPE_INT);

	// Set the integer value
	NewInt->SetValue(Value);

	// Return a reference to the new object
	return NewInt;
}

/*******************************************************
* Function: CFloat::Create()
* Purpose : Static method to create a float object
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CFloat::Create(float32 Value)
{
	// Create a new float object
	CRef<CFloat> NewFloat = CFactory::GetInstance().CreateType(TYPE_FLOAT);

	// Set the float value
	NewFloat->SetValue(Value);

	// Return a reference to the new object
	return NewFloat;
}

/*******************************************************
* Function: CString::Create()
* Purpose : Static method to create a string object
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CString::Create(const std::string& Value)
{
	// Create a new string object
	CRef<CString> NewString = CFactory::GetInstance().CreateType(TYPE_STRING);

	// Set the string value
	NewString->SetValue(Value);

	// Return a reference to the new object
	return NewString;
}

/*******************************************************
* Function: CVector::Create()
* Purpose : Static method to create an integer object
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CVector::Create()
{
	// Create a new vector object
	CRef<CVector> NewVector = CFactory::GetInstance().CreateType(TYPE_VECTOR);

	// Return a reference to the new object
	return NewVector;
}

/*******************************************************
* Function: CVector::ToString()
* Purpose : Produce a string representation
* Initial : Max Payne on February 13, 2006
********************************************************
Revisions and bug fixes:
*/
std::string CVector::ToString() const
{
	// Begin the string representation
	std::string VectorString = "#(";

	// For each contained element
	for (std::vector<CWeakReference>::const_iterator ElemItr = m_Vector.begin(); ElemItr != m_Vector.end(); ++ElemItr)
	{
		// If this is not the first element, separate by a space
		if (ElemItr != m_Vector.begin())
			VectorString += " ";

		// Add this element to the string representation
		VectorString += (*ElemItr)->ToString();
	}

	// End the vector
	return VectorString + ")";
}

/*******************************************************
* Function: CVector::Copy()
* Purpose : Copy this s-list vector object
* Initial : Max Payne on February 14, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CVector::Copy() const
{
	// Create a new vector object
	CRef<CVector> NewVector = CFactory::GetInstance().CreateType(TYPE_VECTOR);

	// Create a vector for the copied objects
	std::vector<CReference> CopyVector;

	// For each stored object
	for (std::vector<CWeakReference>::const_iterator ItemItr = m_Vector.begin(); ItemItr != m_Vector.end(); ++ItemItr)
	{
		// Copy this item
		CReference CopyItem = (*ItemItr)->Copy();

		// Add the copy to the vector
		NewVector->AddItem(CopyItem);
	}

	// Return a reference to the new object
	return NewVector;
}

/*******************************************************
* Function: CMacro::Create()
* Purpose : Static method to create a macro object
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CMacro::Create(const CEnvironment& Env, const std::vector<std::string>& Args, const std::vector<CWeakReference>& Body, bool VarArgs)
{
	// Create a new macro object
	CRef<CMacro> NewMacro = CFactory::GetInstance().CreateType(TYPE_MACRO);

	// Set the macro object attributes
	NewMacro->SetEnvironment(Env);
	NewMacro->SetArguments(Args);
	NewMacro->SetBody(Body);
	NewMacro->SetVarArgs(VarArgs);

	// Return a reference to the new object
	return NewMacro;
}

/*******************************************************
* Function: CMacro::Call()
* Purpose : Call a macro object
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CMacro::Call(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Extend the current environment for the arguments
	CEnvironment ArgumentEnv = Env.Extend();

	// If this function has a variable number of arguments
	if (m_VariableArgs)
	{
		// Declare references to the first and last pair of the argument list
		CRef<CPair> FirstPair = NIL;
		CRef<CPair> LastPair  = NIL;

		// For each argument
		for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
		{
			// Create a thunk to store the unevaluated argument
			CReference Thunk = CreateThunk(Env, *ArgItr);

			// Create a new pair in the list to store this unevaluated argument
			CRef<CPair> NewPair = CPair::Create(Thunk, NIL);

			// If there are no pairs in the list
			if (!LastPair)
			{
				// Make this the first pair
				FirstPair = LastPair = NewPair;
			}
			else
			{
				// Set a reference from the last pair to this one
				LastPair->SetCdr(NewPair);

				// Make the last pair the new pair
				LastPair = NewPair;
			}
		}

		// Bind the argument name to the unevaluated argument list
		ArgumentEnv.Bind(m_ArgNames.front(), FirstPair);
	}

	// Otherwise, this function has a fixed number of arguments
	else
	{
		// Create an iterator for the argument names
		std::vector<std::string>::iterator NameItr = m_ArgNames.begin();

		// For each argument
		for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
		{
			// If there are too many arguments
			if (NameItr == m_ArgNames.end())
			{
				// Throw an exception
				throw CRunError("Invalid argument count");
			}

			// Create a thunk to store the unevaluated argument
			CReference Thunk = CreateThunk(Env, *ArgItr);

			// Bind the argument name to its unevaluated value
			ArgumentEnv.Bind(*NameItr, Thunk);

			// Move to the next argument name
			++NameItr;
		}

		// If there are not enough arguments
		if (NameItr != m_ArgNames.end())
		{
			// Throw an exception
			throw CRunError("Invalid argument count");
		}
	}

	// Declare a reference for the result
	CReference Result;

	// For each term of the macro body
	for (std::vector<CWeakReference>::iterator TermItr = m_Body.begin(); TermItr != m_Body.end(); ++TermItr)
	{
		// Evaluate the term in the extended environment
		Result = Interpreter.EvalPrimitive(*TermItr, ArgumentEnv);
	}

	// Return the result
	return Result;
}

/*******************************************************
* Function: CMacro::CreateThunk()
* Purpose : Create a thunk closure function
* Initial : Max Payne on September 21, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CMacro::CreateThunk(CEnvironment& Env, const CReference& Item)
{
	// Create a vector for the thunk body
	std::vector<CWeakReference> ThunkBody;

	// Add the item to the thunk body
	ThunkBody.push_back(Item);

	// Create and return the thunk function
	return CFunction::Create(Env, std::vector<std::string>(), ThunkBody, false);
}

/*******************************************************
* Function: CFunction::Create()
* Purpose : Static method to create a function object
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CFunction::Create(const CEnvironment& Env, const std::vector<std::string>& Args, const std::vector<CWeakReference>& Body, bool VarArgs)
{
	// Create a new function object
	CRef<CFunction> NewFunction = CFactory::GetInstance().CreateType(TYPE_FUNCTION);

	// Set the function object attributes
	NewFunction->SetEnvironment(Env);
	NewFunction->SetArguments(Args);
	NewFunction->SetBody(Body);
	NewFunction->SetVarArgs(VarArgs);

	// Return a reference to the new object
	return NewFunction;
}

/*******************************************************
* Function: CFunction::Call()
* Purpose : Call a function object (lambda expression)
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CFunction::Call(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Extend the original environment for the arguments
	CEnvironment ArgumentEnv = m_Environment.Extend();

	// If this function has a variable number of arguments
	if (m_VariableArgs)
	{
		// Declare references to the first and last pair of the argument list
		CRef<CPair> FirstPair = NIL;
		CRef<CPair> LastPair  = NIL;

		// For each argument
		for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
		{
			// Evaluate this argument
			CReference Result = Interpreter.EvalPrimitive(*ArgItr, Env);	

			// Create a new pair in the list to store this result
			CRef<CPair> NewPair = CPair::Create(Result, NIL);

			// If there are no pairs in the list
			if (!LastPair)
			{
				// Make this the first pair
				FirstPair = LastPair = NewPair;
			}
			else
			{
				// Set a reference from the last pair to this one
				LastPair->SetCdr(NewPair);

				// Make the last pair the new pair
				LastPair = NewPair;
			}
		}

		// Bind the argument name to the evaluated argument list
		ArgumentEnv.Bind(m_ArgNames.front(), FirstPair);
	}

	// Otherwise, this function has a fixed number of arguments
	else
	{
		// Create an iterator for the argument names
		std::vector<std::string>::iterator NameItr = m_ArgNames.begin();

		// For each argument
		for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
		{
			// If there are too many arguments
			if (NameItr == m_ArgNames.end())
			{
				// Throw an exception
				throw CRunError("Invalid argument count");
			}

			// Evaluate this argument
			CReference Value = Interpreter.EvalPrimitive(*ArgItr, Env);

			// Bind the argument name to its value
			ArgumentEnv.Bind(*NameItr, Value);

			// Move to the next argument name
			++NameItr;
		}

		// If there are not enough arguments
		if (NameItr != m_ArgNames.end())
		{
			// Throw an exception
			throw CRunError("Invalid argument count");
		}
	}

	// Declare a reference for the result
	CReference Result;

	// For each term of the function body
	for (std::vector<CWeakReference>::iterator TermItr = m_Body.begin(); TermItr != m_Body.end(); ++TermItr)
	{
		// Evaluate the term in the extended environment
		Result = Interpreter.EvalPrimitive(*TermItr, ArgumentEnv);
	}

	// Return the result
	return Result;
}

/*******************************************************
* Function: CExtMacro::Create()
* Purpose : Static method to create an external macro
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CExtMacro::Create(THostMacro pFunction)
{
	// Create a new external macro object
	CRef<CExtMacro> NewExtMacro = CFactory::GetInstance().CreateType(TYPE_EXTMACRO);

	// Set the function pointer
	NewExtMacro->SetFunction(pFunction);

	// Return a reference to the new object
	return NewExtMacro;
}

/*******************************************************
* Function: CExtMacro::Call()
* Purpose : Call an external macro
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CExtMacro::Call(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Call the macro with the argument list
	return m_pFunction(Args, Env, Interpreter);
}

/*******************************************************
* Function: CExtFunction::Create()
* Purpose : Static method to create an external function
* Initial : Max Payne on July 9, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CExtFunction::Create(THostFunction pFunction)
{
	// Create a new external function object
	CRef<CExtFunction> NewExtFunction = CFactory::GetInstance().CreateType(TYPE_EXTFUNCTION);

	// Set the function pointer
	NewExtFunction->SetFunction(pFunction);

	// Return a reference to the new object
	return NewExtFunction;
}

/*******************************************************
* Function: CExtFunction::Call()
* Purpose : Call an external function
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CExtFunction::Call(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Declare references to the first and last pair of the argument list
	CRef<CPair> FirstPair = NIL;
	CRef<CPair> LastPair  = NIL;

	// For each argument
	for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// Evaluate this argument
		CReference Result = Interpreter.EvalPrimitive(*ArgItr, Env);	

		// Create a new pair in the list to store this result
		CRef<CPair> NewPair = CPair::Create(Result, NIL);

		// If there are no pairs in the list
		if (!LastPair)
		{
			// Make this the first pair
			FirstPair = LastPair = NewPair;
		}
		else
		{
			// Set a reference from the last pair to this one
			LastPair->SetCdr(NewPair);

			// Make the last pair the new pair
			LastPair = NewPair;
		}
	}

	// Call the function with the evaluated argument list
	return m_pFunction(FirstPair, Interpreter);
}

/*******************************************************
* Function: CSListIterator::CSListIterator()
* Purpose : Constructor for list iterator
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CSListIterator::CSListIterator(const CWeakReference& FirstPair)
: m_FirstPair((CReference)FirstPair),
  m_Reference((CReference)FirstPair)
{
}

/*******************************************************
* Function: CSListIterator::operator ++ ()
* Purpose : Pre-incrementation operator
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CSListIterator& CSListIterator::operator ++ ()
{
	// Attempt to move to the next object
	try
	{
		m_Reference = m_Reference->GetCdr();
	}
	
	// Catch any error
	catch (CRunError Error)
	{
		// Add a level to the error trace
		Error.PushLevel("List iterator incrementation failed - malformed list", m_FirstPair);

		// Rethrow the error
		throw Error;
	}
	
	// Return a reference to the incremented iterator
	return *this; 
}

/*******************************************************
* Function: CSListIterator::operator ++ (int)
* Purpose : Post-incrementation operator
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CSListIterator CSListIterator::operator ++ (int)
{
	// Store the current iterator
	CSListIterator Itr = *this;
	
	// Attempt to move to the next object
	try
	{
		m_Reference = m_Reference->GetCdr();
	}
	
	// Catch any error
	catch (CRunError Error)
	{
		// Add a level to the error trace
		Error.PushLevel("List iterator incrementation failed", m_FirstPair);

		// Rethrow the error
		throw Error;
	}
	
	// Return a reference to the unincremented iterator
	return Itr;
}