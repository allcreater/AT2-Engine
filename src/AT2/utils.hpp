#include <functional>

namespace AT2
{
namespace Utils
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

//I'm afraid, its need to be removed
template <typename C, typename T>
class ControlledList
{
public:
	typedef std::function<void(size_t, const T&)> ControlCallback;

public:
	ControlledList() = default;
	ControlledList(const ControlledList& other) = delete;
	const ControlledList& operator= (const ControlledList& other) = delete;

	ControlledList(C& collection, ControlCallback setCallback, ControlCallback getCallback, ControlCallback removeCallback)
		: m_collection(collection),
		m_setCallback(setCallback),
		m_getCallback(getCallback),
		m_removeCallback(removeCallback)
	{
	}

	T& operator [] (size_t index)
	{
		T& element = m_collection[index];

		m_setCallback(index, element);
		return element;
	}
	const T& operator [] (size_t index) const
	{
		T& element = m_collection[index];

		m_getCallback(index, element);
		return element;
	}

	void Remove(size_t index)
	{
		m_removeCallback(index, m_collection[index]);

		m_collection[index] = default(T);
	}

private:
	C& m_collection;
	ControlCallback m_setCallback, m_getCallback, m_removeCallback;
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
}