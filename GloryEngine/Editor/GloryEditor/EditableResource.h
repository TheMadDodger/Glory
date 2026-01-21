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
		GLORY_EDITOR_API virtual void SetDirty(bool dirty);
		/** @brief Check if the asset needs saving */
		GLORY_EDITOR_API bool IsDirty();

		/** @brief Overidable method for handling reloading of the asset */
		virtual void Reload(const std::filesystem::path&) {}

		/** @brief Is this resource editable */
		virtual bool IsEditable() const { return true; }

		/** @brief True if the resource was recently saved */
		bool& WasSaved() { return m_WasSaved; }

	private:
		virtual void References(Engine*, std::vector<UUID>&) const override {}

	protected:
		bool m_Dirty{ false };
		bool m_WasSaved{ false };
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
		YAMLResourceBase()
		{
			APPEND_TYPE(YAMLResourceBase);
		}

		/** @brief Get the root YAML node for editing */
		virtual Utils::NodeValueRef operator*() = 0;

		/** @brief Get the YAML file this resource points to */
		virtual Utils::YAMLFileRef& File() = 0;

		/** @brief Change the path of the underlying YAML file
		 * @param path The new absolute path to the YAML file
		 */
		virtual void SetPath(const std::filesystem::path& path) = 0;

		/** @brief Path of the underlying YAML file */
		virtual const std::filesystem::path& Path() const = 0;

		virtual bool IsSectionedResource() const = 0;
	};

	/** @brief A YAML based editor asset for a specific type */
	template<class T>
	class YAMLResource : public YAMLResourceBase
	{
	public:
		/** @brief Constructor for creating new files */
		YAMLResource() : YAMLResourceBase()
		{
			APPEND_TYPE(YAMLResource<T>);
		}
	};

	/** @brief A YAML based editor asset for a complete YAML file */
	template<class T>
	class FullYAMLResource final : public YAMLResource<T>
	{
	public:
		/** @brief Constructor for creating new files */
		FullYAMLResource() : YAMLResource<T>()
		{
			APPEND_TYPE(FullYAMLResource<T>);
		}

		/** @brief Constructor
		 * @param path Absolute path to the YAML file
		 */
		FullYAMLResource(const std::filesystem::path& path): YAMLResource<T>(), m_File(path)
		{
			APPEND_TYPE(FullYAMLResource<T>);
			m_File.Load();
		}

		/** @brief Get the root YAML node for editing */
		virtual Utils::NodeValueRef operator*() override
		{
			return Utils::NodeValueRef(m_File);
		}

		/** @brief Get the YAML file this resource points to */
		virtual Utils::YAMLFileRef& File() override
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
		virtual void SetPath(const std::filesystem::path& path)
		{
			m_File.ChangePath(path);
		}

		/** @brief Path of the underlying YAML file */
		virtual const std::filesystem::path& Path() const
		{
			return m_File.Path();
		}

		virtual bool IsSectionedResource() const override
		{
			return false;
		}

	private:
		Utils::YAMLFileRef m_File;
	};

	/** @brief A YAML based editor asset for a section of a YAML file */
	template<class T, class S>
	class YAMLResourceSection final : public YAMLResource<T>
	{
	public:
		/** @brief Constructor for creating new files */
		YAMLResourceSection(YAMLResource<S>* pFullResource, const std::filesystem::path& sectionPath) :
			YAMLResource<T>(), m_pFullResource(pFullResource), m_SectionPath(sectionPath)
		{
			PushInheritence<YAMLResourceSection<T, S>>();
		}

		/** @brief Get the root YAML node for editing */
		virtual Utils::NodeValueRef operator*() override
		{
			Utils::NodeValueRef root = Utils::NodeValueRef(m_pFullResource->File());
			Utils::NodeValueRef section = root[m_SectionPath];
			return section;
		}

		/** @brief Get the YAML file this resource points to */
		virtual Utils::YAMLFileRef& File() override
		{
			return m_pFullResource->File();
		}

		/** @brief Save changes made to the YAML file */
		virtual void OnSave() override
		{
			m_pFullResource->Save();
		}

		/** @brief Reload the YAML file at the new path
		 * @param newPath New path of the YAML file
		 *
		 * This discards any unsaved changes
		 */
		virtual void Reload(const std::filesystem::path& newPath) override
		{
			/* Not allowed! */
			throw std::exception("Reloading a sectioned resource is not allowed!");
		}

		/** @brief Change the path of the underlying YAML file
		 * @param path The new absolute path to the YAML file
		 */
		virtual void SetPath(const std::filesystem::path& path)
		{
			/* Not allowed! */
			throw std::exception("Changing path of a sectioned resource is not allowed!");
		}

		virtual void SetDirty(bool dirty) override
		{
			EditableResource::SetDirty(dirty);

			if (!dirty) return;
			m_pFullResource->SetDirty(dirty);
		}

		/** @brief Path of the underlying YAML file */
		virtual const std::filesystem::path& Path() const
		{
			return m_pFullResource->Path();
		}

		virtual bool IsSectionedResource() const override
		{
			return true;
		}

	private:
		YAMLResource<S>* m_pFullResource;
		std::filesystem::path m_SectionPath;
	};
}
