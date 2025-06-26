#pragma once
#include "UIComponents.h"

#include <Glory.h>

#include <functional>

#include <glm/fwd.hpp>

#define ADD_CONSTRAINT_FUNC(name) AddConstraint(STRINGIFY(name), name)

namespace Glory
{
	struct Constraint
	{
		uint32_t m_Constraint;
		float m_Value;
		float m_FinalValue;
	};

	class Constraints
	{
	public:
		/** @brief Add a constraint
		 * @param name Name of the constraint
		 * @param callback Callback handler of the constraint
		 */
		GLORY_API static void AddConstraint(std::string&& name, std::function<void(Constraint&, ConstraintAxis,
			const glm::vec2&, const glm::vec2&, const glm::vec2&)> callback);

		/** @brief Get a vector with all constraint names */
		GLORY_API static const std::vector<std::string_view>& Names();
		/** @brief Get the index of a constraint type
		 * @parm type The type hash of the constraint
		 */
		GLORY_API static size_t IndexOf(uint32_t type);
		/** @overload
		 * @parm name Name of the constraint
		 */
		GLORY_API static size_t IndexOf(std::string_view name);
		/** @brief Get the type of a constraint
		 * @parm index Index of the constraint
		 */
		GLORY_API static uint32_t Type(size_t index);

		/** @brief Process a constraint
		 * @parm constraint Constraint to process
		 * @parm self The container size of self
		 * @parm parent The container size of the parent
		 */
		template<typename T>
		static void ProcessConstraint(T& constraint, const glm::vec2& self, const glm::vec2& parent, const glm::vec2& screen)
		{
			if (!constraint.m_Constraint)
			{
				constraint.m_FinalValue = constraint.m_Value;
				return;
			}

			const size_t handler = IndexOf(constraint.m_Constraint);
			if (handler == 0)
			{
				constraint.m_FinalValue = constraint.m_Value;
				return;
			}
			ProcessConstraint(T::Axis, static_cast<void*>(&constraint), handler, self, parent, screen);
		}

		/** @brief Register built-in constraints */
		GLORY_API static void AddBuiltinConstraints();

	private:
		/** @brief Process a constraint
		 * @parm axis Axis to process the constraint on
		 * @parm constraint The constraint
		 * @parm handler Index of the constraint handler
		 * @parm self The container size of self
		 * @parm parent The container size of the parent
		 */
		GLORY_API static void ProcessConstraint(ConstraintAxis axis, void* constraint, size_t handler,
			const glm::vec2& self, const glm::vec2& parent, const glm::vec2& screen);

	private:
		static std::vector<std::string> m_Names;
		static std::vector<std::string_view> m_NamesView;
		static std::vector<uint32_t> m_Hashes;
		static std::vector<std::function<void(Constraint&, ConstraintAxis,
			const glm::vec2&, const glm::vec2&, const glm::vec2&)>> m_Callbacks;
	};
}
