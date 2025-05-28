#include "InputModule.h"
#include "Engine.h"
#include "Debug.h"
#include "BinaryStream.h"

namespace Glory
{
	const char* Key_InputMaps = "InputMaps";
	const char* Key_InputModes = "InputModes";

	InputModule::InputModule()
		: m_Players(), m_InputBlocked(true), m_CursorBounds(), m_ScreenScale(1.0f, 1.0f)
	{
	}

	InputModule::~InputModule()
	{
	}

	const std::type_info& InputModule::GetModuleType()
	{
		return typeid(InputModule);
	}

	bool InputModule::OnInput(InputEvent& event)
	{
		if (m_InputBlocked) return false;
		UUID deviceUUID = GetDeviceUUID(event.InputDeviceType, event.SourceDeviceID);
		/* Unknown device? */
		if (deviceUUID == 0) return false;
		/* Not claimed by a player? */
		InputDevice& inputDevice = m_InputDevices.at(deviceUUID);
		if (inputDevice.m_PlayerIndex == -1)
		{
			for (size_t i = 0; i < m_Players.size(); i++)
			{
				std::string_view inputMode = m_Players[i].InputMode();
				//if()
			}
			return false;
		}

		m_Players[inputDevice.m_PlayerIndex].HandleInputEvent(event);
		return true;
	}

	void InputModule::OnCursor(CursorEvent& event)
	{
		UUID deviceUUID = GetDeviceUUID(event.InputDeviceType, event.SourceDeviceID);
		/* Unknown device? */
		if (deviceUUID == 0) return;
		/* Not claimed by a player? */
		InputDevice& inputDevice = m_InputDevices.at(deviceUUID);
		if (inputDevice.m_PlayerIndex == -1)
		{
			for (size_t i = 0; i < m_Players.size(); i++)
			{
				std::string_view inputMode = m_Players[i].InputMode();
				//if()
			}
			return;
		}

		m_Players[inputDevice.m_PlayerIndex].HandleCursorEvent(event);
	}

	bool InputModule::OnText(TextEvent&)
	{
		return false;
	}

	size_t InputModule::AddPlayer()
	{
		const size_t index = m_Players.size();
		m_Players.push_back({ this, index });
		return index;
	}

	void InputModule::RemovePlayer(size_t playerIndex)
	{
		/* TODO: Give players a UUID instead of an index */
	}

	void InputModule::ReadInputData(YAML::Node& node)
	{
		ReadInputModes(node[Key_InputModes]);
		ReadInputMaps(node[Key_InputMaps]);
	}

	void InputModule::ClearInputData()
	{
		for (size_t i = 0; i < m_Players.size(); i++)
		{
			m_Players[i].Unbind();
		}

		m_Players.clear();
		AddPlayer();

		m_InputModes.clear();
		m_InputMaps.clear();
	}

	void InputModule::SetPlayerInputMode(const size_t playerIndex, const std::string& inputMode)
	{
		auto& iter = m_InputModes.find(inputMode);
		if (iter == m_InputModes.end())
		{
			m_pEngine->GetDebug().LogError("SetPlayerInputMode: InputMode " + inputMode + " does not exist!");
			return;
		}

		PlayerInput* player = GetPlayer(playerIndex);
		if (!player)
		{
			m_pEngine->GetDebug().LogError("SetPlayerInputMode: Player " + std::to_string(playerIndex) + " does not exist!");
			return;
		}

		/* Unbind current input maps and free devices */
		player->Unbind();
		player->m_InputMode = iter->second.m_Name;

		/* Add input maps */
		for (auto itor = m_InputMaps.begin(); itor != m_InputMaps.end(); ++itor)
		{
			std::map<std::string, InputMap>& maps = itor->second;
			if (maps.find(inputMode) == maps.end()) continue;
			InputMap* inputMap = &maps.at(inputMode);
			player->m_InputData.push_back({ inputMap });
		}

		/* Find and claim available devices */
		for (size_t i = 0; i < m_InputModes.at(inputMode).m_DeviceTypes.size(); i++)
		{
			InputDeviceType deviceType = m_InputModes.at(inputMode).m_DeviceTypes[i];
			const UUID deviceID = FindAvailableInputDevice(deviceType);
			if (deviceID == 0) continue;
			InputDevice* pDevice = GetInputDevice(deviceID);
			pDevice->m_PlayerIndex = (int)playerIndex;
			player->m_ClaimedDevices.push_back(deviceID);
			player->m_ClaimedDeviceTypes.push_back(deviceType);
		}
	}

