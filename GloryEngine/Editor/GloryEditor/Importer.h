#pragma once
#include "GloryEditor.h"
#include "ImportedResource.h"

#include <filesystem>
#include <vector>

namespace Glory
{
	class Resource;
}

namespace Glory::Editor
{
	class EditableResource;

	class Importer
	{
	public:
		/** @brief Try to import a file at a path
		 * @param path Path to the file to import
		 * @param userData User data that the importer can use
		 *
		 * This first searches for an importer that is suited for importing the file
		 * based on its file extension. Then calls the @ref Load() function of that importer.
		 * If not importer is found an invalid @ref ImportedResource is returned.
		 */
		GLORY_EDITOR_API static ImportedResource Import(const std::filesystem::path& path, void* userData=nullptr);
		GLORY_EDITOR_API static Importer* GetImporter(const std::filesystem::path& path);
		GLORY_EDITOR_API static bool Export(const std::filesystem::path& path, Resource* pResource);
		GLORY_EDITOR_API static void Register(Importer* pImporter);
		GLORY_EDITOR_API static EditableResource* CreateEditableResource(const std::filesystem::path& path);

		template<typename T>
		static void Register()
		{
			RegisterOwned(new T());
		}

		virtual std::string_view Name() const = 0;

	protected:
		Importer() {};
		virtual ~Importer() {};

		virtual ImportedResource Load(const std::filesystem::path& path, void* userData) const = 0;
		virtual bool Save(const std::filesystem::path& path, Resource* pResource) const = 0;
		virtual uint32_t ResourceTypeHash() const = 0;
		virtual bool SupportsExtension(const std::filesystem::path& extension) const = 0;
		virtual EditableResource* GetEditableResource(const std::filesystem::path& path) const = 0;

		GLORY_EDITOR_API static void RegisterOwned(Importer* pImporter);

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;

	private:
		friend class MainEditor;
		static void CleanupAll();

		static std::vector<Importer*> m_pImporters;
		static std::vector<Importer*> m_pOwnedImporters;
	};
}
