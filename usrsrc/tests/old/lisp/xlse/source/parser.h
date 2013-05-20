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
#ifndef _PARSER_H_
#define _PARSER_H_

// Header files
#include <vector>
#include "factory.h"
#include "references.h"

// Include this in the XLSE namespace
namespace XLSE
{
	/*******************************************************
	* Class   : CParseError
	* Purpose : Parsing error exception
	* Initial : Max Payne on August 29, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CParseError
	{
	public:

		// Constructor
		CParseError(const std::string& Error, uint32 Line = 0) : m_ErrorText(Error), m_LineNumber(Line) {}

		// Method to get the error text
		const std::string& GetErrorText() const { return m_ErrorText; }

		// Accessor to get the line number
		uint32 GetLineNumber() const { return m_LineNumber; }

	private:

		// Error description text
		std::string m_ErrorText;

		// Error line number
		uint32 m_LineNumber;
	};

	/*******************************************************
	* Class   : CParser
	* Purpose : Parser for XLSE code
	* Initial : Max Payne on February 10, 2006
	********************************************************
	Revisions and bug fixes:
	*/
	class CParser
	{
	public:

		// Constructor and destructor
		CParser();
		~CParser();

		// Method to parse a script file
		CReference ParseFile(const std::string& FilePath);

		// Method to parse a code stream
		CReference ParseStream(const std::string& InputCode);

	private:

		// Method to streamline input code
		void Streamline();

		// Method to parse an s-list
		CReference ParseSList(bool FirstLevel);

		// Method to parse a primitive
		CReference ParsePrimitive();

		// Method to parse a boolean primitive
		CReference ParseBool();

		// Method to parse a string constant
		CReference ParseString();

		// Method to parse a literal
		CReference ParseLiteral();

		// Method to parse a vector
		CReference ParseVector();

		// Method to find the line number of a character
		uint32 LineNumber(size_t CharIndex);

		// Input code to be parsed
		std::string m_InputCode;

		// Code stream for streamlining
		std::string m_CodeStream;

		// Line indices in streamlined code
		std::vector<uint32> m_LineIndices;

		// Current index in code stream
		uint32 m_CurrentIndex;
	};
};

#endif // #ifndef _PARSER_H_