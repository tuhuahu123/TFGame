#pragma once

template <typename T>
class Manager   // 单例模式
{
public:
	static T* instance()
	{
		// 单例模式
		if (!manager)
		{
			manager = new T();
		}
		return manager;	
	}
	
protected:
	static T* manager;				  //单例模式
	Manager() = default;			  // 禁止构造
	~Manager() = default;			  // 禁止析构
	Manager(const Manager&) = delete; // 禁止拷贝构造
	Manager& operator=(const Manager&) = delete; // 禁止赋值
};

template <typename T>
T* Manager<T>::manager = nullptr; // 单例模式



