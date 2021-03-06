//
// CryptoStream.cpp
//
// $Id: //poco/1.4/Crypto/src/CryptoStream.cpp#2 $
//
// Library: Crypto
// Package: Cipher
// Module:  CryptoStream
//
// Copyright (c) 2008, Applied Informatics Software Engineering GmbH.
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


#include "Poco/Crypto/CryptoStream.h"
#include "Poco/Crypto/CryptoTransform.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Exception.h"
#include <algorithm>


#undef min
#undef max


namespace Poco {
namespace Crypto {


//
// CryptoStreamBuf
//


CryptoStreamBuf::CryptoStreamBuf(std::istream& istr, CryptoTransform* pTransform, std::streamsize bufferSize):
	Poco::BufferedStreamBuf(bufferSize, std::ios::in),
	_pTransform(pTransform),
	_pIstr(&istr),
	_pOstr(0),
	_eof(false),
	_buffer(static_cast<std::size_t>(bufferSize))
{
	poco_check_ptr (pTransform);
	poco_assert (bufferSize > 2 * pTransform->blockSize());
}


CryptoStreamBuf::CryptoStreamBuf(std::ostream& ostr, CryptoTransform* pTransform, std::streamsize bufferSize):
	Poco::BufferedStreamBuf(bufferSize, std::ios::out),
	_pTransform(pTransform),
	_pIstr(0),
	_pOstr(&ostr),
	_eof(false),
	_buffer(static_cast<std::size_t>(bufferSize))
{
	poco_check_ptr (pTransform);
	poco_assert (bufferSize > 2 * pTransform->blockSize());
}


CryptoStreamBuf::~CryptoStreamBuf()
{
	try
	{
		close();
	}
	catch (...)
	{
	}
	delete _pTransform;
}


void CryptoStreamBuf::close()
{
	sync();

	if (_pIstr)
	{
		_pIstr = 0;
	}
	else if (_pOstr)
	{
		// Close can be called multiple times. By zeroing the pointer we make
		// sure that we call finalize() only once, even if an exception is
		// thrown.
		std::ostream* pOstr = _pOstr;
		_pOstr = 0;
		
		// Finalize transformation.
		std::streamsize n = _pTransform->finalize(_buffer.begin(), static_cast<std::streamsize>(_buffer.size()));
		
		if (n > 0)
		{
			pOstr->write(reinterpret_cast<char*>(_buffer.begin()), n);
			if (!pOstr->good())
				throw Poco::IOException("Output stream failure");
		}
	}
}


int CryptoStreamBuf::readFromDevice(char* buffer, std::streamsize length)
{
	if (!_pIstr)
		return 0;

	int count = 0;

	while (!_eof)
	{
		int m = (static_cast<int>(length) - count)/2 - static_cast<int>(_pTransform->blockSize());

		// Make sure we can read at least one more block. Explicitely check
		// for m < 0 since blockSize() returns an unsigned int and the
		// comparison might give false results for m < 0.
		if (m <= 0)
			break;

		int n = 0;

		if (_pIstr->good())
		{
			_pIstr->read(reinterpret_cast<char*>(_buffer.begin()), m);
			n = static_cast<int>(_pIstr->gcount());
		}

		if (n == 0)
		{
			_eof = true;

			// No more data, finalize transformation
			count += static_cast<int>(_pTransform->finalize(
				reinterpret_cast<unsigned char*>(buffer + count),
				static_cast<int>(length) - count));
		}
		else
		{
			// Transform next chunk of data
			count += static_cast<int>(_pTransform->transform(
				_buffer.begin(),
				n,
				reinterpret_cast<unsigned char*>(buffer + count),
				static_cast<int>(length) - count));
		}
	}

	return count;
}


int CryptoStreamBuf::writeToDevice(const char* buffer, std::streamsize length)
{
	if (!_pOstr)
		return 0;

	std::size_t maxChunkSize = _buffer.size()/2;
	std::size_t count = 0;

	while (count < length)
	{
		// Truncate chunk size so that the maximum output fits into _buffer.
		std::size_t n = static_cast<std::size_t>(length) - count;
		if (n > maxChunkSize)
			n = maxChunkSize;

		// Transform next chunk of data
		std::streamsize k = _pTransform->transform(
			reinterpret_cast<const unsigned char*>(buffer + count),
			static_cast<std::streamsize>(n),
			_buffer.begin(),
			static_cast<std::streamsize>(_buffer.size()));

		// Attention: (n != k) might be true. In count, we have to track how
		// many bytes from buffer have been consumed, not how many bytes have
		// been written to _pOstr!
		count += n;

		if (k > 0)
		{
			_pOstr->write(reinterpret_cast<const char*>(_buffer.begin()), k);
			if (!_pOstr->good())
				throw Poco::IOException("Output stream failure");
		}
	}

	return static_cast<int>(count);
}


//
// CryptoIOS
//


CryptoIOS::CryptoIOS(std::istream& istr, CryptoTransform* pTransform, std::streamsize bufferSize):
	_buf(istr, pTransform, bufferSize)
{
	poco_ios_init(&_buf);
}


CryptoIOS::CryptoIOS(std::ostream& ostr, CryptoTransform* pTransform, std::streamsize bufferSize):
	_buf(ostr, pTransform, bufferSize)
{
	poco_ios_init(&_buf);
}


CryptoIOS::~CryptoIOS()
{
}


CryptoStreamBuf* CryptoIOS::rdbuf()
{
	return &_buf;
}


//
// CryptoInputStream
//


CryptoInputStream::CryptoInputStream(std::istream& istr, CryptoTransform* pTransform, std::streamsize bufferSize):
	CryptoIOS(istr, pTransform, bufferSize),
	std::istream(&_buf)
{
}


CryptoInputStream::CryptoInputStream(std::istream& istr, Cipher& cipher, std::streamsize bufferSize):
	CryptoIOS(istr, cipher.createEncryptor(), bufferSize),
	std::istream(&_buf)
{
}


CryptoInputStream::~CryptoInputStream()
{
}


//
// CryptoOutputStream
//


CryptoOutputStream::CryptoOutputStream(std::ostream& ostr, CryptoTransform* pTransform, std::streamsize bufferSize):
	CryptoIOS(ostr, pTransform, bufferSize),
	std::ostream(&_buf)
{
}


CryptoOutputStream::CryptoOutputStream(std::ostream& ostr, Cipher& cipher, std::streamsize bufferSize):
	CryptoIOS(ostr, cipher.createDecryptor(), bufferSize),
	std::ostream(&_buf)
{
}


CryptoOutputStream::~CryptoOutputStream()
{
}


void CryptoOutputStream::close()
{
	_buf.close();
}


//
// EncryptingInputStream
//


EncryptingInputStream::EncryptingInputStream(std::istream& istr, Cipher& cipher, std::streamsize bufferSize):
	CryptoIOS(istr, cipher.createEncryptor(), bufferSize),
	std::istream(&_buf)
{
}


EncryptingInputStream::~EncryptingInputStream()
{
}


//
// EncryptingOuputStream
//


EncryptingOutputStream::EncryptingOutputStream(std::ostream& ostr, Cipher& cipher, std::streamsize bufferSize):
	CryptoIOS(ostr, cipher.createEncryptor(), bufferSize),
	std::ostream(&_buf)
{
}


EncryptingOutputStream::~EncryptingOutputStream()
{
}


void EncryptingOutputStream::close()
{
	_buf.close();
}


//
// DecryptingInputStream
//


DecryptingInputStream::DecryptingInputStream(std::istream& istr, Cipher& cipher, std::streamsize bufferSize):
	CryptoIOS(istr, cipher.createDecryptor(), bufferSize),
	std::istream(&_buf)
{
}


DecryptingInputStream::~DecryptingInputStream()
{
}


//
// DecryptingOuputStream
//


DecryptingOutputStream::DecryptingOutputStream(std::ostream& ostr, Cipher& cipher, std::streamsize bufferSize):
	CryptoIOS(ostr, cipher.createDecryptor(), bufferSize),
	std::ostream(&_buf)
{
}


DecryptingOutputStream::~DecryptingOutputStream()
{
}


void DecryptingOutputStream::close()
{
	_buf.close();
}


} } // namespace Poco::Crypto
