#pragma once
#include "ThreadedVar.h"
#include "UUID.h"
#include "Object.h"

namespace Glory
{
	class ObjectManager
	{
	public:
		/** @brief Find an object by ID
		 * @param id ID of the object to find
		 * @returns Object with the corresponding ID or nullptr if not found
		 */
		template<typename T>
		T* Find(const UUID id)
		{
			Object* pObject = Find(id);
			return pObject ? (T*)pObject : nullptr;
		}

		/** @overload */
		Object* Find(const UUID id);

		/** @brief Create an object and add it to the manager */
		template<class O, typename... Args>
		O* Create(Args&&... args)
		{
			O* newObject = new O(args...);
			m_pAllObjects.push_back(newObject);
			return newObject;
		}

		/** @brief Destroy an object and remove it from the manager */
		template<class O>
		void Destroy(O* pObject)
		{
			if (!pObject) return;
			m_pAllObjects.Erase(pObject);
			delete pObject;
		}

		/** @brief Destructor */
		~ObjectManager();

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
