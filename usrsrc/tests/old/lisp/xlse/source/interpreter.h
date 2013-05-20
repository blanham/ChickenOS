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
#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

// Header files
#include <string>
#include "factory.h"
#include "references.h"
#include "environment.h"
#include "factory.h"

// Include this in the XLSE namespace
namespace XLSE
{
	// Forward declarations
	//class CPair;

	/*******************************************************
	* Class   : CInterpreter
	* Purpose : Interpreter for XLSE scripts
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CInterpreter
	{
	public:

		// Constructor and destructor
		CInterpreter();
		~CInterpreter();

		// Method to set the base environment
		void SetBaseEnv(const CEnvironment& Environment);

		// Method to set a global binding
		void SetGlobal(const std::string& Name, const CReference& Item);

		// Method to load a program
		void LoadProgram(const CReference& Script);

		// Method to run a loaded program
		void RunProgram();

		// Method to evaluate a primitive
		CReference EvalPrimitive(const CReference& Primitive, CEnvironment& Env);

		// Method to evaluate an s-expression
		CReference EvalExpression(const CRef<CPair>& Expression, CEnvironment& Env);

		// Accessor to get the execution result
		const CReference& GetResult() const { return m_Result; }

	private:

		// Local copy of a loaded program
		CRef<CPair> m_Program;

		// Base environment
		CEnvironment m_BaseEnv;

		// Global environment
		CEnvironment m_GlobalEnv;

		// Execution result
		CReference m_Result;
	};
};

#endif // #ifndef _INTERPRETER_H_