////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Function.h
/// \author bjh1371
/// \date 2015/07/22
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CFunction
/// \brief bind ��� lamda�� ����ϱ� ���� ���� Ŭ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename ...Args>
class CFunction
{
private:
	std::function<void(Args...)> m_Function; ///< ���� ������ �Լ�


public:
	/// \brief ������
	CFunction(const std::function<void(Args...)>& func)
	:
	m_Function(func)
	{

	}
	
	/// \brief �Ҹ���
	~CFunction() 
	{

	}
	
	
public:
	/// \brief �Լ� ����
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