	const UUID InputModule::GetDeviceUUID(const InputDeviceType deviceType, const size_t deviceID) const
	{
		for (auto itor = m_InputDevices.begin(); itor != m_InputDevices.end(); itor++)
		{
			if (itor->second.m_DeviceType != deviceType) continue;
			if (itor->second.m_DeviceID != deviceID) continue;
			return itor->first;
		}
		return 0;
	}

	InputDevice* InputModule::GetInputDevice(const UUID deviceID)
	{
		if (m_InputDevices.find(deviceID) == m_InputDevices.end()) return nullptr;
		return &m_InputDevices.at(deviceID);
	}

	InputMode* InputModule::GetInputMode(const std::string& name)
	{
		if (m_InputModes.find(name) == m_InputModes.end()) return nullptr;
		return &m_InputModes.at(name);
	}

	bool& InputModule::InputBlocked()
	{
		return m_InputBlocked;
	}

	PlayerInput* InputModule::GetPlayer(size_t playIndex)
	{
		return playIndex >= m_Players.size() ? nullptr : &m_Players[playIndex];
	}

	const PlayerInput* InputModule::GetPlayer(size_t playIndex) const
	{
		return playIndex >= m_Players.size() ? nullptr : &m_Players[playIndex];
	}

	float InputModule::GetAxis(size_t playerIndex, const std::string& inputMap, const std::string& actionName) const
	{
		const PlayerInput* player = GetPlayer(playerIndex);
		return player ? player->GetAxis(inputMap, actionName) : 0.0f;
	}

	float InputModule::GetAxisDelta(size_t playerIndex, const std::string& inputMap, const std::string& actionName) const
	{
		const PlayerInput* player = GetPlayer(playerIndex);
		return player ? player->GetAxisDelta(inputMap, actionName) : 0.0f;
	}

	bool InputModule::GetBool(size_t playerIndex, const std::string& inputMap, const std::string& actionName) const
	{
		const PlayerInput* player = GetPlayer(playerIndex);
		return player ? player->GetBool(inputMap, actionName) : 0.0f;
	}

	glm::vec2 InputModule::GetCursorPos(size_t playerIndex) const
	{
		const PlayerInput* player = GetPlayer(playerIndex);
		return player ? player->GetCursorPos() : glm::vec2{};
	}

	bool InputModule::IsCursorDown(size_t playerIndex) const
	{
		const PlayerInput* player = GetPlayer(playerIndex);
		return player ? player->IsCursorDown() : false;
	}

	void InputModule::FreeDevice(const UUID deviceId)
	{
		InputDevice* pDevice = GetInputDevice(deviceId);
		if (!pDevice) return;
		pDevice->m_PlayerIndex = -1;
	}

	const UUID InputModule::FindAvailableInputDevice(const InputDeviceType deviceType) const
	{
		for (auto itor = m_InputDevices.begin(); itor != m_InputDevices.end(); itor++)
		{
			if (itor->second.m_DeviceType != deviceType || itor->second.m_PlayerIndex != -1)
				continue;
			return itor->first;
		}
		return 0;
	}

	void InputModule::SetCursorBounds(const glm::vec4& bounds)
	{
		m_CursorBounds = bounds;
	}

	const glm::vec4& InputModule::GetCursorBounds()
	{
		return m_CursorBounds;
	}

	void InputModule::SetScreenScale(const glm::vec2& scale)
	{
		m_ScreenScale = scale;
	}

	const glm::vec2& InputModule::GetScreenScale()
	{
		return m_ScreenScale;
	}

