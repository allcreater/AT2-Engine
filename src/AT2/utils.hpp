#include <functional>

namespace AT2::Utils
{

// constant-sized array in heap
template <typename T> class dynarray
{
public:
	dynarray(size_t _size) :
		m_size(_size)
	{
		if (_size <= 0)
			throw std::invalid_argument("_size");

		m_data = new T [_size];
	}

	~dynarray()
	{
		delete [] m_data;
	}

	dynarray (const dynarray<T>& other)
	{
		copy(other);
	}
	dynarray (dynarray<T>&& other) :
		m_size(other.m_size),
		m_data(other.m_data)
	{
	}


	T& operator= (const dynarray<T>&other)
	{
		clear();
		copy(other);

		return (*this);
	}
	T& operator= (dynarray<T>&& other)
	{
		if (&other != this)
		{
			clear();
			m_size = other.m_size;
			m_data = other.m_data;
		}

		return (*this);
	}
public:
	T& at (size_t _index)
	{
		if (_index >= m_size)
			std::out_of_range("_index is out of range");

		return m_data [_index];
	}
	const T& at (size_t _index) const
	{
		if (_index >= m_size)
			std::out_of_range("_index is out of range");

		return m_data [_index];
	}

	T& operator [] (size_t _index)
	{
		return m_data [_index];
	}
	const T& operator [] (size_t _index) const
	{
		return m_data [_index];
	}

	const T* data() const
	{
		return m_data;
	}

	size_t size() const
	{
		return m_size;
	}

private:
	void clear()
	{
		m_size = 0;
		delete m_data;
	}

	void copy (const dynarray<T>& other)
	{
		m_size = other.m_size;
		m_data = new T[m_size];
		memcpy(m_data, other.m_data, m_size * sizeof(T));
	}

private:
	T* m_data;
	size_t m_size;

};

//something like std::span from C++ 20 =/
template <typename T> class wraparray
{
public:
	wraparray(size_t _size, T* _data) :
		m_size(_size),
		m_data(_data)
	{
		assert(_size > 0);
		assert(m_data);
	}

	~wraparray() //maybe I can make destructor virtual so vertex buffers will get an ability to automatically unlock
	{
	}

	wraparray(const dynarray<T>& other) :
		m_size(other.m_size),
		m_data(other.m_data)
	{
	}

	T& operator= (wraparray<T>& other)
	{
		if (&other != this)
		{
			m_size = other.m_size;
			m_data = other.m_data;
		}

		return (*this);
	}
public:
	T& at(size_t _index)
	{
		if (_index >= m_size)
			std::out_of_range("_index is out of range");

		return m_data[_index];
	}
	const T& at(size_t _index) const
	{
		if (_index >= m_size)
			std::out_of_range("_index is out of range");

		return m_data[_index];
	}

	T& operator [] (size_t _index)
	{
		return m_data[_index];
	}
	const T& operator [] (size_t _index) const
	{
		return m_data[_index];
	}

	const T* data() const
	{
		return m_data;
	}

	size_t size() const
	{
		return m_size;
	}

private:
	T* m_data;
	size_t m_size;

};

template <typename T>
class RaiiWrapper
{
public:
	typedef std::function<void(T&)> DeinitFunc;

public:
	RaiiWrapper(std::reference_wrapper<T> object, DeinitFunc deinitializeFunc) : m_objectRef(object), m_deinitializationFunc(deinitializeFunc) {}
	RaiiWrapper(RaiiWrapper& other) = delete;
	RaiiWrapper(const RaiiWrapper& other) = delete;
	
	~RaiiWrapper()
	{
		m_deinitializationFunc(m_objectRef);
	}
	
	operator T& ()
	{
		return m_objectRef.get();
	}

private:
	std::reference_wrapper<T> m_objectRef;
	DeinitFunc m_deinitializationFunc;
};

class IFileSystemListener
{
public:
	typedef std::string namestring;
	typedef std::function<void(const namestring& filename)> callbackFunc;

	virtual void SetFolderCallback(const namestring& folderName, callbackFunc callback) = 0;
	virtual void ResetFolderCallback(const namestring& folderName) = 0;

public:
	static IFileSystemListener* GetInstance();

private:
	static IFileSystemListener* s_instance;
};

}