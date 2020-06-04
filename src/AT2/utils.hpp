#include <functional>

namespace AT2::Utils
{

//as advised at https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
template <typename T>
bool is_uninitialized(std::weak_ptr<T> const& weak) {
	using wt = std::weak_ptr<T>;
	return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
}


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