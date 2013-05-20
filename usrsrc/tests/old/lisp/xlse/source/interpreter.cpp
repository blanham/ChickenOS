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
#include "interpreter.h"
#include "primitives.h"

// Include the XLSE namespace
using namespace XLSE;

/*******************************************************
* Function: CInterpreter::CInterpreter()
* Purpose : Constructor for XLSE interpreter
* Initial : Max Payne on February 12, 2006
********************************************************
Revisions and bug fixes:
*/
CInterpreter::CInterpreter()
{
}

/*******************************************************
* Function: CInterpreter::~CInterpreter()
* Purpose : Constructor for XLSE interpreter
* Initial : Max Payne on February 12, 2006
********************************************************
Revisions and bug fixes:
*/
CInterpreter::~CInterpreter()
{
}

/*******************************************************
* Function: CInterpreter::SetBaseEnv()
* Purpose : Set the base environment
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
void CInterpreter::SetBaseEnv(const CEnvironment& Environment)
{
	// Keep a local copy of the base environment
	m_BaseEnv = Environment;

	// Create a global environment for the program
	m_GlobalEnv = m_BaseEnv.Extend();
}

/*******************************************************
* Function: CInterpreter::SetGlobal()
* Purpose : Set a global binding
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
void CInterpreter::SetGlobal(const std::string& Name, const CReference& Item)
{
	// Set the binding in the global environment
	m_GlobalEnv.Bind(Name, Item);
}

/*******************************************************
* Function: CInterpreter::LoadProgram()
* Purpose : Load a program into the interpreter
* Initial : Max Payne on February 12, 2006
********************************************************
Revisions and bug fixes:
*/
void CInterpreter::LoadProgram(const CReference& Program)
{
	// Clear the execution result
	m_Result.Nullify();

	// If the specified program is not valid
	if (!Program)
	{
		// Throw an exception
		throw CRunError("Cannot load null program");
	}

	// Store a copy of the program to load
	m_Program = Program->Copy();
}

/*******************************************************
* Function: CInterpreter::RunProgram()
* Purpose : Run a loaded program
* Initial : Max Payne on February 12, 2006
********************************************************
Revisions and bug fixes:
*/
void CInterpreter::RunProgram()
{
	// Clear the execution result
	m_Result.Nullify();

	// If no program is loaded
	if (!m_Program)
	{
		// Throw an exception
		throw CRunError("No program loaded");
	}

	// For each item in the program
	for (CSListIterator ProgramItr = m_Program; ProgramItr.IsValid(); ++ProgramItr)
	{
		// Evaluate the primitive
		m_Result = EvalPrimitive(*ProgramItr, m_GlobalEnv);
	}
}

/*******************************************************
* Function: CInterpreter::EvalPrimitive()
* Purpose : Evaluate a primitive
* Initial : Max Payne on February 15, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CInterpreter::EvalPrimitive(const CReference& Primitive, CEnvironment& Env)
{
	// Switch on the primitive type
	switch (Primitive->GetTypeId())
	{
		// Quoted primitive
		case TYPE_QUOTE:
		{
			// Typecast the reference to the appropriate type
			CRef<CQuote> Quote = Primitive;

			// Return a reference to the item
			return Quote->GetItem();
		}
		break;

		// Pair primitive
		case TYPE_PAIR:
		{
			// Typecast the reference to the appropriate type
			CRef<CPair> Pair = Primitive;

			// Evaluate this as an s-expression
			CReference Result = EvalExpression(Pair, Env);

			// Return the result
			return Result;
		}
		break;

		// Symbol primitive
		case TYPE_SYMBOL:
		{
			// Typecast the reference to the appropriate type
			CRef<CSymbol> Symbol = Primitive;

			// Attempt to find the target item
			CReference Item = Env.Find(Symbol->GetValue());

			// If the binding was not resolved
			if (!Item)
			{
				// Throw a runtime error
				throw CRunError("Unresolved symbol: \"" + Symbol->GetValue() + "\"");
			}

			// Return a reference to the item
			return Item;
		}
		break;

		// Value types (default behavior)
		default:
		{
			// Pass along the reference to the primitive
			return Primitive;
		}
	};
}

/*******************************************************
* Function: CInterpreter::EvalExpression()
* Purpose : Evaluate an s-expression
* Initial : Max Payne on February 12, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CInterpreter::EvalExpression(const CRef<CPair>& Expression, CEnvironment& Env)
{
	// If the expression is null
	if (!Expression)
	{
		// Throw a runtime error
		throw CRunError("Cannot evaluate NIL expression");
	}

	// Set a try block to catch evaluation errors
	try
	{
		// Evaluate the first argument (the operator)
		CReference Operator = EvalPrimitive(Expression->GetCar(), Env);

		// Get a reference to the arguments
		CReference Arguments = Expression->GetCdr();

		// Switch on the type of the operator
		switch (Operator->GetTypeId())
		{
			// Internal macro
			case TYPE_MACRO:
			{
				// Obtain a reference to the macro object
				CRef<CMacro> Macro = Operator;

				// Call the macro object with the arguments
				CReference Result = Macro->Call(Arguments, Env, *this);

				// Return the result
				return Result;
			}
			break;

			// Interal function (lambda form)
			case TYPE_FUNCTION:
			{
				// Obtain a reference to the function object
				CRef<CFunction> Function = Operator;

				// Call the function object with the arguments
				CReference Result = Function->Call(Arguments, Env, *this);

				// Return the result
				return Result;
			}
			break;

			// External macro function
			case TYPE_EXTMACRO:
			{
				// Obtain a reference to the external macro
				CRef<CExtMacro> ExtMacro = Operator;

				// Call the external macro with the unevaluated arguments
				CReference Result = ExtMacro->Call(Arguments, Env, *this);

				// Return the result
				return Result;
			}
			break;

			// External function
			case TYPE_EXTFUNCTION:
			{
				// Obtain a reference to the external function
				CRef<CExtFunction> ExtFunction = Operator;

				// Call the external macro with the evaluated arguments
				CReference Result = ExtFunction->Call(Arguments, Env, *this);
				
				// Return the result
				return Result;
			}
			break;

			// Invalid operator application
			default:
			{
				// Throw a runtime error
				throw CRunError("Invalid operator type in s-expression", Operator);
			}
		}
	}

	// Catch any evaluation error
	catch (CRunError Error)
	{
		// Add information about where evaluation failed
		Error.PushLevel("Expression evaluation failed", Expression);

		// Rethrow the error
		throw Error;
	}
}