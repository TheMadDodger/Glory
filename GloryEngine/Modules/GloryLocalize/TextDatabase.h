#pragma once
#include <Resource.h>

namespace Glory
{
	class TextDatabase : public Resource
	{
	public:
		/** @brief Constructor */
		GLORY_API TextDatabase();
		/** @brief Destructor */
		GLORY_API virtual ~TextDatabase();

	private:
		/** @brief Get a vector containing other resources referenced by this resource */
		virtual void References(Engine*, std::vector<UUID>&) const override;

		virtual void Serialize(BinaryStream& container) const override;
		virtual void Deserialize(BinaryStream& container) override;

	private:
	};
}
