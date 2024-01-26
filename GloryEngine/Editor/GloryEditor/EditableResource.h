#pragma once
#include "GloryEditor.h"

#include <Resource.h>
#include <NodeRef.h>

namespace Glory::Editor
{
	/** @brief The editor version of an asset */
	class EditableResource: public Resource
	{
	public:
		/** @brief Constructor */
		GLORY_EDITOR_API EditableResource();

		/** @brief Save this asset and reset its dirty flag */
		GLORY_EDITOR_API void Save();
		/** @brief Overidable method for handling saving */
		GLORY_EDITOR_API virtual void OnSave() = 0;

		/** @brief Set the dirty flag of this asset
		 * @param dirty Whether the asset is dirty or not
		 */
		GLORY_EDITOR_API void SetDirty(bool dirty);
		/** @brief Check if the asset needs saving */
		GLORY_EDITOR_API bool IsDirty();

		/** @brief Overidable method for handling reloading of the asset */
		virtual void Reload(const std::filesystem::path&) {}

	private:
		bool m_Dirty{ false };
	};

	/**
	 * @brief An asset that cannot be edited
	 *
	 * For exampple a model file cannot be edited in the editor.
	 */
	template<class T>
	class NonEditableResource : public EditableResource
	{
	public:
		/** @brief Constructor */
		NonEditableResource()
		{
			APPEND_TYPE(NonEditableResource<T>);
		}

		/** @brief Save does nothing for this asset */
		virtual void OnSave() override {};
	};

	/** @brief A YAML based editor asset */
	template<class T>
	class YAMLResource: public EditableResource
	{
	public:
		/** @brief Constructor
		 * @param path Absolute path to the YAML file
		 */
		YAMLResource(const std::filesystem::path& path): m_File(path)
		{
			APPEND_TYPE(YAMLResource<T>);
			m_File.Load();
		}

		/** @brief Get the undelying YAML file fo editing */
		Utils::YAMLFileRef& operator*()
		{
			return m_File;
		}

		/** @brief Save changes made to the YAML file */
		virtual void OnSave() override
		{
			m_File.Save();
		}

		/** @brief Reload the YAML file at the new path
		 * @param newPath New path of the YAML file
		 *
		 * This discards any unsaved changes
		 */
		virtual void Reload(const std::filesystem::path& newPath) override
		{
			m_File.ChangePath(newPath);
			m_File.Load();
		}

	private:
		Utils::YAMLFileRef m_File;
	};
}
