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
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

// Header files
#include <map>
#include <string>
#include "allocator.h"
#include "types.h"

// Include this in the XLSE namespace
namespace XLSE
{
	/*******************************************************
	* Class   : CEnvironment
	* Purpose : Implement a LISP evaluation environment
	* Initial : Max Payne on February 15, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CEnvironment
	{
	public:

		// Constructor and destructor
		CEnvironment();
		CEnvironment(const CEnvironment& Env);
		virtual ~CEnvironment();

		// Method to clear all bindings
		void Clear();

		// Method to create a binding
		bool Bind(const std::string& Name, const CReference& Item);

		// Method to modify a binding
		bool Set(const std::string& Name, const CReference& Item);

		// Method to find a bound item
		CReference Find(const std::string& Name) const;

		// Method to extend this environment
		CEnvironment Extend();

		// Assignment operator
		CEnvironment& operator = (const CEnvironment& Env);

	private:

		// Item map type
		typedef std::map<std::string, CReference> TItemMap;

		// Environment data structure
		struct SEnvData
		{
			// Internal environment bindings
			TItemMap Bindings;

			// Pointer to parent environment data
			SEnvData* pParent;

			// Reference count
			uint32 RefCount;
		};

		// Environment data pointer
		SEnvData* m_pEnvData;

		// Environment data allocator
		static CAllocator<SEnvData> s_Allocator;
	};
};

#endif // #ifndef _ENVIRONMENT_H_