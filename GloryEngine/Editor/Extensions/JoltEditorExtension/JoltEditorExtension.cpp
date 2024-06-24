#include "JoltEditorExtension.h"
#include "PhysicsSettings.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhase.h>

#include <BroadPhaseImpl.h>

#include <EditorPlayer.h>
#include <LayerRef.h>
#include <JoltPhysicsModule.h>
#include <GLORY_YAML.h>
#include <CreateEntityObjectsCallbacks.h>
#include <PhysicsComponents.h>

#include <PhysicsBodyEditor.h>
#include <CharacterControllerEditor.h>
#include <ObjectMenu.h>
#include <EntitySceneObjectEditor.h>
#include <CreateObjectAction.h>
#include <EditableEntity.h>
#include <EntityEditor.h>
#include <EditorApplication.h>
#include <SceneManager.h>

#include <IconsFontAwesome6.h>

EXTENSION_CPP(JoltEditorExtension)

#define OBJECT_CREATE_MENU(name, component) std::stringstream name##MenuName; \
name##MenuName << STRINGIFY(Create/Entity Object/) << EntitySceneObjectEditor::GetComponentIcon<component>() << "  " << STRINGIFY(name); \
ObjectMenu::AddMenuItem(name##MenuName.str(), Create##name, T_SceneObject | T_Scene | T_Hierarchy);

namespace Glory::Editor
{
	CREATE_OBJECT_CALLBACK_CPP(PhysicsBody, PhysicsBody, ());
	CREATE_OBJECT_CALLBACK_CPP(Character, CharacterController, ());

	PhysicsSettings Physics;

	JoltEditorExtension::JoltEditorExtension()
	{
	}

	JoltEditorExtension::~JoltEditorExtension()
	{
	}

	void JoltEditorExtension::Initialize()
	{
		EditorPlayer::RegisterLoopHandler(this);

		Editor::RegisterEditor<PhysicsBodyEditor>();
		Editor::RegisterEditor<CharacterControllerEditor>();

		static constexpr char* bodyIcon = ICON_FA_CUBES_STACKED;
		static constexpr char* characterIcon = ICON_FA_PERSON;
		EntitySceneObjectEditor::AddComponentIcon<PhysicsBody>(bodyIcon);
		EntitySceneObjectEditor::AddComponentIcon<CharacterController>(characterIcon);

		ProjectSettings::Add(&Physics);

		OBJECT_CREATE_MENU(PhysicsBody, PhysicsBody);
		OBJECT_CREATE_MENU(Character, CharacterController);

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Utils::Reflect::Reflect& reflect = pEngine->Reflection();
		Reflect::SetReflectInstance(&reflect);

		pEngine->GetSceneManager()->ComponentTypesInstance();
	}

	const char* JoltEditorExtension::ModuleName()
	{
		return "Jolt Physics";
	}

	void JoltEditorExtension::HandleBeforeStart(Module* pModule)
	{
		JoltPhysicsModule* pPhysics = static_cast<JoltPhysicsModule*>(pModule);

		std::map<JPH::ObjectLayer, JPH::BroadPhaseLayer> bpLayersMapping;
		bpLayersMapping.emplace(0, 0);
		for (size_t i = 0; i < pPhysics->Settings().ArraySize("BroadPhaseLayerMapping"); ++i)
		{
			const std::string valueStr = pPhysics->Settings().ArrayValue<std::string>("BroadPhaseLayerMapping", i);
			BPLayer bpLayer;
			Enum<BPLayer>().FromString(valueStr, bpLayer);
			bpLayersMapping.emplace((uint16_t)i + 1, JPH::BroadPhaseLayer(JPH::uint8(bpLayer)));
		}
		pPhysics->BPLayerImpl().SetObjectToBroadphase(std::move(bpLayersMapping));

		const size_t bpLayerCount = Enum<BPLayer>().NumValues();
		std::vector<LayerMask> bpCollisionMapping = std::vector<LayerMask>(bpLayerCount);
		for (size_t i = 0; i < bpLayerCount; i++)
		{
			std::string layerName;
			Enum<BPLayer>().ToString(BPLayer(i), layerName);
			const LayerMask mask = pPhysics->Settings().Value<LayerMask>(layerName + "CollisionMask");
			bpCollisionMapping[i] = mask;
		}
		pPhysics->BPCollisionFilter().SetBPCollisionMapping(std::move(bpCollisionMapping));

		pPhysics->SetupPhysics();
	}

	void JoltEditorExtension::HandleStart(Module* pModule)
	{
	}

	void JoltEditorExtension::HandleStop(Module* pModule)
	{
		JoltPhysicsModule* pPhysics = static_cast<JoltPhysicsModule*>(pModule);
		pPhysics->CleanupPhysics();
	}

	void JoltEditorExtension::HandleUpdate(Module* pModule)
	{
	}
}