	void InputModule::OnProcessData()
	{
		if (!m_pEngine->HasData("Input")) return;
		m_InputModes.clear();

		std::vector<char> buffer = m_pEngine->GetData("Input");

		BinaryMemoryStream memoryStream{ buffer };
		BinaryStream* stream = &memoryStream;
		
		size_t inputModesCount;
		stream->Read(inputModesCount);
		for (size_t i = 0; i < inputModesCount; ++i)
		{
			std::string name;
			size_t deviceTypesCount;
			stream->Read(name).Read(deviceTypesCount);
			stream->Write(deviceTypesCount);

			InputMode mode{ name };
			mode.m_DeviceTypes.resize(deviceTypesCount);
			for (size_t j = 0; j < deviceTypesCount; ++j)
			{
				stream->Read(mode.m_DeviceTypes[j]);
			}
			m_InputModes.emplace(name, std::move(mode));
		}

		m_InputMaps.clear();

		size_t inputMapsCount;
		stream->Read(inputMapsCount);
		for (size_t i = 0; i < inputMapsCount; ++i)
		{
			std::string inputMapName;
			size_t actionsCount;
			stream->Read(inputMapName).Read(actionsCount);

			/* Make sure the key exists */
			if (m_InputMaps.find(inputMapName) == m_InputMaps.end())
				m_InputMaps.emplace(inputMapName, std::map<std::string, InputMap>());

			std::map<std::string, InputMap>& inputMaps = m_InputMaps.at(inputMapName);

			for (size_t j = 0; j < actionsCount; ++j)
			{
				std::string actionName;
				InputMappingType actionMapping;
				AxisBlending axisBlending = AxisBlending::Jump;
				float blendingSpeed = 0.0f;

				stream->Read(actionName).Read(actionMapping);
				if (actionMapping == InputMappingType::Float)
				{
					stream->Read(axisBlending).Read(blendingSpeed);
				}

				size_t bindingsCount;
				stream->Read(bindingsCount);
				for (size_t k = 0; k < bindingsCount; ++k)
				{
					std::string bindingName;
					InputState inputState;
					float multiplier;
					std::string inputMode;
					bool mapDeltaToValue;
					std::string bindingString;
					KeyBindingCompact compact;

					stream->Read(bindingName).Read(inputState).
						Read(multiplier).Read(inputMode).
						Read(mapDeltaToValue).Read(bindingString).Read(compact);

					if (inputMaps.find(inputMode) == inputMaps.end())
						inputMaps.emplace(inputMode, InputMap{ inputMapName });

					InputMap& inputMap = inputMaps.at(inputMode);

					/* Get the InputAction */
					if (inputMap.m_Actions.find(actionName) == inputMap.m_Actions.end())
						inputMap.m_Actions.emplace(actionName, InputAction{ actionName, actionMapping, axisBlending, blendingSpeed });

					InputAction& inputAction = inputMap.m_Actions.at(actionName);

					inputAction.m_Bindings.push_back(InputBinding{ bindingName, inputState, multiplier, mapDeltaToValue, KeyBinding{bindingString, compact} });
				}
			}
		}

	}

	void InputModule::Initialize()
	{
		/* Add default devices */
		m_DefaultKeyboardDeviceIndex = UUID();
		m_InputDevices.emplace(m_DefaultKeyboardDeviceIndex, InputDevice("Generic Keyboard", InputDeviceType::Keyboard, 0));
		m_DefaultMouseDeviceIndex = UUID();
		m_InputDevices.emplace(m_DefaultMouseDeviceIndex, InputDevice("Generic Mouse", InputDeviceType::Mouse, 0));

		AddPlayer();
	}

	void InputModule::PostInitialize()
	{
		/* Allow input */
		m_InputBlocked = false;
	}

	void InputModule::Cleanup()
	{
		m_Players.clear();
		m_InputDevices.clear();
		m_InputModes.clear();
		m_InputMaps.clear();
	}

	void InputModule::Update()
	{
		OnUpdate();
		for (size_t i = 0; i < m_Players.size(); i++)
		{
			m_Players[i].Update();
		}
	}

	void InputModule::OnBeginFrame()
	{
		for (size_t i = 0; i < m_Players.size(); ++i)
		{
			m_Players[i].ClearActions();
		}
	}

