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
#include <ctype.h>
#include <cstring>
#include <fstream>
#include "parser.h"
#include "primitives.h"

// Include the XLSE namespace
using namespace XLSE;

/*******************************************************
* Function: CParser::CParser()
* Purpose : Constructor for parser class
* Initial : Max Payne on February 10, 2006
********************************************************
Revisions and bug fixes:
*/
CParser::CParser()
{
}

/*******************************************************
* Function: CParser::CParser()
* Purpose : Destructor for parser class
* Initial : Max Payne on February 10, 2006
********************************************************
Revisions and bug fixes:
*/
CParser::~CParser()
{
}

/*******************************************************
* Function: CParser::ParseFile()
* Purpose : Parse an XLSE script file
* Initial : Max Payne on February 10, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CParser::ParseFile(const std::string& FilePath)
{
	// Declare a file for input
	std::ifstream CodeFile;

	// Declare a string to store the code
	std::string Code;
	
	// Declare a buffer for reading
	char ReadBuf[1024];
		
	// Attempt to open the code file
	CodeFile.open(FilePath.c_str());

	// If we could not open the file
	if (!CodeFile)
	{
		// Throw an exception
		throw CParseError("Could not open file for parsing");
	}

	// Read the code file
	while (!CodeFile.eof())
	{
		// Clear the buffer
		memset(ReadBuf, 0, sizeof(ReadBuf));

		// Read and fill the buffer
		CodeFile.read(ReadBuf, sizeof(ReadBuf) - 1);

		// Add the read code to the string
		Code += ReadBuf;
	}

	// Parse the input code stream
	return ParseStream(Code);
}

/*******************************************************
* Function: CParser::ParseStream()
* Purpose : Parse an XLSE code stream
* Initial : Max Payne on February 10, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CParser::ParseStream(const std::string& InputCode)
{
	// Store the input code
	m_InputCode = InputCode;

	// Streamline the input code
	Streamline();

	// Reset the current parsing index
	m_CurrentIndex = 0;

	// Parse the program recursively from the first level
	return ParseSList(true);
}

/*******************************************************
* Function: CParser::Streamline()
* Purpose : Streamline input code
* Initial : Max Payne on February 10, 2006
********************************************************
Revisions and bug fixes:
*/
void CParser::Streamline()
{
	// Clear the code stream
	m_CodeStream.clear();

	// Reserve space in the code stream
	m_CodeStream.reserve(m_InputCode.length());

	// Clear the line indices
	m_LineIndices.clear();

	// Declare characters for stream processing
	char LastChar = '\0';
	char ThisChar = '\0';
	char NextChar = '\0';

	// Variable to indicate if we are in a line-wide comment
	bool LineComment = false;

	// Variable to indicate if we are in a string constant
	bool StringConstant = false;

	// Variable to indicate the C-style comment level
	size_t CommentLevel = 0;

	// For each character of the input
	for (size_t CharIndex = 0; CharIndex < m_InputCode.length(); ++CharIndex)
	{
		// Read this character and the next
		ThisChar = m_InputCode[CharIndex];
		NextChar = m_InputCode[CharIndex + 1];

		// If this character is a newline
		if (ThisChar == '\n')
		{
			// Add a line index
			m_LineIndices.push_back((uint32)m_CodeStream.length());

			// If we were in a line-wide comment
			if (LineComment)
			{
				// This is the end of the line comment
				LineComment = false;

				// Move to the next character
				continue;
			}
		}

		// If this is the beggining or the end of a string constant
		if (!LineComment && !CommentLevel && ThisChar == '\"')
		{
			// Switch the string constant state
			StringConstant = !StringConstant;
		}

		// If we are in a string constant
		if (StringConstant)
		{
			// Add this character right away
			m_CodeStream += ThisChar;
			LastChar = ThisChar;
			continue;
		}

		// If this is the start of a line-wide comment
		if (!CommentLevel && ThisChar == '/' && NextChar == '/')
		{
			// Indicate that we are in a line-wide comment
			LineComment = true;
		}

		// If this is the beggining of a C-style comment
		if (!LineComment && ThisChar == '/' && NextChar == '*')
		{
			// Increment the comment level
			++CommentLevel;

			// Skip the comment
			++CharIndex;
			continue;
		}

		// If this is the end of a C-style comment
		if (CommentLevel > 0 && ThisChar == '*' && NextChar == '/')
		{
			// Decrement the comment level
			--CommentLevel;

			// Skip the comment
			++CharIndex;
			continue;
		}

		// If we are in a line-wide comment, do not add anything to the stream
		if (LineComment || CommentLevel > 0)
			continue;

		// If this character is whitespace
		if (isspace(ThisChar))
		{
			// Consider it a space
			ThisChar = ' ';
		}

		// If the next character is whitespace
		if (isspace(NextChar))
		{
			// Consider it a space
			NextChar = ' ';
		}

		// If this character is whitespace
		if (ThisChar == ' ')
		{
			// If the next char is whitespace, 
			// or the last was an opening delimiter or a space,
			// or the next is a closing delimiter
			if (NextChar == ' ' || 
			   (LastChar == '\0' || LastChar == '(' || LastChar == ' ') ||
			   (NextChar == '\0' || NextChar == ')'))
			{
				// Ignore this space
				continue;
			}
		}

		// Add this character to the stream
		m_CodeStream += ThisChar;

		// Store the last character added
		LastChar = ThisChar;
	}
}

