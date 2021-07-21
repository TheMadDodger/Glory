#pragma once
#include <typeinfo>
#include "GraphicsCommands.h"

namespace Glory
{
	class BaseGraphicsCommandHandler
	{
	public:
		BaseGraphicsCommandHandler();
		~BaseGraphicsCommandHandler();

	protected:
		virtual const std::type_info& GetCommandType();
		virtual void Invoke(const BaseGraphicsCommand* commandData);

	private:
		friend class GraphicsCommands;
	};

	template<typename T>
	class GraphicsCommandHandler
	{
	public:
		GraphicsCommandHandler() {}
		~GraphicsCommandHandler() {}

	protected:
		virtual void OnInvoke(T& commandData) = 0;

	private:
		virtual const std::type_info& GetCommandType() override
		{
			return typeid(T);
		}

		virtual void Invoke(const BaseGraphicsCommand* commandData) override
		{
			OnInvoke((T&)commandData);
		}
	};
}