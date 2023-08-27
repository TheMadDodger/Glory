#pragma once
#include "EditorWindow.h"

#include <UUID.h>

namespace Glory::Editor
{
	class ResourcesWindow : public EditorWindowTemplate<ResourcesWindow>
	{
    public:
        ResourcesWindow();
        virtual ~ResourcesWindow();

    private:
        virtual void OnGUI() override;
        virtual void OnOpen() override;

        void RunFilter();

    private:
        std::vector<UUID> m_SearchResultCache;
        std::vector<size_t> m_SearchResultIndexCache;
        static bool m_ForceFilter;
        static bool m_SubscribedToEvents;
	};
}
