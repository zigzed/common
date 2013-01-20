/**
 * pugixml parser - version 1.0
 * --------------------------------------------------------
 * Copyright (C) 2006-2010, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
 * Report bugs and download new versions at http://pugixml.org/
 *
 * This library is distributed under the MIT License. See notice at the end
 * of this file.
 *
 * This work is based on the pugxml parser, which is:
 * Copyright (C) 2003, by Kristen Wegner (kristen@tima.net)
 */

#ifndef HEADER_CXXCONFIG_HPP
#define HEADER_CXXCONFIG_HPP

// Uncomment this to enable wchar_t mode
// #define CXXXML_WCHAR_MODE

// Uncomment this to disable XPath
// #define CXXXML_NO_XPATH

// Uncomment this to disable STL
// Note: you can't use XPath with CXXXML_NO_STL
// #define CXXXML_NO_STL

// Uncomment this to disable exceptions
// Note: you can't use XPath with CXXXML_NO_EXCEPTIONS
// #define CXXXML_NO_EXCEPTIONS

// Set this to control attributes for public classes/functions, i.e.:
// #define CXXXML_API __declspec(dllexport) // to export all public symbols from DLL
// #define CXXXML_CLASS __declspec(dllimport) // to import all classes from DLL
// #define CXXXML_FUNCTION __fastcall // to set calling conventions to all public functions to fastcall
// In absence of CXXXML_CLASS/CXXXML_FUNCTION definitions CXXXML_API is used instead

#endif

/**
 * Copyright (c) 2006-2010 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
