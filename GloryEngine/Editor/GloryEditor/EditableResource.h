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

		/** @brief Is this resourc editable */
		virtual bool IsEditable() const { return true; }

	protected:
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

		/** @brief Not editable */
		virtual bool IsEditable() const override { return false; }
	};

	/** @brief A YAML based editor asset */
	class YAMLResourceBase : public EditableResource
	{
	public:
		/** @brief Constructor for creating new files */
		YAMLResourceBase() : m_File()
		{
			APPEND_TYPE(YAMLResourceBase);
		}

		/** @brief Constructor
		 * @param path Absolute path to the YAML file
		 */
		YAMLResourceBase(const std::filesystem::path& path): m_File(path)
		{
			APPEND_TYPE(YAMLResourceBase);
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
			m_Dirty = false;
		}

		/** @brief Change the path of the underlying YAML file
		 * @param path The new absolute path to the YAML file
		 */
		void SetPath(const std::filesystem::path& path)
		{
			m_File.ChangePath(path);
		}

	protected:
		Utils::YAMLFileRef m_File;
	};

	/** @brief A YAML based editor asset for a specific type */
	template<class T>
	class YAMLResource: public YAMLResourceBase
	{
	public:
		/** @brief Constructor for creating new files */
		YAMLResource() : YAMLResourceBase()
		{
			APPEND_TYPE(YAMLResource<T>);
		}

		/** @brief Constructor
		 * @param path Absolute path to the YAML file
		 */
		YAMLResource(const std::filesystem::path& path): YAMLResourceBase(path)
		{
			APPEND_TYPE(YAMLResource<T>);
		}
	};
}
