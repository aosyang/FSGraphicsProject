//=============================================================================
// RSerializer.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RSERIALIZER_H
#define _RSERIALIZER_H

enum class ESerializeMode : UINT8
{
	Read,
	Write,
};

class RSerializer
{
public:
	~RSerializer()
	{
		Close();
	}

	void Open(const string& filename, ESerializeMode mode)
	{
		m_Mode = mode;

		m_FileStream.open(filename.c_str(), (mode == ESerializeMode::Read ? ios::in : ios::out) | ios::binary);

		if (!m_FileStream.is_open())
			return;
	}

	void Close()
	{
		if (m_FileStream.is_open())
			m_FileStream.close();
	}

	bool IsOpen()
	{
		return m_FileStream.is_open();
	}

	bool IsReading()
	{
		return m_Mode == ESerializeMode::Read;
	}

	bool EnsureHeader(const char* header, UINT size)
	{
		if (m_Mode == ESerializeMode::Write)
		{
			m_FileStream.write(header, size);
		}
		else
		{
			assert(!m_FileStream.eof());
			char* pBuf = new char[size];
			m_FileStream.read(pBuf, size);

			for (UINT i = 0; i < size; i++)
			{
				if (pBuf[i] != header[i])
				{
					delete[] pBuf;
					return false;
				}
			}

			delete[] pBuf;
		}

		return true;
	}

	template<typename T>
	void SerializeVector(vector<T>& vec)
	{
		if (m_Mode == ESerializeMode::Write)
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

	template<typename T>
	void SerializeVector(vector<T>& vec, void (RSerializer::*func)(T&))
	{
		UINT size;
		if (m_Mode == ESerializeMode::Write)
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

	template<typename T>
	void SerializeData(T& data)
	{
		if (m_Mode == ESerializeMode::Write)
		{
			m_FileStream.write((char*)&data, sizeof(T));
		}
		else
		{
			assert(!m_FileStream.eof());
			m_FileStream.read((char*)&data, sizeof(T));
		}
	}

	template<>
	void SerializeData<string>(string& str)
	{
		if (m_Mode == ESerializeMode::Write)
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

	template<typename T>
	void SerializeArray(T** arr, UINT size)
	{
		if (size)
		{
			if (m_Mode == ESerializeMode::Write)
				m_FileStream.write((char*)*arr, sizeof(T) * size);
			else
			{
				assert(!m_FileStream.eof());
				*arr = new T[size];
				m_FileStream.read((char*)*arr, sizeof(T) * size);
			}
		}
	}

	template<typename T>
	void SerializeObject(T& obj)
	{
		obj.Serialize(*this);
	}

	template<typename T>
	void SerializeObjectPtr(T** pObj)
	{
		int flag = (*pObj) ? 1 : 0;
		SerializeData(flag);
		if (flag)
		{
			if (m_Mode == ESerializeMode::Read)
			{
				assert(!m_FileStream.eof());
				*pObj = new T();
			}
			(*pObj)->Serialize(*this);
		}
	}

private:
	fstream			m_FileStream;
	ESerializeMode	m_Mode;
};

#endif
