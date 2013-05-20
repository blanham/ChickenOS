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
#ifndef _SCRIPT_H_
#define _SCRIPT_H_

// Header files
#include <map>
#include "parser.h"
#include "interpreter.h"

// Include this in the XLSE namespace
namespace XLSE
{
	/*******************************************************
	* Class   : CScript
	* Purpose : Act as an interface to script files
	* Initial : Max Payne on September 17, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CScript
	{
	public:

		// Constructor and destructor
		CScript();
		~CScript();

		// Method to load a module
		bool LoadScript(const std::string& FileName);

		// Method to free a module
		bool FreeScript();

		// Static method to set the base environment
		static void SetBaseEnv(const CEnvironment& Env) { s_BaseEnv = Env; }

	private:

		// Script data structure
		struct SScriptData
		{
			// Script file name
			std::string FileName;

			// Interpreter for this module
			CInterpreter Interpreter;

			// Reference count
			size_t RefCount;
		};

		// Data relating to this module
		SScriptData* m_pScriptData;

		// Map of module names to module data
		typedef std::map<std::string, SScriptData*> TScriptMap;

		// Static parser instance
		static CParser s_Parser;

		// Static base environment
		static CEnvironment s_BaseEnv;

		// Static module map
		static TScriptMap s_Scripts;
	};
};

#endif // #ifndef _SCRIPT_H_