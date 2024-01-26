#pragma once
#include <vector>
#include <map>
#include <functional>

#include <UUID.h>

namespace Glory
{
	class Engine;
	class Resource;
	struct UUID;

namespace Editor
{
	class EditableResource;

	/**
	 * @brief Resource manager for the editor
	 */
	class EditorResourceManager
	{
	public:
		/** @brief Constructor */
		EditorResourceManager(Engine* pEngine);
		/** @brief Destructor */
		~EditorResourceManager();

		//void GetAsset(const UUID uuid, std::function<void(Resource*)> callback);
		//void RunCallbacks();

		/** @brief Get the editable resource for an existing asset
		 * @param uuid ID of the asset
		 *
		 * This allows the editor to load the editor version of a resource
		 * and allow it to be edited in the inspector without having to load
		 * the generated asset file into the editor.
		 *
		 * Have a look at @ref EditableResource
		 */
		EditableResource* GetEditableResource(const UUID uuid);
		/** @brief Save all dirty editor versions of assets */
		void Save();
		/** @brief Reload the contents of an asset and update its path if applicable
		 * @param uuid ID of the asset
		 *
		 * Should be used if the asset was moved.
		 */
		void ReloadEditableAsset(const UUID uuid);

	private:
		std::map<UUID, uint32_t> m_Hashes;
		std::map<UUID, std::function<void(Resource*)>> m_Callbacks;

		Engine* m_pEngine;

		std::map<UUID, EditableResource*> m_pEditableResources;
	};
}
}