/*******************************************************
* Function: CParser::ParseSList()
* Purpose : Parse an s-list
* Initial : Max Payne on February 11, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CParser::ParseSList(bool FirstLevel)
{
	// If this is not the first level
	if (!FirstLevel)
	{
		// If the list doesn't start with a '(' character
		if (m_CodeStream[m_CurrentIndex] != '(')
		{
			// Throw an exception
			throw CParseError("Invalid s-list", LineNumber(m_CurrentIndex));
		}

		// skip this character
		++m_CurrentIndex;
	}

	// Create a first pair for the list
	CRef<CPair> StartPair = NIL;

	// If the code stream is empty
	if (m_CodeStream.empty())
	{
		// Return the empty list
		return StartPair;
	}

	// Declare a reference to the current pair
	CRef<CPair> CurrentPair = StartPair;

	// Declare variables for the current and next characters
	char ThisChar;
	char NextChar;

	// Declare a variable for the pair flag
	bool PairFlag = false;

	// Declare a variable for the pair end flag
	bool PairEnd = false;

	// For each character
	for (;; ++m_CurrentIndex)
	{
		// If we are past the length of the code stream
		if (m_CurrentIndex > m_CodeStream.length())
		{
			// Throw an exception
			throw CParseError("Unexpected end of input in list", LineNumber(m_CurrentIndex));
		}

		// Update the current and next characters
		ThisChar = m_CodeStream[m_CurrentIndex];
		NextChar = m_CodeStream[m_CurrentIndex + 1];

		// If we are on the first level
		if (FirstLevel)
		{
			// If this is the end of string
			if (ThisChar == '\0')
			{
				// S-list parsing complete
				break;
			}

			// If this is an end of list
			if (ThisChar == ')')
			{
				// Throw an error
				throw CParseError("Wrongly terminated first-level list", LineNumber(m_CurrentIndex));
			}
		}
		else
		{
			// If this is the end of the list, parsing complete
			if (ThisChar == ')')
			{
				// S-list parsing complete
				break;
			}

			// If this is the end of string
			if (ThisChar == '\0')
			{
				// Throw an exception
				throw CParseError("Unterminated s-list", LineNumber(m_CurrentIndex));
			}
		}

		// If this is whitespace
		if (isspace(ThisChar))
		{
			// Move to the next character
			continue;
		}

		// If the pair should have ended but did not
		if (PairEnd)
		{
			// Throw an exception
			throw CParseError("Pairs can only have two members", LineNumber(m_CurrentIndex));
		}

		// If this is a pair marker
		if (ThisChar == '.' && isspace(NextChar))
		{
			// If no items were added yet
			if (!CurrentPair)
			{
				// Throw an exception
				throw CParseError("Cannot create pair with no car member", LineNumber(m_CurrentIndex));
			}

			// Enable the pair flag
			PairFlag = true;

			// Move to the next character
			continue;
		}

		// Attempt to parse the current item
		CReference CurrentItem = ParsePrimitive();

		// If this is a pair
		if (PairFlag)
		{
			// Add the new item to the pair
			CurrentPair->SetCdr(CurrentItem);

			// The second pair item has been added, must end the pair
			PairEnd = true;
		}

		// This is not a pair, continue the list
		else
		{
			// Create a new list pair
			CRef<CPair> NewPair = CPair::Create(NIL, NIL);

			// If there are previous list items
			if (CurrentPair)
			{
				// Link the previous pair to this one
				CurrentPair->SetCdr(NewPair);
			}
			else
			{
				// Set the starting pair
				StartPair = NewPair;
			}

			// Make the new pair the current pair
			CurrentPair = NewPair;

			// Add the new item to the list
			CurrentPair->SetCar(CurrentItem);
		}
	}

	// Return a reference to the start pair
	return StartPair;
}

/*******************************************************
* Function: CParser::ParsePrimitive()
* Purpose : Parse a language primitive
* Initial : Max Payne on February 13, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CParser::ParsePrimitive()
{
	// If we are past the end of the stream
	if (m_CurrentIndex >= m_CodeStream.length())
	{
		// Throw an exception
		throw CParseError("Primitive parsing failed, unexpected end of input", LineNumber(m_CurrentIndex));
	}

	// Extract the current and next characters
	char ThisChar = m_CodeStream[m_CurrentIndex];
	char NextChar = m_CodeStream[m_CurrentIndex + 1];

	// If this is an invalid character
	if (isspace(ThisChar) || ThisChar == ')')
	{
		// Throw an exception
		throw CParseError("Invalid primitive", LineNumber(m_CurrentIndex));
	}

	// Declare a reference for the new primitive
	CReference NewItem;

	// If this is a quoted item
	if (ThisChar == '\'')
	{
		// Move to the next character
		++m_CurrentIndex;

		// Attempt to parse the item recursively
		CReference QuotedItem = ParsePrimitive();

		// Set the new item reference
		NewItem = CQuote::Create(QuotedItem);
	}

	// If this is the start of a list
	else if (ThisChar == '(')
	{
		// Parse the list
		NewItem = ParseSList(false);
	}

	// If this is the start of a vector
	else if (ThisChar == '#' && NextChar == '(')
	{
		// Parse the vector
		NewItem = ParseVector();
	}

	// If this is the start of a boolean
	else if (ThisChar == '#')
	{
		// Parse the boolean
		NewItem = ParseBool();
	}

	// If this is the start of a string constant
	else if (ThisChar == '\"')
	{
		// Parse the string constant
		NewItem = ParseString();
	}

	// Otherwise, this is a literal
	else
	{
		// Parse the literal
		NewItem = ParseLiteral();
	}

	// Return the new item
	return NewItem;
}

/*******************************************************
* Function: CParser::ParseBool()
* Purpose : Parse a boolean constant
* Initial : Max Payne on February 11, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CParser::ParseBool()
{
	// If we are past the end of the stream
	if (m_CurrentIndex >= m_CodeStream.length())
	{
		// Throw an exception
		throw CParseError("Boolean parsing failed, unexpected end of input", LineNumber(m_CurrentIndex));
	}

	// Make sure the constant begins with '#'
	if (m_CodeStream[m_CurrentIndex] != '#')
	{
		// Throw an exception
		throw CParseError("Invalid boolean constant", LineNumber(m_CurrentIndex));
	}

	// Declare a boolean for the value
	bool Value;

	// Extract the boolean value character
	char ThisChar = m_CodeStream[m_CurrentIndex + 1];

	// If the constant is '#T'
	if (ThisChar == 'T' || ThisChar == 't')
	{
		// Store a true boolean value
		Value = true;
	}

	// If the constant is '#F'
	else if (ThisChar == 'F' || ThisChar == 'f')
	{
		// Store a false boolean value
		Value = false;
	}

	// Otherwise, invalid boolean
	else
	{
		// Throw an exception
		throw CParseError("Invalid boolean constant", LineNumber(m_CurrentIndex));
	}

	// Move to the next character
	m_CurrentIndex += 1;

	// Return the boolean value item
	return CBool::Create(Value);
}

/*******************************************************
* Function: CParser::ParseString()
* Purpose : Parse a string constant
* Initial : Max Payne on February 12, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CParser::ParseString()
{
	// If we are past the end of the stream
	if (m_CurrentIndex >= m_CodeStream.length())
	{
		// Throw an exception
		throw CParseError("String parsing failed, unexpected end of input", LineNumber(m_CurrentIndex));
	}

	// Make sure the string begins with a '\"' character
	if (m_CodeStream[m_CurrentIndex] != '\"')
	{
		// Throw an exception
		throw CParseError("String literal does not start appropriately", LineNumber(m_CurrentIndex));
	}

	// Declare a string for parsing
	std::string String;

	// Declare a variable for the current character
	char ThisChar;

	// For each character
	for (++m_CurrentIndex;; ++m_CurrentIndex)
	{
		// If we are past the end of the code stream
		if (m_CurrentIndex >= m_CodeStream.length())
		{
			// Throw an exception
			throw CParseError("Unterminated string literal", LineNumber(m_CurrentIndex));
		}

		// Update the current character
		ThisChar = m_CodeStream[m_CurrentIndex];

		// If this is the end of string constant, parsing complete
		if (ThisChar == '\"')
			break;

		// If this is the beggining of an escape sequence
		if (ThisChar == '\\')
		{
			// Move to the next character
			++m_CurrentIndex;

			// Switch on the current character
			switch (m_CodeStream[m_CurrentIndex])
			{
				case '\"':
				ThisChar = '\"';
				break;

				case '\'':
				ThisChar = '\'';
				break;

				case 't':
				ThisChar = '\t';
				break;

				case 'n':
				ThisChar = '\n';
				break;

				case 'r':
				ThisChar = '\r';
				break;

				case '0':
				ThisChar = '\0';
				break;

				// Unknown escape sequence
				default:
				{
					// Throw an exception
					throw CParseError("Unknown escape sequence", LineNumber(m_CurrentIndex));
				}
			};
		}

		// Add the current character to the string
		String += ThisChar;
	}

	// Return a pointer to the new string
	return CString::Create(String);
}

/*******************************************************
* Function: CParser::ParseLiteral()
* Purpose : Parse a literal (int, float, symbol)
* Initial : Max Payne on February 12, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CParser::ParseLiteral()
{
	// If we are past the end of the stream
	if (m_CurrentIndex >= m_CodeStream.length())
	{
		// Throw an exception
		throw CParseError("Literal parsing failed, unexpected end of input", LineNumber(m_CurrentIndex));
	}

	// Declare a string to store the literal
	std::string Literal;

	// Test whether this is a number or not
	bool IsNumeric = 
		isdigit(m_CodeStream[m_CurrentIndex]) ||
		(m_CodeStream[m_CurrentIndex] == '-' && isdigit(m_CodeStream[m_CurrentIndex + 1])) ||
		(m_CodeStream[m_CurrentIndex] == '+' && isdigit(m_CodeStream[m_CurrentIndex + 1]))
		? true:false;

	// Declare variables to store if this could be floating-point
	bool IsFloat = false;

	// For each character
	for (; m_CurrentIndex < m_CodeStream.length(); ++m_CurrentIndex)
	{
		// Store the current character
		char ThisChar = m_CodeStream[m_CurrentIndex];

		// If this is the end of the literal, stop reading
		if (isspace(ThisChar) || ThisChar == ')')
			break;

		// If this character is a period, this could be a float
		if (ThisChar == '.')
			IsFloat = true;

		// Add this char to the literal
		Literal += ThisChar;
	}

	// Move back to the last character
	--m_CurrentIndex;

	// Declare a variable to store a reference to the new item
	CReference NewItem;

	// If this is a number
	if (IsNumeric)
	{
		// If this is floating-point
		if (IsFloat)
		{
			// Parse the float
			float32 Float = (float32)atof(Literal.c_str());

			// Create a new float object
			NewItem = CFloat::Create(Float);
		}
		else
		{
			// Parse the int
			int32 Int = atoi(Literal.c_str());

			// Create a new int object
			NewItem = CInt::Create(Int);
		}
	}
	else
	{
		// Create a new symbol object
		NewItem = CSymbol::Create(Literal);
	}

	// Return the new item
	return NewItem;
}

/*******************************************************
* Function: CParser::ParseVector()
* Purpose : Parse a vector primitive
* Initial : Max Payne on February 13, 2006
********************************************************
Revisions and bug fixes:
*/
CReference CParser::ParseVector()
{
	// If we are past the end of the stream
	if (m_CurrentIndex >= m_CodeStream.length())
	{
		// Throw an exception
		throw CParseError("Vector parsing failed, unexpected end of input", LineNumber(m_CurrentIndex));
	}

	// If the vector does not start appropriately
	if (m_CodeStream[m_CurrentIndex] != '#' || m_CodeStream[m_CurrentIndex + 1] != '(')
	{
		// Throw an exception
		throw CParseError("Invalid vector primitive", LineNumber(m_CurrentIndex));
	}

	// Create a new vector object
	CRef<CVector> NewVector = CVector::Create();

	// Declare variables for the current and next characters
	char ThisChar;
	char NextChar;

	// For each character
	for (m_CurrentIndex += 2;; ++m_CurrentIndex)
	{
		// If we are past the length of the code stream
		if (m_CurrentIndex > m_CodeStream.length())
		{
			// Throw an exception
			throw CParseError("Unexpected end of input in vector", LineNumber(m_CurrentIndex));
		}

		// Update the current and next characters
		ThisChar = m_CodeStream[m_CurrentIndex];
		NextChar = m_CodeStream[m_CurrentIndex + 1];

		// If this is the end of the vector
		if (ThisChar == ')')
		{
			// Vector parsing complete
			break;
		}

		// If this is whitespace
		else if (isspace(ThisChar))
		{
			// Move to the next character
			continue;
		}

		// Attempt to parse the current item
		CReference CurrentItem = ParsePrimitive();

		// If the item parsing failed
		if (!CurrentItem)
		{
			// Throw an exception
			throw CParseError("Item parsing failed", LineNumber(m_CurrentIndex));
		}

		// Add the item to the vector
		NewVector->AddItem(CurrentItem);
	}

	// Return a reference to the new vector
	return NewVector;
}

/*******************************************************
* Function: CParser::LineNumber()
* Purpose : Handle parsing errors
* Initial : Max Payne on February 14, 2006
********************************************************
Revisions and bug fixes:
*/
uint32 CParser::LineNumber(size_t CharIndex)
{
	// Start at line index 1
	uint32 LineNumber = 1;

	// For each line index in the line indices
	for (uint32 i = 0; i < m_LineIndices.size(); ++i)
	{
		// Increment the line index as long as we are farther than this new line
		if (CharIndex >= m_LineIndices[i])
			++LineNumber;
		else
			break;
	}

	// Return the line number
	return LineNumber;
}
