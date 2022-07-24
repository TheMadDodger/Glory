#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "GloryEditor.h"

namespace ImGuizmo
{
	enum OPERATION;
	enum MODE;
}

namespace Glory::Editor
{
	class IGizmo;

	class Gizmos
	{
	public:
		GLORY_EDITOR_API Gizmos();
		virtual GLORY_EDITOR_API ~Gizmos();

		static GLORY_EDITOR_API bool DrawGizmo(glm::mat4* transfrom);

		static ImGuizmo::OPERATION m_DefaultOperation;
		static ImGuizmo::MODE m_DefaultMode;

	private:
		static void DrawGizmos(const glm::mat4& cameraView, const glm::mat4& cameraProjection);
		static void Clear();

	private:
		friend class MainEditor;
		friend class SceneWindow;
		static std::vector<IGizmo*> m_pGizmos;
		static std::vector<bool> m_ManipulatedGizmos;
	};

	class IGizmo
	{
	public:
		virtual bool OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection) = 0;

	protected:
		GLORY_EDITOR_API IGizmo();
		virtual GLORY_EDITOR_API ~IGizmo();

	private:
		friend class Gizmos;
	};

	class DefaultGizmo : public IGizmo
	{
	public:
		GLORY_EDITOR_API DefaultGizmo(glm::mat4* pTransform);
		virtual GLORY_EDITOR_API ~DefaultGizmo();

	private:
		virtual GLORY_EDITOR_API bool OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection) override;

	private:
		glm::mat4* m_pTransform;
	};
}
