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
#include "lisp.h"

// Include the XLSE namespace
using namespace XLSE;

/*******************************************************
* Function: CLISPCore::CLISPCore()
* Purpose : Constructor for LISP core class
* Initial : Max Payne on July 5, 2006
********************************************************
Revisions and bug fixes:
*/
CLISPCore::CLISPCore()
{
	// Debug/error management primitives
	Bind("assert", CExtFunction::Create(func_assert));

	// Environment manipulation primitives
	Bind("define", CExtMacro::Create(macro_define));
	Bind("set", CExtMacro::Create(macro_set));

	// Function/macro definition primitives
	Bind("macro", CExtMacro::Create(macro_macro));
	Bind("lambda", CExtMacro::Create(macro_lambda));

	// Control flow primitives
	Bind("if", CExtMacro::Create(macro_if));
	Bind("cond", CExtMacro::Create(macro_cond));
	Bind("while", CExtMacro::Create(macro_while));

	// Logical operators
	Bind("=", CExtFunction::Create(func_equals));
	Bind("<", CExtFunction::Create(func_lt));
	Bind("<=", CExtFunction::Create(func_lte));
	Bind(">", CExtFunction::Create(func_gt));
	Bind(">=", CExtFunction::Create(func_gte));
	Bind("and", CExtFunction::Create(func_and));
	Bind("or", CExtFunction::Create(func_or));
	Bind("not", CExtFunction::Create(func_not));

	// Pair/s-list manipulation operators
	Bind("cons", CExtFunction::Create(func_cons));
	Bind("car", CExtFunction::Create(func_car));
	Bind("cdr", CExtFunction::Create(func_cdr));
	Bind("list", CExtFunction::Create(func_list));

	// Arithmetic operators
	Bind("+", CExtFunction::Create(func_add));
	Bind("-", CExtFunction::Create(func_sub));
	Bind("*", CExtFunction::Create(func_mul));
	Bind("/", CExtFunction::Create(func_div));
	Bind("%", CExtFunction::Create(func_mod));

	// String functions and operators
	Bind("string", CExtFunction::Create(func_string));
	Bind("strsub", CExtFunction::Create(func_strsub));
	Bind("strlen", CExtFunction::Create(func_strlen));

	// Vector operators
	Bind("vector", CExtFunction::Create(func_vector));
	Bind("vector-ref", CExtFunction::Create(func_vector_ref));
	Bind("vector-length", CExtFunction::Create(func_vector_length));

	// Type testing predicates
	Bind("nil?", CExtFunction::Create(func_nilp));
	Bind("pair?", CExtFunction::Create(func_pairp));
	Bind("list?", CExtFunction::Create(func_listp));
	Bind("symbol?", CExtFunction::Create(func_symbolp));
	Bind("bool?", CExtFunction::Create(func_boolp));
	Bind("int?", CExtFunction::Create(func_intp));
	Bind("float?", CExtFunction::Create(func_floatp));
	Bind("string?", CExtFunction::Create(func_stringp));
	Bind("vector?", CExtFunction::Create(func_vectorp));
	Bind("function?", CExtFunction::Create(func_functionp));
}