	void InputModule::ReadInputModes(YAML::Node& node)
	{
		m_InputModes.clear();

		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node inputModeNode = node[i];
			YAML::Node nameNode = inputModeNode["Name"];
			std::string name = nameNode.as<std::string>();
			InputMode inputMode{ name };
			YAML::Node deviceTypesNode = inputModeNode["DeviceTypes"];
			for (size_t i = 0; i < deviceTypesNode.size(); i++)
			{
				YAML::Node deviceTypeNode = deviceTypesNode[i];
				const std::string deviceTypeString = deviceTypeNode.as<std::string>();
				InputDeviceType deviceType;
				Enum<InputDeviceType>().FromString(deviceTypeString, deviceType);
				inputMode.m_DeviceTypes.push_back(deviceType);
			}
			m_InputModes.emplace(inputMode.m_Name.data(), std::move(inputMode));
		}
	}

	void InputModule::ReadInputMaps(YAML::Node& node)
	{
		m_InputMaps.clear();

		auto inputMappingEnum = Enum<InputMappingType>();
		auto keyStateEnum = Enum<InputState>();
		auto axisBlendingEnum = Enum<AxisBlending>();

		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node inputMapNode = node[i];
			YAML::Node inputMapNameNode = inputMapNode["Name"];
			const std::string inputMapName = inputMapNameNode.as<std::string>();
			/* Make sure the key exists */
			if (m_InputMaps.find(inputMapName) == m_InputMaps.end())
				m_InputMaps.emplace(inputMapName, std::map<std::string, InputMap>());

			std::map<std::string, InputMap>& inputMaps = m_InputMaps.at(inputMapName);

			YAML::Node actionsMapNode = inputMapNode["Actions"];
			for (size_t j = 0; j < actionsMapNode.size(); j++)
			{
				YAML::Node actionMapNode = actionsMapNode[j];
				YAML::Node actionMapNameNode = actionMapNode["Name"];
				YAML::Node actionMapActionMappingNode = actionMapNode["ActionMapping"];
				YAML::Node actionMapAxisBlendingNode = actionMapNode["AxisBlending"];
				YAML::Node actionMapAxisBlendingSpeedNode = actionMapNode["AxisBlendingSpeed"];
				YAML::Node actionMapBindingsNode = actionMapNode["Bindings"];
				const std::string actionMapName = actionMapNameNode.as<std::string>();
				const std::string actionMapMappingString = actionMapActionMappingNode.as<std::string>();
				const std::string actionMapAxisBlendingString = actionMapAxisBlendingNode.as<std::string>();
				const float axisBlendingSpeed = actionMapAxisBlendingSpeedNode.as<float>();
				InputMappingType actionMappingType;
				inputMappingEnum.FromString(actionMapMappingString, actionMappingType);
				AxisBlending axisBlending;
				axisBlendingEnum.FromString(actionMapAxisBlendingString, axisBlending);

				for (size_t k = 0; k < actionMapBindingsNode.size(); k++)
				{
					/* Get the InputMap for the corresponding InputMode */
					YAML::Node bindingNode = actionMapBindingsNode[k];
					YAML::Node bindingInputModeNode = bindingNode["InputMode"];

					const std::string bindingInputModeName = bindingInputModeNode.as<std::string>();
					if (inputMaps.find(bindingInputModeName) == inputMaps.end())
						inputMaps.emplace(bindingInputModeName, InputMap{ inputMapName });

					InputMap& inputMap = inputMaps.at(bindingInputModeName);

					/* Get the InputAction */
					if (inputMap.m_Actions.find(actionMapName) == inputMap.m_Actions.end())
						inputMap.m_Actions.emplace(actionMapName, InputAction{ actionMapName, actionMappingType, axisBlending, axisBlendingSpeed });

					InputAction& inputAction = inputMap.m_Actions.at(actionMapName);

					/* Add the binding */
					YAML::Node bindingNameNode = bindingNode["Name"];
					YAML::Node keyStateNode = bindingNode["State"];
					YAML::Node bindingMultiplierNode = bindingNode["Multiplier"];
					YAML::Node bindingBindingNode = bindingNode["Binding"];
					YAML::Node mapDeltaToValueNode = bindingNode["MapDeltaToValue"];

					const std::string bindingName = bindingNameNode.as<std::string>();
					const std::string bindingPath = bindingBindingNode.as<std::string>();
					const float multiplier = bindingMultiplierNode.as<float>();
					const bool mapDeltaToValue = mapDeltaToValueNode.as<bool>();

					KeyBinding keyBinding{ bindingPath };

					InputState keyState = InputState::Axis;
					if (!keyBinding.m_IsAxis)
					{
						const std::string keyStateString = keyStateNode.as<std::string>();
						keyStateEnum.FromString(keyStateString, keyState);
					}

					inputAction.m_Bindings.emplace_back(InputBinding{ bindingName , keyState , multiplier, mapDeltaToValue, keyBinding });
				}
			}
		}
	}
}
