#pragma once
#include "ThreadedVar.h"
#include "UUID.h"

namespace Glory
{
	class Object;

	class ObjectManager
	{
	public:
		/** @brief Find an object by ID
		 * @param id ID of the object to find
		 * @returns Object with the corresponding ID or nullptr if not found
		 */
		Object* Find(const UUID id);

	private:
		/** @brief Add an object
		 * @param pObject Object to add
		 */
		void Add(Object* pObject);
		/** @brief Remove an object
		 * @param pObject Object to remove
		 */
		void Remove(Object* pObject);

	private:
		friend class Object;
		ThreadedVector<Object*> m_pAllObjects;
	};
}
