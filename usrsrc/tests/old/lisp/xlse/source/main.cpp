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
#include <iostream>
#include <cstring>
#include <cstdio>
#include "primitives.h"
#include "interpreter.h"
#include "lisp.h"
#include "references.h"

/*******************************************************
* Function: ConsolePrint()
* Purpose : Print function for the console interpreter
* Initial : Max Payne on September 21, 2006
********************************************************
Revisions and bug fixes:
*/
XLSE::CReference ConsolePrint(const XLSE::CRef<XLSE::CPair>& Args, XLSE::CInterpreter& Interpreter)
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

	// Print the output string
	std::cout << Output;

	// Return a string containing the output
	return XLSE::CString::Create(Output);
}

/*******************************************************
* Function: main()
* Purpose : Starting point for console applications
* Initial : Max Payne on February 10, 2006
********************************************************
Revisions and bug fixes:
*/
int main(int argc, char** argv)
{
	// Create a parser object
	//printf("fuck\n");
	XLSE::CParser Parser;
	argc = 0;
	argv = 0;
	// Create an interpreter object
	XLSE::CInterpreter Interpreter;

	// Create a LISP core object
	XLSE::CLISPCore LISPCore;

	LISPCore.Bind("print", XLSE::CExtFunction::Create(ConsolePrint));

	// Set the basic LISP bindings in the interpreter
	Interpreter.SetBaseEnv(LISPCore);

	// If an argument was provided
	if (argc > 1)
	{
		// Attempt to parse and execute the program
		try
		{
			XLSE::CReference Program = Parser.ParseFile(argv[1]);

			std::cout << "PROGRAM: " << std::endl << Program->ToString() << std::endl;

			Interpreter.LoadProgram(Program);

			Interpreter.RunProgram();

			std::cout << "RESULT: " << Interpreter.GetResult()->ToString() << std::endl;
		}

		// Catch any parsing error
		catch (XLSE::CParseError Error)
		{
			std::cout << "PARSING FAILED: " << Error.GetLineNumber() << ": " << Error.GetErrorText() << std::endl;
		}

		// Catch any runtime error
		catch (XLSE::CRunError Error)
		{
			std::cout << "RUNTIME ERROR: " << Error.GetErrorText() << std::endl;
		}
	}

	// Otherwise, if no argument was provided
	else
	{
		// Declare a buffer for reading the input
		char Buffer[1024];

		// While the user has not requested to exit
		while (true) 
		{
			// Print the prompt symbol
			std::cout << ">> ";

			// Read one line of input
			std::cin.getline(Buffer, sizeof(Buffer));

			// If the user wants to exit, break out of the loop
			if (!strcmp(Buffer, "exit") || !strcmp(Buffer, "quit") || std::cin.eof())
				break;

			// Attempt to parse and execute a single line of code
			try
			{
				XLSE::CReference Program = Parser.ParseStream(Buffer);

				Interpreter.LoadProgram(Program);

				Interpreter.RunProgram();
				
				std::cout << std::endl << Interpreter.GetResult()->ToString() << std::endl;
			}

			// Catch any parsing error
			catch (XLSE::CParseError Error)
			{
				std::cout << std::endl << "PARSING FAILED: " << Error.GetLineNumber() << ": " << Error.GetErrorText() << std::endl << std::endl;
			}

			// Catch any runtime error
			catch (XLSE::CRunError Error)
			{
				std::cout << std::endl << "RUNTIME ERROR: " << std::endl << Error.GetErrorText() << std::endl;
			}
		}
	}
}
