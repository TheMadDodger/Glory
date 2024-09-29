#include "JoltDebugRenderer.h"
#ifdef JPH_DEBUG_RENDERER
#include "Helpers.h"

#include <Engine.h>
#include <RendererModule.h>

namespace Glory
{
	JoltDebugRenderer::JoltDebugRenderer(Engine* pEngine)
		: m_pEngine(pEngine)
	{
		Initialize();
	}

	void JoltDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
	{
		m_pEngine->GetMainModule<RendererModule>()->DrawLine(glm::identity<glm::mat4>(), ToVec3(inFrom), ToVec3(inTo), ToVec3(inColor));
	}

	void JoltDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor)
	{
		/* FIXME: Temporary until there is solid primitive rendering */
		DrawLine(inV1, inV2, inColor);
		DrawLine(inV2, inV3, inColor);
		DrawLine(inV3, inV1, inColor);
	}

	JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
	{
		return JPH::DebugRenderer::Batch();
	}

	JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount)
	{
		return JPH::DebugRenderer::Batch();
	}

	void JoltDebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
	{
	}

	void JoltDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor, float inHeight)
	{
	}
}
#endif