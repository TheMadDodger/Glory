#pragma once
#include <queue>
#include "RenderData.h"
#include "CameraRef.h"
#include "LightData.h"

#include "Debug.h"

namespace Glory
{
	template<typename T>
	struct FrameData
	{
	public:
		FrameData(size_t max) : m_Counter(0), m_ActiveObjects(max) {}
		~FrameData()
		{
			m_ActiveObjects.clear();
			m_Counter = 0;
		}

		void push_back(const T& value)
		{
			if (m_Counter >= m_ActiveObjects.size())
			{
				//const std::string typeName = typeid(T).name();
				//m_pEngine->GetDebug().LogWarning(typeName + " overflow in render data!");
				return;
			}
			m_ActiveObjects[m_Counter] = std::move(value);
			++m_Counter;
		}

		size_t count() const
		{
			return m_Counter;
		}

		size_t size() const
		{
			return m_ActiveObjects.size();
		}

		const T& operator[](size_t index) const
		{
			return m_ActiveObjects[index];
		}

		T& operator[](size_t index)
		{
			return m_ActiveObjects[index];
		}

		const void* data() const
		{
			return m_ActiveObjects.data();
		}

		void reset()
		{
			m_Counter = 0;
		}

	private:
		std::vector<T> m_ActiveObjects;
		size_t m_Counter;
	};

	struct RenderFrame
	{
	public:
		RenderFrame(size_t maxLigts);
		~RenderFrame();

		void Reset();

	public:
		std::vector<std::pair<glm::ivec2, UUID>> Picking;
		FrameData<LightData> ActiveLights;
		FrameData<glm::mat4> LightSpaceTransforms;
		std::vector<UUID> ActiveLightIDs;
	};

	enum RenderPassType : size_t
	{
		RP_Prepass,
		RP_CameraPrepass,
		RP_ObjectPass,
		RP_LateobjectPass,
		RP_CameraPostpass,
		RP_PreCompositePass,
		RP_CameraCompositePass,
		RP_PostCompositePass,
		RP_Postpass,
		RP_Postblit,
		RP_Count
	};

	class RendererModule;
	struct RenderPass
	{
		std::string m_Name;
		std::function<void(uint32_t, RendererModule*)> m_Callback;
	};
}
