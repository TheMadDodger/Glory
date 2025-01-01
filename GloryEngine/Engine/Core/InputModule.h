#pragma once
#include "Module.h"
#include "PlayerInput.h"

#include <glm/glm.hpp>

namespace Glory
{
	class BinaryStream;

	class InputModule : public Module
	{
	public:
		InputModule();
		virtual ~InputModule();

		virtual const std::type_info& GetModuleType() override;

		bool OnInput(InputEvent& event);
		void OnCursor(CursorEvent& event);

		size_t AddPlayer();
		void RemovePlayer(size_t playerIndex);

		void ReadInputData(YAML::Node& node);

		void ClearInputData();

		void SetPlayerInputMode(const size_t playerIndex, const std::string& inputMode);

		const UUID GetDeviceUUID(const InputDeviceType deviceType, const size_t deviceID) const;
		InputDevice* GetInputDevice(const UUID deviceID);

		InputMode* GetInputMode(const std::string& name);

		bool& InputBlocked();

		PlayerInput* GetPlayer(size_t playIndex);
		const PlayerInput* GetPlayer(size_t playIndex) const;
		float GetAxis(size_t playerIndex, const std::string& inputMap, const std::string& actionName) const;
		float GetAxisDelta(size_t playerIndex, const std::string& inputMap, const std::string& actionName) const;
		bool GetBool(size_t playerIndex, const std::string& inputMap, const std::string& actionName) const;
		glm::vec2 GetCursorPos(size_t playerIndex) const;

		void FreeDevice(const UUID deviceId);

		const UUID FindAvailableInputDevice(const InputDeviceType deviceType) const;

		GLORY_API void SetCursorBounds(const glm::vec4& bounds);
		GLORY_API const glm::vec4& GetCursorBounds();

	protected:
		virtual void OnInitialize() {};
		virtual void OnCleanup() {};
		virtual void OnUpdate() {};

		virtual void OnProcessData() override;

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Cleanup() override;
		virtual void Update() override;
		virtual void OnBeginFrame() override;

		void ReadInputModes(YAML::Node& node);
		void ReadInputMaps(YAML::Node& node);

	private:
		std::map<UUID, InputDevice> m_InputDevices;
		UUID m_DefaultMouseDeviceIndex;
		UUID m_DefaultKeyboardDeviceIndex;

		std::vector<PlayerInput> m_Players;
		std::map<std::string, InputMode> m_InputModes;
		/* First map is by InputMap name, second by InputMode name */
		std::map<std::string, std::map<std::string, InputMap>> m_InputMaps;

		bool m_InputBlocked;
		glm::vec4 m_CursorBounds;
	};
}
