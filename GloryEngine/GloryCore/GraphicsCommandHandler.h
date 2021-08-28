#pragma once
#include <typeinfo>
#include "GraphicsCommands.h"
#include <any>

namespace Glory
{
	class BaseGraphicsCommandHandler
	{
	public:
		BaseGraphicsCommandHandler();
		virtual ~BaseGraphicsCommandHandler();

	protected:
		virtual const std::type_info& GetCommandType();
		virtual void Invoke(const std::any& commandData);

	private:
		friend class GraphicsCommands;
	};

	template<typename T>
	class GraphicsCommandHandler : public BaseGraphicsCommandHandler
	{
	public:
		GraphicsCommandHandler() {}
		virtual ~GraphicsCommandHandler() {}

	protected:
		virtual void OnInvoke(T commandData) = 0;

	private:
		virtual const std::type_info& GetCommandType() override
		{
			return typeid(T);
		}

		virtual void Invoke(const std::any& commandData) override
		{
			T data = std::any_cast<T>(commandData);
			OnInvoke(data);
		}
	};
}