/*******************************************************
* Function: static CLISPCore::func_assert()
* Purpose : Assertion debug primitive
* Initial : Max Payne on September 21, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_assert(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Create an iterator for the arguments
	CSListIterator ArgItr = Args;

	// Get a reference to the test value
	CRef<CBool> TestValue = *(ArgItr++);

	// If the test value is false
	if (TestValue->GetValue() == false)
	{
		// Declare a string for the error string
		std::string ErrorString = "Assertion Failed";

		// If there is an error description
		if (ArgItr.IsValid())
		{
			// Get a reference to the error description string
			CRef<CString> Description = *ArgItr;

			// Add the description to the error string
			ErrorString += ": " + Description->GetValue();
		}

		// Throw an exception with the error string
		throw CRunError(ErrorString);
	}

	// Return NIL
	return NIL;
}

/*******************************************************
* Function: static CLISPCore::macro_define()
* Purpose : Global definition macro
* Initial : Max Payne on August 30, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::macro_define(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Create an argument iterator for the list
	CSListIterator ArgItr = Args;

	// Extract the name
	CRef<CSymbol> Name = *(ArgItr++);

	// Evaluate the primitive
	CReference Primitive = Interpreter.EvalPrimitive(*ArgItr, Env);

	// Set the global variable
	Interpreter.SetGlobal(Name->GetValue(), Primitive);

	// Return the evaluated primitive
	return Primitive;
}

/*******************************************************
* Function: static CLISPCore::macro_set()
* Purpose : Variable assignment macro
* Initial : Max Payne on August 30, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::macro_set(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Create an argument iterator for the list
	CSListIterator ArgItr = Args;

	// Extract the name
	CRef<CSymbol> Name = *(ArgItr++);

	// Evaluate the primitive
	CReference Primitive = Interpreter.EvalPrimitive(*ArgItr, Env);

	// Attempt to set the binding
	bool Result = Env.Set(Name->GetValue(), Primitive);

	// If the binding was not found
	if (!Result)
	{
		// Create a new binding at this level
		Env.Bind(Name->GetValue(), Primitive);
	}

	// Return the evaluated primitive
	return Primitive;
}

/*******************************************************
* Function: static CLISPCore::macro_macro()
* Purpose : Define a macro
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::macro_macro(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Create an argument iterator for the list
	CSListIterator ArgItr = Args;	

	// Extract the argument names
	CReference Arguments = *(ArgItr++);

	// Declare a vector for the macro body
	std::vector<CWeakReference> Body;

	// For each term of the body
	for (; ArgItr.IsValid(); ++ArgItr)
	{
		// Add this term to the vector
		Body.push_back(*ArgItr);
	}

	// Declare a vector for the argument names
	std::vector<std::string> ArgNames;

	// Declare a variable for the variable arguments flag
	bool VarArgs;

	// If the argument list is just one symbol
	switch (Arguments->GetTypeId())
	{
		// The argument list is ust one symbol
		case TYPE_SYMBOL:
		{
			// This function has a variable argument count
			VarArgs = true;

			// Get a reference to the argument name
			CRef<CSymbol> ArgName = Arguments;

			// Set the argument name
			ArgNames.push_back(ArgName->GetValue());
		}
		break;

		// This is a proper argument list
		case TYPE_PAIR:
		case TYPE_NIL:
		{
			// This function has a fixed argument count
			VarArgs = false;

			// For each input argument
			for (CSListIterator ArgItr = Arguments; ArgItr.IsValid(); ++ArgItr)
			{
				// Get a reference to this argument names
				CRef<CSymbol> ArgName = *ArgItr;

				// Add the argument name to the list
				ArgNames.push_back(ArgName->GetValue());
			}
		}
		break;

		// Invalid argument list type
		default:
		{
			// Throw an exception
			throw CRunError("Invalid argument list", Arguments);
		}
	}

	// Return a new macro object
	return CMacro::Create(Env.Extend(), ArgNames, Body, VarArgs);
}

/*******************************************************
* Function: static CLISPCore::macro_lambda()
* Purpose : Define a function (lambda expression)
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::macro_lambda(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Create an argument iterator for the list
	CSListIterator ArgItr = Args;	

	// Extract the argument names
	CReference Arguments = *(ArgItr++);

	// Declare a vector for the function body
	std::vector<CWeakReference> Body;

	// For each term of the body
	for (; ArgItr.IsValid(); ++ArgItr)
	{
		// Add this term to the vector
		Body.push_back(*ArgItr);
	}

	// Declare a vector for the argument names
	std::vector<std::string> ArgNames;

	// Declare a variable for the variable arguments flag
	bool VarArgs;

	// If the argument list is just one symbol
	switch (Arguments->GetTypeId())
	{
		// The argument list is ust one symbol
		case TYPE_SYMBOL:
		{
			// This function has a variable argument count
			VarArgs = true;

			// Get a reference to the argument name
			CRef<CSymbol> ArgName = Arguments;

			// Set the argument name
			ArgNames.push_back(ArgName->GetValue());
		}
		break;

		// This is a proper argument list
		case TYPE_PAIR:
		case TYPE_NIL:
		{
			// This function has a fixed argument count
			VarArgs = false;

			// For each input argument
			for (CSListIterator ArgItr = Arguments; ArgItr.IsValid(); ++ArgItr)
			{
				// Get a reference to this argument names
				CRef<CSymbol> ArgName = *ArgItr;

				// Add the argument name to the list
				ArgNames.push_back(ArgName->GetValue());
			}
		}
		break;

		// Invalid argument list type
		default:
		{
			// Throw an exception
			throw CRunError("Invalid argument list", Arguments);
		}
	}

	// Return a new function object
	return CFunction::Create(Env.Extend(), ArgNames, Body, VarArgs);
}

/*******************************************************
* Function: static CLISPCore::macro_if()
* Purpose : If test control flow primitive
* Initial : Max Payne on July 10, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::macro_if(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Create an argument iterator for the list
	CSListIterator ArgItr = Args;

	// Evaluate the test value
	CRef<CBool> TestValue = Interpreter.EvalPrimitive(*(ArgItr++), Env);

	// If the value is true
	if (TestValue->GetValue() == true)
	{
		// Evaluate the true-expression
		return Interpreter.EvalPrimitive(*ArgItr, Env);
	}

	// Otherwise, the value is false
	else
	{
		// If there is a false-expression
		if ((++ArgItr).IsValid())
		{
			// Evaluate the false-expression
			return Interpreter.EvalPrimitive(*ArgItr, Env);
		}
		else
		{
			// There is nothing to evaluate
			return CBool::Create(false);
		}
	}
}

/*******************************************************
* Function: static CLISPCore::macro_cond()
* Purpose : Cond control flow primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::macro_cond(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Declare an iterator for the arguments
	CSListIterator ArgItr;

	// For each input argument except the last
	for (ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// Get a reference to this clause
		CSListIterator Clause = *ArgItr;

		// Evaluate the condition
		CRef<CBool> Condition = Interpreter.EvalPrimitive(*Clause++, Env);

		// If the condition is true
		if (Condition->GetValue())
		{
			// Evaluate this expression and return the value
			return Interpreter.EvalPrimitive(*Clause++, Env);
		}
	}

	// No clauses, throw an exception
	throw CRunError("No clauses in cond expression");
}

/*******************************************************
* Function: static CLISPCore::macro_while()
* Purpose : While loop control flow primitive
* Initial : Max Payne on September 18, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::macro_while(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter)
{
	// Create an argument iterator for the argument list
	CSListIterator ArgItr = Args;

	// Extract the condition
	CReference Condition = *(ArgItr++);

	// Save an iterator to the primitives
	CSListIterator Primitives = ArgItr;

	// Declare a variable for the evaluation result
	CReference Result;

	// Loop until the evaluation fails
	for (;;)
	{
		// Evaluate the condition
		CRef<CBool> Boolean = Interpreter.EvalPrimitive(Condition, Env);

		// If the condition evaluates to false
		if (Boolean->GetValue() == false)
		{
			// Break out of the loop
			break;
		}

		// For each primitive to be evaluated
		for (CSListIterator PrimItr = Primitives; PrimItr.IsValid(); ++PrimItr)
		{
			// Evaluate the primitive and store the result
			Result = Interpreter.EvalPrimitive(*PrimItr, Env);
		}
	}

	// Return the last evaluation result
	return Result;
}

/*******************************************************
* Function: static CLISPCore::func_equals()
* Purpose : Equality testing primitive
* Initial : Max Payne on August 30, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_equals(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the first item
	CReference FirstItem = Args->GetCar();

	// Switch on the type of the first item
	switch (FirstItem->GetTypeId())
	{
		// Types we can handle
		case TYPE_BOOL:
		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_STRING:
		break;

		// Unhandled argument type
		default:
		{
			// Throw an exception
			throw CRunError("Cannot compare unhandled argument type", FirstItem);
		}
	}

	// Declare a variable for the result
	bool Result = true;

	// For each input argument
	for (CSListIterator ArgItr = Args->GetCdr(); ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Boolean argument
			case TYPE_BOOL:
			{
				// Get a reference to this argument
				CRef<CBool> ThisArg = *ArgItr;

				// Switch on the type of the first argument
				switch (FirstItem->GetTypeId())
				{
					// Boolean type
					case TYPE_BOOL:
					{
						// Get a reference to the first argument
						CRef<CBool> FirstArg = FirstItem;

						// Perform the comparison
						Result = (ThisArg->GetValue() == FirstArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to this argument
				CRef<CInt> ThisArg = *ArgItr;

				// Switch on the type of the first argument
				switch (FirstItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the first argument
						CRef<CInt> FirstArg = FirstItem;

						// Perform the comparison
						Result = (ThisArg->GetValue() == FirstArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the first argument
						CRef<CFloat> FirstArg = FirstItem;

						// Perform the comparison
						Result = (ThisArg->GetValue() == FirstArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to this argument
				CRef<CFloat> ThisArg = *ArgItr;

				// Switch on the type of the first argument
				switch (FirstItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the first argument
						CRef<CInt> FirstArg = FirstItem;

						// Perform the comparison
						Result = (ThisArg->GetValue() == FirstArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the first argument
						CRef<CFloat> FirstArg = FirstItem;

						// Perform the comparison
						Result = (ThisArg->GetValue() == FirstArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// String argument
			case TYPE_STRING:
			{
				// Get a reference to this argument
				CRef<CString> ThisArg = *ArgItr;

				// Switch on the type of the first argument
				switch (FirstItem->GetTypeId())
				{
					// Boolean type
					case TYPE_STRING:
					{
						// Get a reference to the first argument
						CRef<CString> FirstArg = FirstItem;

						// Perform the comparison
						Result = (ThisArg->GetValue() == FirstArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot compare unhandled argument type", *ArgItr);
			}
		}

		// If the result is false
		if (Result == false)
		{
			// The comparison is done, the items are not equal
			break;
		}
	}

	// Return the result
	return CBool::Create(Result);
}

/*******************************************************
* Function: static CLISPCore::func_lt()
* Purpose : Less than testing primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_lt(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the first item
	CReference PrevItem = Args->GetCar();

	// Switch on the type of the first item
	switch (PrevItem->GetTypeId())
	{
		// Types we can handle
		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_STRING:
		break;

		// Unhandled argument type
		default:
		{
			// Throw an exception
			throw CRunError("Cannot compare unhandled argument type", PrevItem);
		}
	}

	// Declare a variable for the result
	bool Result = true;

	// For each input argument
	for (CSListIterator ArgItr = Args->GetCdr(); ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to this argument
				CRef<CInt> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the previous argument
						CRef<CInt> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() < ThisArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the previous argument
						CRef<CFloat> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() < ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to this argument
				CRef<CFloat> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the previous argument
						CRef<CInt> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() < ThisArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the previous argument
						CRef<CFloat> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() < ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// String argument
			case TYPE_STRING:
			{
				// Get a reference to this argument
				CRef<CString> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Boolean type
					case TYPE_STRING:
					{
						// Get a reference to the previous argument
						CRef<CString> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() < ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot compare unhandled argument type", *ArgItr);
			}
		}

		// If the result is false
		if (Result == false)
		{
			// The comparison is done, the items are not equal
			break;
		}

		// Update the previous argument for next iteration
		PrevItem = *ArgItr;
	}

	// Return the result
	return CBool::Create(Result);
}

/*******************************************************
* Function: static CLISPCore::func_lte()
* Purpose : Less than or equal testing primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_lte(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the first item
	CReference PrevItem = Args->GetCar();

	// Switch on the type of the first item
	switch (PrevItem->GetTypeId())
	{
		// Types we can handle
		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_STRING:
		break;

		// Unhandled argument type
		default:
		{
			// Throw an exception
			throw CRunError("Cannot compare unhandled argument type", PrevItem);
		}
	}

	// Declare a variable for the result
	bool Result = true;

	// For each input argument
	for (CSListIterator ArgItr = Args->GetCdr(); ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to this argument
				CRef<CInt> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the previous argument
						CRef<CInt> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() <= ThisArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the previous argument
						CRef<CFloat> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() <= ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to this argument
				CRef<CFloat> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the previous argument
						CRef<CInt> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() <= ThisArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the previous argument
						CRef<CFloat> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() <= ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// String argument
			case TYPE_STRING:
			{
				// Get a reference to this argument
				CRef<CString> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Boolean type
					case TYPE_STRING:
					{
						// Get a reference to the previous argument
						CRef<CString> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() <= ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot compare unhandled argument type", *ArgItr);
			}
		}

		// If the result is false
		if (Result == false)
		{
			// The comparison is done, the items are not equal
			break;
		}

		// Update the previous argument for next iteration
		PrevItem = *ArgItr;
	}

	// Return the result
	return CBool::Create(Result);
}

/*******************************************************
* Function: static CLISPCore::func_gt()
* Purpose : Greater than testing primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_gt(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the first item
	CReference PrevItem = Args->GetCar();

	// Switch on the type of the first item
	switch (PrevItem->GetTypeId())
	{
		// Types we can handle
		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_STRING:
		break;

		// Unhandled argument type
		default:
		{
			// Throw an exception
			throw CRunError("Cannot compare unhandled argument type", PrevItem);
		}
	}

	// Declare a variable for the result
	bool Result = true;

	// For each input argument
	for (CSListIterator ArgItr = Args->GetCdr(); ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to this argument
				CRef<CInt> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the previous argument
						CRef<CInt> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() > ThisArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the previous argument
						CRef<CFloat> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() > ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to this argument
				CRef<CFloat> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the previous argument
						CRef<CInt> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() > ThisArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the previous argument
						CRef<CFloat> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() > ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// String argument
			case TYPE_STRING:
			{
				// Get a reference to this argument
				CRef<CString> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Boolean type
					case TYPE_STRING:
					{
						// Get a reference to the previous argument
						CRef<CString> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() > ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot compare unhandled argument type", *ArgItr);
			}
		}

		// If the result is false
		if (Result == false)
		{
			// The comparison is done, the items are not equal
			break;
		}

		// Update the previous argument for next iteration
		PrevItem = *ArgItr;
	}

	// Return the result
	return CBool::Create(Result);
}

/*******************************************************
* Function: static CLISPCore::func_gte()
* Purpose : Greater than or equal testing primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_gte(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the first item
	CReference PrevItem = Args->GetCar();

	// Switch on the type of the first item
	switch (PrevItem->GetTypeId())
	{
		// Types we can handle
		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_STRING:
		break;

		// Unhandled argument type
		default:
		{
			// Throw an exception
			throw CRunError("Cannot compare unhandled argument type", PrevItem);
		}
	}

	// Declare a variable for the result
	bool Result = true;

	// For each input argument
	for (CSListIterator ArgItr = Args->GetCdr(); ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to this argument
				CRef<CInt> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the previous argument
						CRef<CInt> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() >= ThisArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the previous argument
						CRef<CFloat> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() >= ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to this argument
				CRef<CFloat> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Integer type
					case TYPE_INT:
					{
						// Get a reference to the previous argument
						CRef<CInt> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() >= ThisArg->GetValue());
					}
					break;

					// Floating-point type
					case TYPE_FLOAT:
					{
						// Get a reference to the previous argument
						CRef<CFloat> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() >= ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// String argument
			case TYPE_STRING:
			{
				// Get a reference to this argument
				CRef<CString> ThisArg = *ArgItr;

				// Switch on the type of the previous argument
				switch (PrevItem->GetTypeId())
				{
					// Boolean type
					case TYPE_STRING:
					{
						// Get a reference to the previous argument
						CRef<CString> PrevArg = PrevItem;

						// Perform the comparison
						Result = (PrevArg->GetValue() >= ThisArg->GetValue());
					}
					break;

					// Incomparable type
					default:
					{
						// The values are not equal
						Result = false;
					}
				}
			}
			break;

			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot compare unhandled argument type", *ArgItr);
			}
		}

		// If the result is false
		if (Result == false)
		{
			// The comparison is done, the items are not equal
			break;
		}

		// Update the previous argument for next iteration
		PrevItem = *ArgItr;
	}

	// Return the result
	return CBool::Create(Result);
}

/*******************************************************
* Function: static CLISPCore::func_and()
* Purpose : Logical and operator
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_and(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// For each input argument
	for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// Get a reference to this boolean value
		CRef<CBool> Boolean = *ArgItr;

		// If one value is false
		if (Boolean->GetValue() == false)
		{
			// The result is false
			return CBool::Create(false);
		}
	}

	// All values are true, result is true
	return CBool::Create(true);
}

/*******************************************************
* Function: static CLISPCore::func_or()
* Purpose : Logical or operator
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_or(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// For each input argument
	for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// Get a reference to this boolean value
		CRef<CBool> Boolean = *ArgItr;

		// If one value is true
		if (Boolean->GetValue() == true)
		{
			// The result is true
			return CBool::Create(true);
		}
	}

	// All values are false, result is false
	return CBool::Create(false);
}

/*******************************************************
* Function: static CLISPCore::func_not()
* Purpose : Logical negation operator
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_not(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the boolean value
	CRef<CBool> Boolean = Args->GetCar();

	// Return a boolean containing the negated value
	return CBool::Create(!Boolean->GetValue());
}

/*******************************************************
* Function: static CLISPCore::func_cons()
* Purpose : Cons pair creation primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_cons(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Create an iterator for the arguments
	CSListIterator ArgItr = Args;

	// Create a new pair containing the arguments
	return CPair::Create(*ArgItr, *(++ArgItr));
}

/*******************************************************
* Function: static CLISPCore::func_car()
* Purpose : Car pair access primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_car(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the pair argument
	CRef<CPair> Pair = Args->GetCar();

	// Return the car of the pair
	return Pair->GetCar();
}

/*******************************************************
* Function: static CLISPCore::func_cdr()
* Purpose : Cdr pair access primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_cdr(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the pair argument
	CRef<CPair> Pair = Args->GetCar();

	// Return the cdr of the pair
	return Pair->GetCdr();
}

/*******************************************************
* Function: static CLISPCore::func_list()
* Purpose : List creation primitive
* Initial : Max Payne on September 17, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_list(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Return the list containing the arguments
	return Args;
}

/*******************************************************
* Function: static CLISPCore::func_add()
* Purpose : Addition arithmetic primitive
* Initial : Max Payne on July 10, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_add(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Integer and floating-point results
	int32 IntResult = 0;
	float32 FloatResult = 0.0f;

	// Indicates whether float arguments were encountered
	bool FloatValues = false;

	// For each input argument
	for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to the int argument
				CRef<CInt> Int = *ArgItr;

				// Update the integer result
				IntResult += Int->GetValue();
			
				// Update the floating-point result
				FloatResult += (float)Int->GetValue();
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to the float argument
				CRef<CFloat> Float = *ArgItr;

				// Set the float value flag
				FloatValues = true;
			
				// Update the floating-point result
				FloatResult += Float->GetValue();
			}
			break;

			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot add unhandled argument type", *ArgItr);
			}
		}
	}

	// If floating-point values were encountered
	if (FloatValues)
	{
		// Return a floating-point value for the result
		return CFloat::Create(FloatResult);
	}
	else
	{
		// Return an integer value for the result
		return CInt::Create(IntResult);
	}
}

/*******************************************************
* Function: static CLISPCore::func_sub()
* Purpose : Subtraction arithmetic primitive
* Initial : Max Payne on August 30, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_sub(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Integer and floating-point results
	int32 IntResult = 0;
	float32 FloatResult = 0.0f;

	// Indicates whether float arguments were encountered
	bool FloatValues = false;

	// For each input argument
	for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// If there is more than one argument
		if (ArgItr == (CSListIterator)Args->GetCdr())
		{
			// Inverse the current result
			IntResult *= -1;
			FloatResult *= -1.0f;
		}

		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to the int argument
				CRef<CInt> Int = *ArgItr;

				// Update the integer result
				IntResult -= Int->GetValue();
			
				// Update the floating-point result
				FloatResult -= (float)Int->GetValue();
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to the float argument
				CRef<CFloat> Float = *ArgItr;

				// Set the float value flag
				FloatValues = true;
			
				// Update the floating-point result
				FloatResult -= Float->GetValue();
			}
			break;

			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot subtract unhandled argument type", *ArgItr);
			}
		}
	}

	// If floating-point values were encountered
	if (FloatValues)
	{
		// Return a floating-point value for the result
		return CFloat::Create(FloatResult);
	}
	else
	{
		// Return an integer value for the result
		return CInt::Create(IntResult);
	}
}

/*******************************************************
* Function: static CLISPCore::func_mul()
* Purpose : Multiplication arithmetic primitive
* Initial : Max Payne on July 10, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_mul(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Integer and floating-point results
	int32 IntResult = 1;
	float32 FloatResult = 1.0f;

	// Indicates whether float arguments were encountered
	bool FloatValues = false;

	// For each input argument
	for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to the int argument
				CRef<CInt> Int = *ArgItr;

				// Update the integer result
				IntResult *= Int->GetValue();
			
				// Update the floating-point result
				FloatResult *= (float)Int->GetValue();
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to the float argument
				CRef<CFloat> Float = *ArgItr;

				// Set the float value flag
				FloatValues = true;
			
				// Update the floating-point result
				FloatResult *= Float->GetValue();
			}
			break;
            
			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot add unhandled argument type", *ArgItr);
			}
		}

	}

	// If floating-point values were encountered
	if (FloatValues)
	{
		// Return a floating-point value for the result
		return CFloat::Create(FloatResult);
	}
	else
	{
		// Return an integer value for the result
		return CInt::Create(IntResult);
	}
}

/*******************************************************
* Function: static CLISPCore::func_div()
* Purpose : Division arithmetic primitive
* Initial : Max Payne on August 29, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_div(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Integer and floating-point results
	int32 IntResult = 0;
	float32 FloatResult = 0.0f;

	// Indicates whether float arguments were encountered
	bool FloatValues = false;

	// Create an iterator for the arguments
	CSListIterator ArgItr = Args;

	// Switch on the type of the first argument
	switch (ArgItr->GetTypeId())
	{
		// Integer argument
		case TYPE_INT:
		{
			// Get a reference to the int argument
			CRef<CInt> Int = *ArgItr;

			// Update the integer result
			IntResult = Int->GetValue();
		
			// Update the floating-point result
			FloatResult = (float)Int->GetValue();
		}
		break;

		// Floating-point argument
		case TYPE_FLOAT:
		{
			// Get a reference to the float argument
			CRef<CFloat> Float = *ArgItr;

			// Set the float value flag
			FloatValues = true;
		
			// Update the floating-point result
			FloatResult = Float->GetValue();
		}
		break;
        
		// Unhandled argument type
		default:
		{
			// Throw an exception
			throw CRunError("Cannot divide unhandled argument type", *ArgItr);
		}
	}

	// For each input argument
	for (++ArgItr; ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the type of the argument
		switch (ArgItr->GetTypeId())
		{
			// Integer argument
			case TYPE_INT:
			{
				// Get a reference to the int argument
				CRef<CInt> Int = *ArgItr;

				// If the integer is 0
				if (Int->GetValue() == 0)
				{
					// Throw an exception
					throw CRunError("Division by 0 error");
				}

				// Update the integer result
				IntResult /= Int->GetValue();
			
				// Update the floating-point result
				FloatResult /= (float)Int->GetValue();
			}
			break;

			// Floating-point argument
			case TYPE_FLOAT:
			{
				// Get a reference to the float argument
				CRef<CFloat> Float = *ArgItr;

				// Set the float value flag
				FloatValues = true;

				// If the float is 0
				if (Float->GetValue() == 0.0f)
				{
					// Throw an exception
					throw CRunError("Division by 0 error");
				}

				// Update the floating-point result
				FloatResult /= Float->GetValue();
			}
			break;
            
			// Unhandled argument type
			default:
			{
				// Throw an exception
				throw CRunError("Cannot divide with unhandled argument type", *ArgItr);
			}
		}
	}

	// If floating-point values were encountered
	if (FloatValues)
	{
		// Return a floating-point value for the result
		return CFloat::Create(FloatResult);
	}
	else
	{
		// Return an integer value for the result
		return CInt::Create(IntResult);
	}
}

/*******************************************************
* Function: static CLISPCore::func_mod()
* Purpose : Modulo arithmetic primitive
* Initial : Max Payne on September 20, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_mod(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Create an iterator for the arguments
	CSListIterator ArgItr = Args;

	// Extract the left and right values
	CRef<CInt> LValue = *(ArgItr++);
	CRef<CInt> RValue = *ArgItr;

	// If the r-value is 0
	if (RValue->GetValue() == 0)
	{
		// Throw an exception
		throw CRunError("Modulo by 0 error");
	}

	// Perform the modulo operation
	int32 IntResult = LValue->GetValue() % RValue->GetValue();

	// Return an integer value for the result
	return CInt::Create(IntResult);
}

/*******************************************************
* Function: static CLISPCore::func_string()
* Purpose : String constructor function
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_string(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Declare a string for the output
	std::string Output;

	// For each argument
	for (XLSE::CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// Switch on the argument type
		switch (ArgItr->GetTypeId())
		{
			// If the argument is a string
			case XLSE::TYPE_STRING:
			{
				// Get a typed reference to the string
				XLSE::CRef<XLSE::CString> String = *ArgItr;

				// Add the string to the output
				Output += String->GetValue();
			}
			break;

			// Default behavior
			default:
			{
				// Add the default text representation
				Output += ArgItr->ToString();
			}
		}
	}

	// Return a string object containing the output
	return XLSE::CString::Create(Output);
}

/*******************************************************
* Function: static CLISPCore::func_strsub()
* Purpose : Substring function
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_strsub(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Create an iterator for the arguments
	CSListIterator ArgItr = Args;

	// Get a reference to the string
	CRef<CString> String = *(ArgItr++);

	// Get the starting index
	CRef<CInt> Index = *(ArgItr++);

	// Get the substring length
	CRef<CInt> Length = *ArgItr;

	// If the starting index is greater than the string length
	if (Index->GetValue() > (int32)String->GetValue().length())
	{
		// Throw an exception
		throw CRunError("Index out of range");
	}

	// Extract the substring
	std::string SubString = String->GetValue().substr(Index->GetValue(), Length->GetValue());

	// Return the substring in a string object
	return CString::Create(SubString);
}

/*******************************************************
* Function: static CLISPCore::func_strlen()
* Purpose : String length function
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_strlen(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the string
	CRef<CString> String = Args->GetCar();

	// Obtain the string length
	int32 StringLength = (int32)String->GetValue().length();

	// Return the string length as an integer value
	return CInt::Create(StringLength);
}

/*******************************************************
* Function: static CLISPCore::func_vector()
* Purpose : Vector primitive constructor
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_vector(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Create a vector primitive
	CRef<CVector> Vector = CVector::Create();

	// For each argument
	for (CSListIterator ArgItr = Args; ArgItr.IsValid(); ++ArgItr)
	{
		// Add the argument to the vector
		Vector->AddItem(*ArgItr);
	}

	// Return the vector object
	return Vector;
}

/*******************************************************
* Function: static CLISPCore::func_vector_ref()
* Purpose : Vector access operator
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_vector_ref(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Create an iterator for the arguments
	CSListIterator ArgItr = Args;

	// Get a reference to the vector primitive
	CRef<CVector> Vector = *(ArgItr++);

	// Get a reference to the index
	CRef<CInt> Index = *ArgItr;

	// If the index is out of range
	if (Index->GetValue() >= (int32)Vector->GetVector().size())
	{
		// Throw an exception
		throw CRunError("Index out of range");
	}

	// Return the object stored at the vector index
	return Vector->GetVector()[Index->GetValue()];
}

/*******************************************************
* Function: static CLISPCore::func_vector_length()
* Purpose : Vector length operator
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_vector_length(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Get a reference to the vector primitive
	CRef<CVector> Vector = Args->GetCar();

	// Obtain the vector length
	int32 VectorLength = (int32)Vector->GetVector().size();

	// Return the vector length as an int primitive
	return CInt::Create(VectorLength);
}

/*******************************************************
* Function: static CLISPCore::func_nilp()
* Purpose : NIL type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_nilp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Test if the argument is nil
	return CBool::Create(Args->GetCar()->GetTypeId() == TYPE_NIL);
}

/*******************************************************
* Function: static CLISPCore::func_pairp()
* Purpose : Pair type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_pairp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Test if the argument is a pair
	return CBool::Create(Args->GetCar()->GetTypeId() == TYPE_PAIR);
}

/*******************************************************
* Function: static CLISPCore::func_listp()
* Purpose : List type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_listp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Switch on the argument type
	switch (Args->GetCar()->GetTypeId())
	{
		// If its a nil or pair, it is a list
		case TYPE_NIL:
		case TYPE_PAIR:
		return CBool::Create(true);

		// Otherwise, not a list
		default:
		return CBool::Create(false);
	}
}

/*******************************************************
* Function: static CLISPCore::func_symbolp()
* Purpose : Symbol type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_symbolp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Test if the argument is a symbol
	return CBool::Create(Args->GetCar()->GetTypeId() == TYPE_SYMBOL);
}

/*******************************************************
* Function: static CLISPCore::func_boolp()
* Purpose : Boolean type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_boolp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Test if the argument is a boolean
	return CBool::Create(Args->GetCar()->GetTypeId() == TYPE_BOOL);
}

/*******************************************************
* Function: static CLISPCore::func_intp()
* Purpose : Integer type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_intp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Test if the argument is an integer
	return CBool::Create(Args->GetCar()->GetTypeId() == TYPE_INT);
}

/*******************************************************
* Function: static CLISPCore::func_floatp()
* Purpose : Float type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_floatp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Test if the argument is a float
	return CBool::Create(Args->GetCar()->GetTypeId() == TYPE_FLOAT);
}

/*******************************************************
* Function: static CLISPCore::func_stringp()
* Purpose : String type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_stringp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Test if the argument is a string
	return CBool::Create(Args->GetCar()->GetTypeId() == TYPE_STRING);
}

/*******************************************************
* Function: static CLISPCore::func_vectorp()
* Purpose : Vector type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_vectorp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Test if the argument is a vector
	return CBool::Create(Args->GetCar()->GetTypeId() == TYPE_VECTOR);
}

/*******************************************************
* Function: static CLISPCore::func_functionp()
* Purpose : Function type testing predicate
* Initial : Max Payne on September 22, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CLISPCore::func_functionp(const CRef<CPair>& Args, CInterpreter& Interpreter)
{
	// Switch on the argument type
	switch (Args->GetCar()->GetTypeId())
	{
		// If it is any function or macro type
		case TYPE_MACRO:
		case TYPE_FUNCTION:
		case TYPE_EXTMACRO:
		case TYPE_EXTFUNCTION:
		return CBool::Create(true);

		// Otherwise, not a function
		default:
		return CBool::Create(false);
	}
}