#include "ProjectSettings.h"
#include "ListView.h"
#include "EditorUI.h"
#include "Undo.h"
#include <imgui.h>
#include <Input.h>
#include <Engine.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	const char* Key_InputMaps = "InputMaps";
	const char* Key_InputModes = "InputModes";

	const size_t BUFFER_SIZE = 256;
	char TextBuffer[BUFFER_SIZE] = "\0";

	const char* DeviceIcons[] = {
		ICON_FA_KEYBOARD,
		ICON_FA_COMPUTER_MOUSE,
		ICON_FA_GAMEPAD,
	};

	InputDeviceType CurrentDeviceType = InputDeviceType(-1);
	bool KeysNeedFilter = false;

	char FilterBuffer[200] = "\0";

	bool InputModesGui(YAMLFileRef& file, NodeValueRef settings)
	{
		bool change = false;

		NodeValueRef inputModes = settings[Key_InputModes];

		ListView listView = ListView("Input Modes");

		listView.OnDrawElement = [&](size_t index) {
			NodeValueRef inmputMode = inputModes[index];
			NodeValueRef nameNode = inmputMode["Name"];
			const std::string name = nameNode.As<std::string>();

			if (EditorUI::Header(name))
			{
				strcpy(TextBuffer, name.c_str());
				change |= EditorUI::InputText(file, nameNode.Path(), ImGuiInputTextFlags_EnterReturnsTrue);

				NodeValueRef deviceTypesNode = inmputMode["DeviceTypes"];
				ListView deviceTypesListView{ "Device Types" };

				deviceTypesListView.OnDrawElement = [&](size_t index) {
					NodeValueRef deviceTypeNode = deviceTypesNode[index];
					EditorUI::PushFlag(EditorUI::NoLabel);
					change |= EditorUI::InputEnum<InputDeviceType>(file, deviceTypeNode.Path());
					EditorUI::PopFlag();
				};

				deviceTypesListView.OnAdd = [&]() {
					const size_t count = deviceTypesNode.Size();
					Undo::StartRecord("Add Input Device");
					YAML::Node oldValue = YAML::Node(YAML::NodeType::Null);
					YAML::Node newValue = YAML::Node(YAML::NodeType::Scalar);
					newValue = "";
					deviceTypesNode.PushBack<std::string>("");
					NodeValueRef deviceTypeNode = deviceTypesNode[count];
					Undo::YAMLEdit(file, deviceTypeNode.Path(), oldValue, newValue);
					Undo::StopRecord();

					change = true;
				};

				deviceTypesListView.OnRemove = [&](int index) {
					Undo::StartRecord("Remove Input Device");
					YAML::Node oldValue = YAML::Node(YAML::NodeType::Scalar);
					YAML::Node newValue = YAML::Node(YAML::NodeType::Null);
					oldValue = deviceTypesNode[index].As<std::string>();
					deviceTypesNode.Remove(index);
					NodeValueRef deviceTypeNode = deviceTypesNode[index];
					Undo::YAMLEdit(file, deviceTypeNode.Path(), oldValue, newValue);
					Undo::StopRecord();
					change = true;
				};

				deviceTypesListView.Draw(deviceTypesNode.Size());
			}
		};

		listView.OnAdd = [&]() {
			const size_t count = inputModes.Size();
			Undo::StartRecord("Add Input Mode");
			YAML::Node oldValue = YAML::Node(YAML::NodeType::Null);
			YAML::Node newValue{ YAML::NodeType::Map };
			newValue["Name"] = "New Input Mode";
			newValue["DeviceTypes"] = YAML::Node(YAML::NodeType::Sequence);
			inputModes.PushBack(newValue);
			NodeValueRef inputMode = inputModes[count];
			Undo::YAMLEdit(file, inputMode.Path(), oldValue, newValue);
			Undo::StopRecord();

			change = true;
		};

		listView.OnRemove = [&](int index) {
			Undo::StartRecord("Remove Input Mode");
			YAML::Node oldValue = YAML::Node(YAML::NodeType::Map);
			YAML::Node newValue = YAML::Node(YAML::NodeType::Null);
			oldValue = inputModes[index].Node();
			inputModes.Remove(index);
			NodeValueRef inputMode = inputModes[index];
			Undo::YAMLEdit(file, inputMode.Path(), oldValue, newValue);
			Undo::StopRecord();

			inputModes.Remove(index);
			change = true;
		};

		listView.Draw(inputModes.Size());

		return change;
	}

	struct InputBindingData
	{
		InputBindingData(InputDeviceType deviceType, std::string label, size_t keyIndex, bool isAxis)
			: DeviceType(deviceType), Label(label), KeyIndex(keyIndex), IsAxis(isAxis) {}

		InputDeviceType DeviceType;
		std::string Label;
		size_t KeyIndex;
		bool IsAxis;
	};
	std::vector<InputBindingData> FilteredBindings;

	void BackButton()
	{
		/* Back button */
		const float availableWidth = ImGui::GetContentRegionAvail().x;
		if (ImGui::Selectable(ICON_FA_ARROW_LEFT, false, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups))
		{
			CurrentDeviceType = InputDeviceType(-1);
			KeysNeedFilter = true;
		}

		ImGui::Separator();
	}

	bool KeyPopup(std::string& value)
	{
		bool keyChosen = false;

		if (ImGui::BeginPopup("BindingPicker"))
		{
			ImGui::TextUnformatted(ICON_FA_MAGNIFYING_GLASS);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::InputText("##search", FilterBuffer, 200))
				KeysNeedFilter = true;

			const ImVec2 availablePopupRegion = ImGui::GetContentRegionAvail();

			const std::string_view search = FilterBuffer;

			if (KeysNeedFilter)
			{
				FilteredBindings.clear();
				if (CurrentDeviceType == InputDeviceType(-1) || CurrentDeviceType == InputDeviceType::Keyboard)
				{
					auto keyEnum = GloryReflect::Enum<KeyboardKey>();
					for (size_t i = 0; i < keyEnum.NumValues(); i++)
					{
						std::string keyString;
						keyEnum.ToString(KeyboardKey(i), keyString);
						if (keyString.find(search) == std::string::npos) continue;
						FilteredBindings.push_back({ InputDeviceType::Keyboard, keyString, i, false });
					}
				}

				if (CurrentDeviceType == InputDeviceType(-1) || CurrentDeviceType == InputDeviceType::Mouse)
				{
					auto keyEnum = GloryReflect::Enum<MouseButton>();
					for (size_t i = 0; i < keyEnum.NumValues(); i++)
					{
						std::string buttonString;
						keyEnum.ToString(MouseButton(i), buttonString);
						if (buttonString.find(search) == std::string::npos) continue;
						FilteredBindings.push_back({ InputDeviceType::Mouse, buttonString, i, false });
					}

					auto axisEnum = GloryReflect::Enum<MouseAxis>();
					for (size_t i = 0; i < axisEnum.NumValues(); i++)
					{
						std::string axisString;
						axisEnum.ToString(MouseAxis(i), axisString);
						if (axisString.find(search) == std::string::npos) continue;
						FilteredBindings.push_back({ InputDeviceType::Mouse, axisString, i, true });
					}
				}

				if (CurrentDeviceType == InputDeviceType(-1) || CurrentDeviceType == InputDeviceType::Gamepad)
				{

				}

				KeysNeedFilter = false;
			}

			if (CurrentDeviceType != InputDeviceType(-1))
			{
				BackButton();
			}

			if (CurrentDeviceType == InputDeviceType(-1) && search.empty())
			{
				auto deviceTypeEnum = GloryReflect::Enum<InputDeviceType>();
				for (size_t i = 0; i < deviceTypeEnum.NumValues(); i++)
				{
					InputDeviceType deviceType = InputDeviceType(i);
					std::string deviceTypeString;
					deviceTypeEnum.ToString(deviceType, deviceTypeString);
					const float availableWidth = ImGui::GetContentRegionAvail().x;
					const ImVec2 cursorPos = ImGui::GetCursorPos();
					if (ImGui::Selectable(DeviceIcons[i], false, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups))
					{
						CurrentDeviceType = deviceType;
						KeysNeedFilter = true;
					}

					ImGui::SameLine();
					ImGui::SetCursorPosX(cursorPos.x + 30.0f);
					ImGui::TextUnformatted(deviceTypeString.c_str());

					ImGui::SameLine();
					ImGui::SetCursorPos({ cursorPos.x + availableWidth - ImGui::CalcTextSize(ICON_FA_ARROW_RIGHT).x, cursorPos.y });
					ImGui::TextUnformatted(ICON_FA_ARROW_RIGHT);
				}
			}
			else
			{
				if (ImGui::BeginChild("Filtered Bindings"))
				{
					for (size_t i = 0; i < FilteredBindings.size(); i++)
					{
						const InputBindingData& bindingData = FilteredBindings[i];
						const float availableWidth = ImGui::GetContentRegionAvail().x;
						const ImVec2 cursorPos = ImGui::GetCursorPos();

						ImGui::PushID((int)i);
						/* Icon */
						std::string bindingString;
						GloryReflect::Enum<InputDeviceType>().ToString(bindingData.DeviceType, bindingString);
						std::filesystem::path bindingPath{ bindingString };
						bindingPath = bindingPath.append(bindingData.IsAxis ? "Axis" : "Key").append(bindingData.Label);
						if (ImGui::Selectable(bindingData.IsAxis ? ICON_FA_ARROW_RIGHT_ARROW_LEFT : ICON_FA_CIRCLE_DOWN, bindingPath.string() == value, ImGuiSelectableFlags_AllowItemOverlap))
						{
							value = bindingPath.string();
							keyChosen = true;
						}

						/* Key name */
						ImGui::SameLine();
						ImGui::SetCursorPosX(cursorPos.x + 30.0f);
						ImGui::TextUnformatted(bindingData.Label.c_str());
						ImGui::PopID();
					}
				}
				ImGui::EndChild();
			}

			if (keyChosen)
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		return keyChosen;
	}

	bool KeyDrowdown(std::string_view label, std::string& value)
	{
		ImGui::PushID(label.data());

		bool openPopup = false;
		float start, width;
		EditorUI::EmptyDropdown(EditorUI::MakeCleanName(label), value, [&]
		{
			KeysNeedFilter = true;
			CurrentDeviceType = InputDeviceType(-1);
			openPopup = true;
		}, start, width);

		const float scrollHeight = ImGui::GetScrollY();

		if (openPopup)
			ImGui::OpenPopup("BindingPicker");
		openPopup = false;

		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		Window* pWindow = Game::GetGame().GetEngine()->GetWindowModule()->GetMainWindow();
		int mainWindowWidth, mainWindowHeight;
		pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
		ImGui::SetNextWindowPos({ windowPos.x + start, windowPos.y + cursor.y - 2.5f - scrollHeight });
		ImGui::SetNextWindowSize({ width, mainWindowHeight - windowPos.y - cursor.y - 10.0f + scrollHeight });
		bool change = KeyPopup(value);
		ImGui::PopID();
		return change;
	}

	bool InputMapsGui(YAMLFileRef& file, NodeValueRef settings)
	{
		bool change = false;

		std::vector<std::string> inputModeNamesTemp;
		std::vector<std::string_view> inputModeNames;
		NodeValueRef inputModes = settings[Key_InputModes];
		for (size_t i = 0; i < inputModes.Size(); ++i)
		{
			const std::string name = inputModes[i]["Name"].As<std::string>();
			inputModeNamesTemp.push_back(name.c_str());
			inputModeNames.push_back(inputModeNamesTemp[i].c_str());
		}

		NodeValueRef inputMaps = settings[Key_InputMaps];

		ListView listView = ListView("Input Maps");

		listView.OnDrawElement = [&](size_t index) {
			NodeValueRef inmputMode = inputMaps[index];
			NodeValueRef nameNode = inmputMode["Name"];
			const std::string name = nameNode.As<std::string>();

			if (EditorUI::Header(name))
			{
				change |= EditorUI::InputText(file, nameNode.Path(), ImGuiInputTextFlags_EnterReturnsTrue);

				NodeValueRef actionsNode = inmputMode["Actions"];
				ListView actionsListView{ "Actions" };

				actionsListView.OnDrawElement = [&](size_t index) {
					NodeValueRef actionNode = actionsNode[index];

					NodeValueRef actionNameNode = actionNode["Name"];
					const std::string actionName = actionNameNode.As<std::string>();

					if (EditorUI::Header(actionName))
					{
						change |= EditorUI::InputText(file, actionNameNode.Path(), ImGuiInputTextFlags_EnterReturnsTrue);

						NodeValueRef actionMappingNode = actionNode["ActionMapping"];
						InputMappingType actionMapping = actionMappingNode.AsEnum<InputMappingType>();
						NodeValueRef axisBlendingNode = actionNode["AxisBlending"];
						AxisBlending axisBlending = axisBlendingNode.AsEnum<AxisBlending>();
						NodeValueRef axisBlendingSpeedNode = actionNode["AxisBlendingSpeed"];

						change |= EditorUI::InputEnum<InputMappingType>(file, actionMappingNode.Path());

						if (actionMapping == InputMappingType::Float)
						{
							change |= EditorUI::InputEnum<AxisBlending>(file, axisBlendingNode.Path());
							float blendingSpeed = axisBlendingSpeedNode.As<float>();
							change |= EditorUI::InputFloat(file, axisBlendingSpeedNode.Path(), 0.0f);
						}

						NodeValueRef bindingsNode = actionNode["Bindings"];

						ListView bindingsListView{ "Bindings" };

						bindingsListView.OnDrawElement = [&](size_t index) {
							NodeValueRef bindingNode = bindingsNode[index];
							NodeValueRef bindingNameNode = bindingNode["Name"];
							const std::string bindingName = bindingNameNode.As<std::string>();

							if (EditorUI::Header(bindingName))
							{
								NodeValueRef stateNode = bindingNode["State"];
								NodeValueRef multiplierNode = bindingNode["Multiplier"];
								NodeValueRef inputModeNode = bindingNode["InputMode"];
								NodeValueRef bindingKeyNode = bindingNode["Binding"];
								NodeValueRef mapDeltaToValueNode = bindingNode["MapDeltaToValue"];
								std::string stateString = stateNode.As<std::string>();
								std::string bindingString = bindingKeyNode.As<std::string>();

								KeyBinding binding{ bindingString };
								change |= EditorUI::InputText(file, bindingNameNode.Path());

								if (actionMapping == InputMappingType::Bool && binding.m_DeviceType != InputDeviceType(-1) && !binding.m_IsAxis)
								{
									change |= EditorUI::InputEnum<InputState>(file, stateNode.Path(), {size_t(InputState::Axis)});
								}

								if (actionMapping == InputMappingType::Float)
								{
									bool mapToDelta = mapDeltaToValueNode.As<bool>();

									change |= EditorUI::CheckBox(file, mapDeltaToValueNode.Path());

									float multiplier = multiplierNode.As<float>();
									change |= EditorUI::InputFloat(file, multiplierNode.Path());
								}

								std::string inputMode = inputModeNode.As<std::string>();
								size_t inputModeIndex = 0;
								for (size_t i = 0; i < inputModeNames.size(); i++)
								{
									if (inputMode == inputModeNames[i])
									{
										inputModeIndex = i;
										break;
									}
								}
								if (inputMode.empty() && inputModeNames.size() > 0)
								{
									inputMode = inputModeNames[inputModeIndex];
									inputModeNode.Set(inputMode);
								}

								const std::string oldInputModeName = std::string(inputModeNames[inputModeIndex]);
								if (EditorUI::InputDropdown("Input Mode", inputModeNames, &inputModeIndex, inputMode))
								{
									inputMode = inputModeNames[inputModeIndex];
									std::string newInputModeName = std::string(inputModeNames[inputModeIndex]);
									Undo::StartRecord("Input Mode");
									Undo::ApplyYAMLEdit(file, inputModeNode.Path(), oldInputModeName, newInputModeName);
									Undo::StopRecord();
									change = true;
								}

								const std::string oldBindingString = bindingString;
								if (KeyDrowdown("Binding", bindingString))
								{
									Undo::StartRecord("Input Mode");
									Undo::ApplyYAMLEdit(file, bindingKeyNode.Path(), oldBindingString, bindingString);
									Undo::StopRecord();
									change = true;
								}
							}
						};

						bindingsListView.OnAdd = [&]() {
							YAML::Node newNode{ YAML::NodeType::Map };
							newNode["Name"] = "New Binding";
							newNode["State"] = "KeyDown";
							newNode["MapDeltaToValue"] = false;
							newNode["Multiplier"] = 1.0f;
							newNode["InputMode"] = "";
							newNode["Binding"] = "";

							const size_t count = bindingsNode.Size();
							Undo::StartRecord("Add Binding");
							YAML::Node oldValue = YAML::Node(YAML::NodeType::Null);
							NodeValueRef bindingNode = bindingsNode[count];
							Undo::YAMLEdit(file, bindingNode.Path(), oldValue, newNode);
							bindingsNode.PushBack(newNode);
							Undo::StopRecord();
							change = true;
						};

						bindingsListView.OnRemove = [&](int index) {
							Undo::StartRecord("Remove Binding");
							YAML::Node oldValue = bindingsNode[index].Node();
							YAML::Node newValue = YAML::Node(YAML::NodeType::Null);
							NodeValueRef bindingNode = bindingsNode[index];
							Undo::YAMLEdit(file, bindingNode.Path(), oldValue, newValue);
							bindingsNode.Remove(index);
							Undo::StopRecord();
							change = true;
						};

						bindingsListView.Draw(bindingsNode.Size());
					}
				};

				actionsListView.OnAdd = [&]() {
					YAML::Node newNode{ YAML::NodeType::Map };
					newNode["Name"] = "New Action";
					newNode["ActionMapping"] = "Bool";
					newNode["AxisBlending"] = "Jump";
					newNode["AxisBlendingSpeed"] = 5.0f;
					newNode["Bindings"] = YAML::Node(YAML::NodeType::Sequence);

					const size_t count = actionsNode.Size();
					Undo::StartRecord("Add Input Action");
					YAML::Node oldValue = YAML::Node(YAML::NodeType::Null);
					NodeValueRef actionNode = actionsNode[count];
					Undo::YAMLEdit(file, actionNode.Path(), oldValue, newNode);
					actionsNode.PushBack(newNode);
					Undo::StopRecord();
					change = true;
				};

				actionsListView.OnRemove = [&](int index) {
					Undo::StartRecord("Remove Input Action");
					YAML::Node oldValue = actionsNode[index].Node();
					YAML::Node newValue = YAML::Node(YAML::NodeType::Null);
					NodeValueRef actionNode = actionsNode[index];
					Undo::YAMLEdit(file, actionNode.Path(), oldValue, newValue);
					actionsNode.Remove(index);
					Undo::StopRecord();

					change = true;
				};

				actionsListView.Draw(actionsNode.Size());
			}
		};

		listView.OnAdd = [&]() {
			YAML::Node newNode{ YAML::NodeType::Map };
			newNode["Name"] = "New Input Map";
			newNode["Actions"] = YAML::Node(YAML::NodeType::Sequence);

			const size_t count = inputMaps.Size();
			Undo::StartRecord("Add Input Map");
			YAML::Node oldValue = YAML::Node(YAML::NodeType::Null);
			NodeValueRef inputMap = inputMaps[count];
			Undo::YAMLEdit(file, inputMap.Path(), oldValue, newNode);
			inputMaps.PushBack(newNode);
			Undo::StopRecord();

			change = true;
		};

		listView.OnRemove = [&](int index) {
			Undo::StartRecord("Remove Input Map");
			YAML::Node oldValue = inputMaps[index].Node();
			YAML::Node newValue = YAML::Node(YAML::NodeType::Null);
			NodeValueRef inputMap = inputMaps[index];
			Undo::YAMLEdit(file, inputMap.Path(), oldValue, newValue);
			inputMaps.Remove(index);
			Undo::StopRecord();
			change = true;
		};

		listView.Draw(inputMaps.Size());

		return change;
	}

	bool InputSettings::OnGui()
	{
		bool change = false;
		ImGui::BeginChild("Input Settings");
		change |= InputModesGui(m_YAMLFile, RootValue());
		change |= InputMapsGui(m_YAMLFile, RootValue());
		ImGui::EndChild();
		return change;
	}

	void InputSettings::OnSettingsLoaded()
	{
		SETTINGS_DEFAULT_KEY(inputModes, Key_InputModes, Sequence);
		SETTINGS_DEFAULT_KEY(kayMaps, Key_InputMaps, Sequence);
	}

	void InputSettings::OnStartPlay_Impl()
	{
		Game::GetGame().GetEngine()->GetInputModule()->ReadInputData(RootValue().Node());
		Game::GetGame().GetEngine()->GetInputModule()->InputBlocked() = false;
	}

	void InputSettings::OnStopPlay_Impl()
	{
		Game::GetGame().GetEngine()->GetInputModule()->InputBlocked() = true;
		Game::GetGame().GetEngine()->GetInputModule()->ClearInputData();
	}
}