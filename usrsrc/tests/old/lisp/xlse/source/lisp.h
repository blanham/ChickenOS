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
#ifndef _LISP_H_
#define _LISP_H_

// Header files
#include "factory.h"
#include "primitives.h"
#include "interpreter.h"
#include "environment.h"

// Include this in the XLSE namespace
namespace XLSE
{	
	/*******************************************************
	* Class   : CLISPCore
	* Purpose : Provide elementary lisp constructs
	* Initial : Max Payne on July 5, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CLISPCore : public CEnvironment
	{
	public:

		// Constructor
		CLISPCore();

	private:

		// Debug/error management primitives
		static CReference func_assert(const CRef<CPair>& Args, CInterpreter& Interpreter);

		// Environment manipulation primitives
		static CReference macro_define(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);
		static CReference macro_set(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);

		// Function/macro definition operators
		static CReference macro_macro(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);
		static CReference macro_lambda(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);

		// Control flow operators
		static CReference macro_if(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);
		static CReference macro_cond(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);
		static CReference macro_while(const CRef<CPair>& Args, CEnvironment& Env, CInterpreter& Interpreter);

		// Logical operators
		static CReference func_equals(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_lt(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_lte(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_gt(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_gte(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_and(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_or(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_not(const CRef<CPair>& Args, CInterpreter& Interpreter);

		// Pair/list manipulation operators
		static CReference func_cons(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_car(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_cdr(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_list(const CRef<CPair>& Args, CInterpreter& Interpreter);

		// Arithmetic operators
		static CReference func_add(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_sub(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_mul(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_div(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_mod(const CRef<CPair>& Args, CInterpreter& Interpreter);

		// String functions and operators
		static CReference func_string(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_strsub(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_strlen(const CRef<CPair>& Args, CInterpreter& Interpreter);

		// Vector operators
		static CReference func_vector(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_vector_ref(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_vector_length(const CRef<CPair>& Args, CInterpreter& Interpreter);

		// Type testing predicates
		static CReference func_nilp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_pairp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_listp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_symbolp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_boolp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_intp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_floatp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_stringp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_vectorp(const CRef<CPair>& Args, CInterpreter& Interpreter);
		static CReference func_functionp(const CRef<CPair>& Args, CInterpreter& Interpreter);
	};
};

#endif // #ifndef _LISP_H_