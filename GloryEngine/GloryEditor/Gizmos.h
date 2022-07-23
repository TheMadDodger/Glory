#pragma once
#include <glm/glm.hpp>
#include <vector>

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
		Gizmos();
		virtual ~Gizmos();

		static bool DrawGizmo(glm::mat4* transfrom);

		static ImGuizmo::OPERATION m_DefaultOperation;
		static ImGuizmo::MODE m_DefaultMode;

	private:
		static void DrawGizmos(const glm::mat4& cameraView, const glm::mat4& cameraProjection);
		static void Clear();

	private:
		friend class MainEditor;
		friend class SceneWindow;
		std::vector<IGizmo*> m_pGizmos;
		std::vector<bool> m_ManipulatedGizmos;
	};

	class IGizmo
	{
	public:
		virtual bool OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection) = 0;

	protected:
		IGizmo();
		virtual ~IGizmo();

	private:
		friend class Gizmos;
	};

	class DefaultGizmo : public IGizmo
	{
	public:
		DefaultGizmo(glm::mat4* pTransform);
		virtual ~DefaultGizmo();

	private:
		virtual bool OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection) override;

	private:
		glm::mat4* m_pTransform;
	};
}
