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
		static void DrawGizmo(glm::mat4* transfrom);

		static ImGuizmo::OPERATION m_DefaultOperation;
		static ImGuizmo::MODE m_DefaultMode;

	private:
		Gizmos();
		virtual ~Gizmos();

		static void DrawGizmos(const glm::mat4& cameraView, const glm::mat4& cameraProjection);
		static void Clear();

	private:
		friend class MainEditor;
		friend class SceneWindow;
		static std::vector<IGizmo*> m_pGizmos;

	};

	class IGizmo
	{
	public:
		virtual void OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection) = 0;

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
		virtual void OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection) override;

	private:
		glm::mat4* m_pTransform;
	};
}
