//
// Attributes.h
//
// $Id: //poco/1.4/CppParser/include/Poco/CppParser/Attributes.h#2 $
//
// Library: CppParser
// Package: Attributes
// Module:  Attributes
//
// Definition of the Attributes class.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef CppParser_Attributes_INCLUDED
#define CppParser_Attributes_INCLUDED


#include "Poco/CppParser/CppParser.h"
#include <map>


namespace Poco {
namespace CppParser {


class CppParser_API Attributes
	/// This class stores attributes for a symbol table entry.
	/// Attributes are simple name-value pairs, where both
	/// name and values are strings.
{
public:
	typedef std::map<std::string, std::string> AttrMap;
	typedef AttrMap::const_iterator Iterator;
	
	Attributes();
		/// Creates the Attributes object.

	Attributes(const Attributes& attrs);
		/// Creates the Attributes object by copying another one.

	~Attributes();
		/// Destroys the Attributes object.

	Attributes& operator = (const Attributes& attrs);
		/// Assignment operator.
		
	bool has(const std::string& name) const;
		/// Returns true if an attribute with the given name exists.
	
	std::string getString(const std::string& name) const;
		/// Returns the attribute's value as a string.
		///
		/// Throws a Poco::NotFoundException if the attribute does not exist.
		
	std::string getString(const std::string& name, const std::string& defaultValue) const;
		/// Returns the attribute's value as a string, if it exists.
		/// Returns the defaultValue if the attribute does not exist.

	int getInt(const std::string& name) const;
		/// Returns the attribute's value as an integer.
		///
		/// Throws a Poco::NotFoundException if the attribute does not exist.
		/// Throws a Poco::SyntaxException if the stored value is not an integer.
		
	int getInt(const std::string& name, int defaultValue) const;
		/// Returns the attribute's value as an integer, if it exists.
		/// Returns the defaultValue if the attribute does not exist.
		///
		/// Throws a Poco::SyntaxException if the stored value is not an integer.

	bool getBool(const std::string& name) const;
		/// Returns the attribute's value as a boolean.
		/// The returned value is 'true', iff the stored value is not "false".
		/// 
		/// Throws a Poco::NotFoundException if the attribute does not exist.

	bool getBool(const std::string& name, bool defaultValue) const;
		/// Returns the attribute's value as a boolean, if it exists.
		/// The returned value is 'true', iff the stored value is not "false".

	void set(const std::string& name, const std::string& value);
		/// Sets the value of an attribute.
	
	void remove(const std::string& name);
		/// Removes the attribute with the given name.
		/// Does nothing if the attribute does not exist.
		
	const std::string& operator [] (const std::string& name) const;
	std::string& operator [] (const std::string& name);		
		
	Iterator begin() const;
	Iterator end() const;
	
	void clear();
		/// Clears all attributes.

private:
	AttrMap _map;
};


//
// inlines
//
inline Attributes::Iterator Attributes::begin() const
{
	return _map.begin();
}


inline Attributes::Iterator Attributes::end() const
{
	return _map.end();
}


} } // namespace Poco::CppParser


#endif // CppParser_Attributes_INCLUDED
