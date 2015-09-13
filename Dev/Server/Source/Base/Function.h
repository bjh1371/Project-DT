////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Function.h
/// \author bjh1371
/// \date 2015/07/22
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CFunction
/// \brief bind 대신 lamda를 사용하기 위해 만든 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename ...Args>
class CFunction
{
private:
	std::function<void(Args...)> m_Function; ///< 실제 실행할 함수


public:
	/// \brief 생성자
	CFunction(const std::function<void(Args...)>& func)
	:
	m_Function(func)
	{

	}
	
	/// \brief 소멸자
	~CFunction() 
	{

	}
	
	
public:
	/// \brief 함수 실행
	template <typename...Arguments>
	void operator()(Arguments&&...args) const
	{
		m_Function(std::forward<Arguments>(args)...);
	}
};

namespace FuncUtil
{
	template <typename...Args>
	CFunction<Args...> Bind(void(*func)(Args...))
	{
		return CFunction<Args...>([func](Args&&...args){ func(std::forward<Args>(args)...); });
	}

	template <typename Type, typename...Args>
	CFunction<Args...> Bind(Type* type, void(Type::*memfunc)(Args...))
	{
		return CFunction<Args...>([type, memfunc](Args&&...args) {  (type->*memfunc)(std::forward<Args>(args)...); });
	}
}
