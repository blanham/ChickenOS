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
#include <cassert>
#include "script.h"

// Include the XLSE namespace
using namespace XLSE;

// Static parser instance
CParser CScript::s_Parser;

// Static base environment
CEnvironment CScript::s_BaseEnv;

// Static script map
CScript::TScriptMap CScript::s_Scripts;

/*******************************************************
* Function: CScript::CScript()
* Purpose : Constructor for script class
* Initial : Max Payne on September 28, 2006
********************************************************
Revisions and bug fixes:
*/
CScript::CScript()
: m_pScriptData(NULL)
{
}

/*******************************************************
* Function: CScript::~CScript()
* Purpose : Destructor for script class
* Initial : Max Payne on September 28, 2006
********************************************************
Revisions and bug fixes:
*/
CScript::~CScript()
{
	// Free the script
	FreeScript();
}

/*******************************************************
* Function: CScript::LoadScript()
* Purpose : Method to load a script
* Initial : Max Payne on September 28, 2006
********************************************************
Revisions and bug fixes:
*/
bool CScript::LoadScript(const std::string& FileName)
{
	// Ensure a script is not already loaded
	if (m_pScriptData)
		return false;

	// Attempt to find the script among the script map
	TScriptMap::iterator ScriptItr = s_Scripts.find(FileName);

	// If the script is already loaded
	if (ScriptItr == s_Scripts.end())
	{
		// Set the script data pointer
		m_pScriptData = ScriptItr->second;

		// Increment the reference count for the script data
		++m_pScriptData->RefCount;
	}

	// Otherwise, the script needs to be loaded
	else
	{
		// Attempt to parse the script file
		CReference NewProgram = s_Parser.ParseFile(FileName);

		// Create a new script data object
		m_pScriptData = new SScriptData;

		// Store the script file name
		m_pScriptData->FileName = FileName;

		// Initialize the reference count to 1
		m_pScriptData->RefCount = 1;

		// Set the base environment of the interpreter
		m_pScriptData->Interpreter.SetBaseEnv(s_BaseEnv);

		// Load the new program in the interpreter
		m_pScriptData->Interpreter.LoadProgram(NewProgram);

		// Run the new program in the interpreter
		m_pScriptData->Interpreter.RunProgram();

		// Add the script to the script map
		s_Scripts[FileName] = m_pScriptData;
	}

	// Nothing went wrong
	return true;
}

/*******************************************************
* Function: CScript::FreeScript()
* Purpose : Method to free a script
* Initial : Max Payne on September 30, 2006
********************************************************
Revisions and bug fixes:
*/
bool CScript::FreeScript()
{
	// Ensure a script is currently loaded
	if (!m_pScriptData)
		return false;

	// Decrement the reference count of the script data
	--m_pScriptData->RefCount;

	// If there are no references to the script data
	if (m_pScriptData->RefCount == 0)
	{
		// Obtain an iterator to the script data
		TScriptMap::iterator ScriptItr = s_Scripts.find(m_pScriptData->FileName);

		// Erase this script from the map
		s_Scripts.erase(ScriptItr);

		// Delete the script data
		delete m_pScriptData;

		// Nullify the script data pointer
		m_pScriptData = NULL;
	}

	// Nothing went wrong
	return true;
}