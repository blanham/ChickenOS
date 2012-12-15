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
#ifndef _BASICTYPES_H_
#define _BASICTYPES_H_

// Header files
#include <cstdlib>

// Basic byte type
typedef unsigned char byte;

// Signed integer types
typedef signed char      int8;
typedef signed short     int16;
typedef signed int       int32;
typedef signed long long int64;

// Unsigned integer types
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

// Floating point types
typedef float  float32;
typedef double float64;

/*******************************************************
* Function: ConvertEndian()
* Purpose : Convert the endianness of variables
* Initial : Max Payne on March 10, 2005
********************************************************
Revisions and bug fixes:
*/
template <class Type>
inline Type ConvertEndian(Type InVar)
{

#ifdef MAC // For big endian platforms

	// Declare a variable for the output
	Type OutVar;

	// Write the bytes of the input to the output, but in reverted order
	for (int i = 0; i < sizeof(InVar); ++i)
		((byte*)&OutVar)[sizeof(InVar) - i - 1] = ((byte*)&InVar)[i];

	// Return the output in reverted endian
	return OutVar;

#else // For little endian platforms

	// Do not change the endian
	return InVar;

#endif // #ifdef MAC

}

#endif // #ifndef _BASICTYPES_H_