//=============================================================================
// RSerializer.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

enum class ESerializeMode : uint8_t
{
	Read,
	Write,
};

class RSerializer
{
public:
	~RSerializer();

	/// Open a file for serialization
	void Open(const std::string& filename, ESerializeMode mode);

	/// Close file stream of serializer
	FORCEINLINE void Close()
	{
		if (m_FileStream.is_open())
			m_FileStream.close();
	}

	/// Check if file stream is open
	FORCEINLINE bool IsOpen() const
	{
		return m_FileStream.is_open();
	}

	/// Check if serializer is in reading mode
	FORCEINLINE bool IsReading() const
	{
		return OperationMode == ESerializeMode::Read;
	}

	/// Check if serializer is in writing mode
	FORCEINLINE bool IsWriting() const
	{
		return OperationMode == ESerializeMode::Write;
	}

	/// Serialize a string file header
	///   Write mode : Write header string to file stream.
	///                Always returns true
	///
	///   Read mode  : Read header string and compare it with given header.
	///                Returns true if both header equal, false otherwise.
	bool EnsureHeader(const char* header, UINT size);

	/// Serialize a std::vector with plain data type
	template<typename T>
	void SerializeVector(std::vector<T>& vec)
	{
		if (OperationMode == ESerializeMode::Write)
		{
			UINT size = (UINT)vec.size();
			m_FileStream.write((char*)&size, sizeof(size));
			if (size)
				m_FileStream.write((char*)vec.data(), sizeof(T) * size);
		}
		else
		{
			assert(!m_FileStream.eof());
			UINT size;
			m_FileStream.read((char*)&size, sizeof(size));
			vec.resize(size);
			if (size)
				m_FileStream.read((char*)vec.data(), sizeof(T) * size);
		}
	}

	/// Serialize a std::vector with a custom serialization function
	template<typename T>
	void SerializeVector(std::vector<T>& vec, void (RSerializer::*func)(T&))
	{
		UINT size;
		if (OperationMode == ESerializeMode::Write)
		{
			size = (UINT)vec.size();
			m_FileStream.write((char*)&size, sizeof(size));
		}
		else
		{
			assert(!m_FileStream.eof());
			m_FileStream.read((char*)&size, sizeof(size));
			vec.resize(size);
		}

		for (UINT i = 0; i < size; i++)
		{
			(this->*func)(vec[i]);
		}
	}

	/// Serialize a plain data type
	template<typename T>
	void SerializeData(T& data)
	{
		if (OperationMode == ESerializeMode::Write)
		{
			m_FileStream.write((char*)&data, sizeof(T));
		}
		else
		{
			assert(!m_FileStream.eof());
			m_FileStream.read((char*)&data, sizeof(T));
		}
	}

	/// Serialize a string
	template<>
	void SerializeData<std::string>(std::string& str)
	{
		if (OperationMode == ESerializeMode::Write)
		{
			UINT size = (UINT)str.size();
			m_FileStream.write((char*)&size, sizeof(size));
			if (size)
				m_FileStream.write((char*)str.data(), size);
		}
		else
		{
			assert(!m_FileStream.eof());
			UINT size;
			m_FileStream.read((char*)&size, sizeof(size));
			str.resize(size);
			if (size)
				m_FileStream.read((char*)str.data(), size);
		}
	}

	/// Serialize an array of plain data type
	template<typename T>
	void SerializeArray(T** arr, UINT size)
	{
		if (size)
		{
			if (OperationMode == ESerializeMode::Write)
				m_FileStream.write((char*)*arr, sizeof(T) * size);
			else
			{
				assert(!m_FileStream.eof());
				*arr = new T[size];
				m_FileStream.read((char*)*arr, sizeof(T) * size);
			}
		}
	}

	/// Serialize a class which implemented function 'Serialize(RSerializer&)'
	template<typename T>
	void SerializeObject(T& obj)
	{
		obj.Serialize(*this);
	}

	/// Serialize a pointer of a class which implemented function 'Serialize(RSerializer&)'.
	/// When in read mode, pointer should be initially unused.
	template<typename T>
	void SerializeObjectPtr(T** pObj)
	{
		int flag = (*pObj) ? 1 : 0;
		SerializeData(flag);
		if (flag)
		{
			if (OperationMode == ESerializeMode::Read)
			{
				assert(!m_FileStream.eof());
				*pObj = new T();
			}
			(*pObj)->Serialize(*this);
		}
	}

private:
	/// Current operation mode (read or write)
	ESerializeMode	OperationMode;
	std::fstream	m_FileStream;
};

