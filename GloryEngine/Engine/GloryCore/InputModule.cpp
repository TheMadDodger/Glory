#include "InputModule.h"

namespace Glory
{
	const char* Key_InputMaps = "InputMaps";
	const char* Key_InputModes = "InputModes";

	InputModule::InputModule()
		: m_Players()
	{
	}

	InputModule::~InputModule()
	{
	}

	const std::type_info& InputModule::GetModuleType()
	{
		return typeid(InputModule);
	}

	void InputModule::OnInput(InputEvent& event)
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
	}

	size_t InputModule::AddPlayer()
	{
		const size_t index = m_Players.size();
		m_Players.push_back({ this, index });
		return index;
	}

	void InputModule::ReadInputData(YAML::Node& node)
	{
		ReadInputModes(node[Key_InputModes]);
		ReadInputMaps(node[Key_InputMaps]);
	}

	void InputModule::SetPlayerInputMode(const size_t player, const std::string& inputMode)
	{

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

	void InputModule::Initialize()
	{
		/* Add default devices */
		m_DefaultKeyboardDeviceIndex = UUID();
		m_InputDevices.emplace(m_DefaultKeyboardDeviceIndex, InputDevice("Generic Keyboard", InputDeviceType::Keyboard, 0));
		m_DefaultMouseDeviceIndex = UUID();
		m_InputDevices.emplace(m_DefaultMouseDeviceIndex, InputDevice("Generic Mouse", InputDeviceType::Mouse, 0));

		/* Temporary? */
		/* Claim the mouse and keyboard for player 1 */
		//m_InputDevices[m_DefaultKeyboardDeviceIndex].m_PlayerIndex = 0;
		//m_InputDevices[m_DefaultMouseDeviceIndex].m_PlayerIndex = 0;

		AddPlayer();

		/* TODO: Load runtime input mappings */
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
				GloryReflect::Enum<InputDeviceType>().FromString(deviceTypeString, deviceType);
				inputMode.m_DeviceTypes.push_back(deviceType);
			}
			m_InputModes.emplace(inputMode.m_Name.data(), std::move(inputMode));
		}
	}

	void InputModule::ReadInputMaps(YAML::Node& node)
	{
		m_InputMaps.clear();

		auto inputMappingEnum = GloryReflect::Enum<InputMappingType>();

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
				YAML::Node actionMapBindingsNode = actionMapNode["Bindings"];
				const std::string actionMapName = actionMapNameNode.as<std::string>();
				const std::string actionMapMappingString = actionMapActionMappingNode.as<std::string>();
				InputMappingType actionMappingType;
				inputMappingEnum.FromString(actionMapMappingString, actionMappingType);

				for (size_t k = 0; k < actionMapBindingsNode.size(); k++)
				{
					/* Get the InputMap for the corresponding InputMode */
					YAML::Node bindingNode = actionMapBindingsNode[k];
					YAML::Node bindingInputModeNode = bindingNode["InputMode"];

					const std::string bindingInputModeName = bindingInputModeNode.as<std::string>();
					if (inputMaps.find(bindingInputModeName) == inputMaps.end())
						inputMaps.emplace(bindingInputModeName, InputMap{ bindingInputModeName });

					InputMap& inputMap = inputMaps.at(bindingInputModeName);

					/* Get the InputAction */
					if (inputMap.m_Actions.find(actionMapName) == inputMap.m_Actions.end())
						inputMap.m_Actions.emplace(actionMapName, InputAction{ actionMapName, actionMappingType });

					InputAction& inputAction = inputMap.m_Actions.at(actionMapName);

					/* Add the binding */
					YAML::Node bindingNameNode = bindingNode["Name"];
					YAML::Node bindingMappingNode = bindingNode["MappingType"];
					YAML::Node bindingMultiplierNode = bindingNode["Multiplier"];
					YAML::Node bindingBindingNode = bindingNode["Binding"];

					const std::string bindingName = bindingNameNode.as<std::string>();
					const std::string bindingMappingString = bindingMappingNode.as<std::string>();
					InputMappingType bindingMappingType;
					inputMappingEnum.FromString(bindingMappingString, bindingMappingType);
					const float multiplier = bindingMultiplierNode.as<float>();
					const std::string bindingPath = bindingBindingNode.as<std::string>();

					inputAction.m_Bindings.emplace_back(InputBinding{
						bindingName , bindingMappingType , multiplier, KeyBinding{bindingPath}
						});
				}
			}
		}
	}
}
