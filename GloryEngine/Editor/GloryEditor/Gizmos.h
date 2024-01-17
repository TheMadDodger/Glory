#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <UUID.h>
#include <functional>
#include "GloryEditor.h"
#include "Toolbar.h"

namespace ImGuizmo
{
	enum OPERATION;
	enum MODE;
}

namespace Glory::Editor
{
	class IGizmo;
	class GridToolchain;
	class GizmosToolChain;

	class Gizmos
	{
	public:
		static const char* Shortcut_Gizmos_Translate;
		static const char* Shortcut_Gizmos_Rotate;
		static const char* Shortcut_Gizmos_Scale;
		static const char* Shortcut_Gizmos_Universal;
		static const char* Shortcut_Gizmos_Local;
		static const char* Shortcut_Gizmos_World;

	public:
		//static GLORY_EDITOR_API bool DrawGizmo(glm::mat4* transfrom);
		template<class T, typename ...Args>
		static T* GetGizmo(UUID uuid, Args&&... args)
		{
			T* pGizmo = (T*)FindGizmo(uuid);
			if (pGizmo != nullptr) return pGizmo;
			pGizmo = new T(args...);
			AddGizmo(uuid, pGizmo);
			return pGizmo;
		}

		static GLORY_EDITOR_API void FreeGizmo(UUID uuid);
		static GLORY_EDITOR_API IGizmo* FindGizmo(UUID uuid);

		static ImGuizmo::OPERATION m_DefaultOperation;
		static ImGuizmo::MODE m_DefaultMode;

		static void ToggleMode();

		static void Initialize();
		static void Cleanup();

		static const float* GetSnap();

	private:
		GLORY_EDITOR_API Gizmos();
		virtual GLORY_EDITOR_API ~Gizmos();

		static void DrawGizmos(const glm::mat4& cameraView, const glm::mat4& cameraProjection);
		static void Clear();

		static GLORY_EDITOR_API void AddGizmo(UUID uuid, IGizmo* pGizmo);

	private:
		friend class MainEditor;
		friend class SceneWindow;
		static std::map<UUID, IGizmo*> m_pGizmos;
		static GizmosToolChain* m_pToolChain;
		static GridToolchain* m_pGridToolChain;
	};

	class IGizmo
	{
	public:
		virtual void OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection) = 0;

		virtual bool WasManipulated(glm::mat4& oldTransform, glm::mat4& newTransform) = 0;
		virtual void ManualManipulate(const glm::mat4& newTransform) = 0;
		virtual void UpdateTransform(const glm::mat4& newTransform) = 0;
		virtual bool IsManipulating() = 0;

	protected:
		GLORY_EDITOR_API IGizmo();
		virtual GLORY_EDITOR_API ~IGizmo();

	private:
		friend class Gizmos;
	};

	class DefaultGizmo : public IGizmo
	{
	public:
		GLORY_EDITOR_API DefaultGizmo(glm::mat4 pTransform);
		virtual GLORY_EDITOR_API ~DefaultGizmo();

		virtual bool WasManipulated(glm::mat4& oldTransform, glm::mat4& newTransform) override;
		virtual void ManualManipulate(const glm::mat4& newTransform) override;
		virtual void UpdateTransform(const glm::mat4& newTransform) override;
		virtual bool IsManipulating() override;

		std::function<void(const glm::mat4&)> OnManualManipulate;

	private:
		virtual void OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection) override;

	private:
		glm::mat4 m_Transform;
		glm::mat4 m_OldTransform;
		bool m_IsManipulating;
		bool m_WasManipulated;
	};
}
