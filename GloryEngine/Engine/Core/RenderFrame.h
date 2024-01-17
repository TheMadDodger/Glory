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

		const void* data() const
		{
			return m_ActiveObjects.data();
		}

	private:
		std::vector<T> m_ActiveObjects;
		size_t m_Counter;
	};

	struct RenderFrame
	{
	public:
		RenderFrame();

	public:
		std::vector<RenderData> ObjectsToRender;
		std::vector<CameraRef> ActiveCameras;
		FrameData<PointLight> ActiveLights;
	};
}
