#include "LocalizeManagers.h"
#include "LocalizeModule.h"

#include <Debug.h>

#include <EntityRegistry.h>
#include <Components.h>

namespace Glory
{
	StringTableLoaderManager::StringTableLoaderManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity), m_pModule(nullptr)
	{
	}

	StringTableLoaderManager::~StringTableLoaderManager()
	{
	}

	void StringTableLoaderManager::OnValidateImpl(Utils::ECS::EntityID entity, StringTableLoader& pComponent)
	{
		m_pModule->LoadStringTable(pComponent.m_StringTable.GetUUID());
	}

	void StringTableLoaderManager::OnStopImpl(Utils::ECS::EntityID entity, StringTableLoader& pComponent)
	{
		if (pComponent.m_KeepLoaded) return;
		m_pModule->UnloadStringTable(pComponent.m_StringTable.GetUUID());
	}

	void StringTableLoaderManager::GetReferencesImpl(std::vector<UUID>& references) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const StringTableLoader& localizeComponent = GetAt(i);
			const UUID id = localizeComponent.m_StringTable.GetUUID();
			if (!id) continue;
			references.push_back(id);
		}
	}

	void StringTableLoaderManager::OnDeserialize(Utils::BinaryStream&)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const StringTableLoader& loader = GetAt(i);
			loader.m_StringTable.ManualRegisterReference();
		}
	}

	void StringTableLoaderManager::OnCopy(StringTableLoader& loader)
	{
		loader.m_StringTable.ManualRegisterReference();
	}

	void StringTableLoaderManager::OnInitialize()
	{
		Bind(DoValidate, &StringTableLoaderManager::OnValidateImpl);
		Bind(DoStop, &StringTableLoaderManager::OnStopImpl);
		Bind(DoGetReferences, &StringTableLoaderManager::GetReferencesImpl);
	}

	LocalizeManager::LocalizeManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity), m_pModule(nullptr), m_pDebug(nullptr)
	{
	}

	LocalizeManager::~LocalizeManager()
	{
	}

	void LocalizeManager::OnValidateImpl(Utils::ECS::EntityID entity, Localize& pComponent)
	{
		if (m_pRegistry->HasComponent<TextComponent>(entity)) return;
		m_pDebug->LogError("Localize component requires a TextComponent on the entity");
	}

	void LocalizeManager::OnStartImpl(Utils::ECS::EntityID entity, Localize& pComponent)
	{
		if (!m_pRegistry->HasComponent<TextComponent>(entity)) return;
		TextComponent& text = m_pRegistry->GetComponent<TextComponent>(entity);
		const std::string_view fullTerm = pComponent.m_Term;
		const size_t firstDot = fullTerm.find('.');
		if (firstDot == std::string::npos) return;
		const std::string_view tableName = fullTerm.substr(0, firstDot);
		const std::string_view term = fullTerm.substr(firstDot + 1);
		text.m_Dirty |= m_pModule->FindString(tableName, term, text.m_Text);
	}

	void LocalizeManager::UnpackDataInto(const void* data, Localize& newComponent)
	{
		const Localize* pLocalizeSource = reinterpret_cast<const Localize*>(data);
		newComponent.m_Term = pLocalizeSource->m_Term;
	}

	void LocalizeManager::SerializeDense(Utils::BinaryStream& stream) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const Localize& localize = GetAt(i);
			stream.Write(localize.m_Term);
		}
	}

	void LocalizeManager::DeserializeDense(Utils::BinaryStream& stream)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			Localize& localize = GetAt(i);
			stream.Read(localize.m_Term);
		}
	}

	void LocalizeManager::OnInitialize()
	{
		Bind(DoValidate, &LocalizeManager::OnValidateImpl);
		Bind(DoStart, &LocalizeManager::OnStartImpl);
	}
}
