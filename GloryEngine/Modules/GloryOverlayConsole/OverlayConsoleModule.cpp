#include "OverlayConsoleModule.h"

#include <WindowModule.h>
#include <RendererModule.h>
#include <Console.h>
#include <MaterialManager.h>
#include <Engine.h>
#include <AssetManager.h>
#include <GraphicsModule.h>
#include <GPUResourceManager.h>
#include <CameraManager.h>
#include <InternalMaterial.h>
#include <InternalPipeline.h>
#include <FontData.h>
#include <FontDataStructs.h>
#include <Material.h>
#include <MaterialData.h>
#include <GameTime.h>

#include <DistributedRandom.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(OverlayConsoleModule);

	float TextScaleFactor = 0.35f;

	OverlayConsoleModule::OverlayConsoleModule():
		m_ConsoleButtonDown(false), m_ConsoleOpen(false), m_ConsoleOpenedThisFrame(false),
		m_ConsoleAnimationTime(0.0f), m_Scroll(0.0f), m_TextDirty(false),
		m_InputTextDirty(false), m_CursorPos(0), m_ConsoleInput("\0"),
		m_HistoryRewindIndex(-1), m_BackedUpInput("")
	{
	}

	OverlayConsoleModule::~OverlayConsoleModule()
	{
	}

	void OverlayConsoleModule::CollectReferences(std::vector<UUID>& references)
	{
		ModuleSettings& settings = Settings();

		std::vector<UUID> newReferences;
		newReferences.push_back(settings.Value<uint64_t>("Console Background Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Console Text Pipeline"));

		for (size_t i = 0; i < newReferences.size(); ++i)
		{
			if (!newReferences[i]) continue;
			references.push_back(newReferences[i]);
			Resource* pPipelineResource = m_pEngine->GetAssetManager().GetAssetImmediate(newReferences[i]);
			if (!pPipelineResource) continue;
			PipelineData* pPipelineData = static_cast<PipelineData*>(pPipelineResource);
			for (size_t i = 0; i < pPipelineData->ShaderCount(); ++i)
			{
				const UUID shaderID = pPipelineData->ShaderID(i);
				if (!shaderID) continue;
				references.push_back(shaderID);
			}
		}
		references.push_back(settings.Value<uint64_t>("Console Font"));
	}

	const std::type_info& OverlayConsoleModule::GetModuleType()
	{
		return typeid(OverlayConsoleModule);
	}

	bool OverlayConsoleModule::OnOverrideInputEvent(InputEvent& e)
	{
		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();
		Window* pMainWindow = pWindows->GetMainWindow();

		switch (e.State)
		{
		case InputState::KeyDown:
		{
			if (e.InputDeviceType != InputDeviceType::Keyboard) return m_ConsoleOpen;
			if (m_ConsoleOpen && e.KeyMods & KeyboardMod::ModCtrl && e.KeyID == KeyV)
			{
				const std::string clipboard = pMainWindow->GetClipboardText();
				for (size_t i = 0; i < clipboard.size(); ++i)
				{
					if (m_CursorPos >= MAX_CONSOLE_INPUT - 1) break;
					m_ConsoleInput[m_CursorPos] = clipboard[i];
					++m_CursorPos;
					m_ConsoleInput[m_CursorPos] = '\0';
				}
				m_InputTextDirty = true;
				break;
			}
			HandleKeyboardInput(pMainWindow, m_pEngine->GetConsole(), KeyboardKey(e.KeyID));
			break;
		}
		case InputState::KeyUp:
			if (e.KeyID != KeyBackQuote) break;
			m_ConsoleButtonDown = false;
			break;
		case InputState::Axis:
		{
			if (!m_ConsoleOpen) break;
			if (e.InputDeviceType != InputDeviceType::Mouse) break;
			if (e.KeyID != MouseAxis::MouseAxisScrollY) break;

			const UUID consoleFont = Settings().Value<uint64_t>("Console Font");
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(consoleFont);
			if (!pResource) return false;
			FontData* pFont = static_cast<FontData*>(pResource);

			int windowWidth, windowHeight;
			pMainWindow->GetDrawableSize(&windowWidth, &windowHeight);
			const float textScale = TextScaleFactor*windowWidth/1920.0f;
			m_Scroll += e.Delta * pFont->FontHeight() * textScale;
			m_TextDirty = true;
			break;
		}
		}

		return m_ConsoleOpen;
	}

	bool OverlayConsoleModule::OnOverrideTextEvent(TextEvent& e)
	{
		if (!m_ConsoleOpen || m_ConsoleOpenedThisFrame) return false;
		if (m_CursorPos >= MAX_CONSOLE_INPUT - 1) return true;
		m_ConsoleInput[m_CursorPos] = e.Character;
		++m_CursorPos;
		m_ConsoleInput[m_CursorPos] = '\0';
		m_InputTextDirty = true;
		return true;
	}

	void OverlayConsoleModule::Initialize()
	{
	}

	void OverlayConsoleModule::PostInitialize()
	{
		RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();
		if (!pRenderer)
		{
			m_pEngine->GetDebug().LogError("OverlayConsole does not work without a renderer");
			return;
		}

		pRenderer->AddRenderPass(RenderPassType::RP_Postblit, { "Console Pass", [this](CameraRef, const RenderFrame&) {
			OverlayPass();
		} });

		m_pEngine->GetConsole().RegisterConsole(this);

		const ModuleSettings& settings = Settings();
		const UUID consoleBackgroundPipeline = settings.Value<uint64_t>("Console Background Pipeline");
		const UUID consoleTextPipeline = settings.Value<uint64_t>("Console Text Pipeline");

		m_pConsoleBackgroundMaterial = new MaterialData();
		m_pConsoleBackgroundMaterial->SetPipeline(consoleBackgroundPipeline);
		m_pConsoleTextMaterial = new MaterialData();
		m_pConsoleTextMaterial->SetPipeline(consoleTextPipeline);

		m_pConsoleMesh.reset(new MeshData(4, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }));

		m_pConsoleLogTextMesh.reset(new MeshData(0, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }));
		m_pInputTextMesh.reset(new MeshData(0, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }));

		const float xpos = 0.0f;
		const float ypos = 0.0f;

		const float w = 1.0f;
		const float h = 1.0f;

		const glm::vec4 color{ 0.0f, 0.0f, 0.0f, 1.0f };

		VertexPosColorTex vertices[4] = {
			{ { xpos, ypos + h, }, color, {0.0f, 0.0f}},
			{ { xpos, ypos, }, color, { 0.0f, 1.0f } },
			{ { xpos + w, ypos, }, color, { 1.0f, 1.0f } },
			{ { xpos + w, ypos + h, }, color, { 1.0f, 0.0f }, }
		};

		m_pConsoleMesh->AddVertex(reinterpret_cast<float*>(&vertices[0]));
		m_pConsoleMesh->AddVertex(reinterpret_cast<float*>(&vertices[1]));
		m_pConsoleMesh->AddVertex(reinterpret_cast<float*>(&vertices[2]));
		m_pConsoleMesh->AddVertex(reinterpret_cast<float*>(&vertices[3]));
		m_pConsoleMesh->AddFace(0, 1, 2, 3);

		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();
		Window* pMainWindow = pWindows->GetMainWindow();
		pMainWindow->AddInputOverrideHandler(this);
	}

	void OverlayConsoleModule::Update()
	{
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();
		Window* pMainWindow = pWindows->GetMainWindow();

		if (m_ConsoleOpen && m_ConsoleAnimationTime < 1.0f)
			m_ConsoleAnimationTime += m_pEngine->Time().GetDeltaTime()*10.0f;
		else if (m_ConsoleOpen) m_ConsoleAnimationTime = 1.0f;
		else if (!m_ConsoleOpen && m_ConsoleAnimationTime > 0.0f)
			m_ConsoleAnimationTime -= m_pEngine->Time().GetDeltaTime()*10.0f;
		else if (!m_ConsoleOpen) m_ConsoleAnimationTime = 0.0f;

		m_ConsoleOpenedThisFrame = false;
	}

	void OverlayConsoleModule::Cleanup()
	{
		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();
		Window* pMainWindow = pWindows->GetMainWindow();
		pMainWindow->RemoveInputOverrideHandler(this);

		delete m_pConsoleBackgroundMaterial;
		m_pConsoleBackgroundMaterial = nullptr;

		delete m_pConsoleTextMaterial;
		m_pConsoleTextMaterial = nullptr;
	}

	void OverlayConsoleModule::OverlayPass()
	{
		/* Do not render if console is fully closed */
		Console& console = m_pEngine->GetConsole();
		if (!m_ConsoleOpen && m_ConsoleAnimationTime == 0.0f || console.LineCount() == 0) return;

		const UUID consoleFont = Settings().Value<uint64_t>("Console Font");
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(consoleFont);
		if (!pResource) return;
		FontData* pFont = static_cast<FontData*>(pResource);

		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pGPUResourceManager = pGraphics->GetResourceManager();
		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();

		int windowWidth, windowHeight;
		pWindows->GetMainWindow()->GetWindowSize(&windowWidth, &windowHeight);

		const float textScale = TextScaleFactor*windowWidth/1920.0f;
		const float consolePadding = 10.0f;
		const float textLineHeight = pFont->FontHeight()*textScale;
		const float consoleHeight = 20.0f*textLineHeight + consolePadding;
		const float animatedConsoleHeight = consoleHeight*(1.0f - m_ConsoleAnimationTime);
		const float textHeight = textLineHeight*console.LineCount();
		const float textStart = glm::max(consoleHeight - textHeight - consolePadding, 0.0f);
		const float maxScroll = glm::max(textHeight + consolePadding - consoleHeight, 0.0f);
		m_Scroll = glm::clamp(m_Scroll, 0.0f, maxScroll);

		/* Generate input text bracket */
		if (!m_pInputTextBracketMesh)
		{
			m_pInputTextBracketMesh.reset(new MeshData(4, sizeof(VertexPosColorTex),
				{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }));

			TextRenderData textData;
			textData.m_Color = glm::vec4(1.0f);
			textData.m_Text = "]";
			textData.m_FontID = consoleFont;
			textData.m_TextWrap = 0.0f;
			textData.m_Alignment = Alignment::Left;
			textData.m_Scale = textScale;
			textData.m_Offsets.y = -1.0f*textLineHeight;
			textData.m_Append = false;
			Utils::GenerateTextMesh(m_pInputTextBracketMesh.get(), pFont, textData);
		}

		/* Generate input text cursor */
		if (!m_pInputTextCursorMesh)
		{
			m_pInputTextCursorMesh.reset(new MeshData(4, sizeof(VertexPosColorTex),
				{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }));

			TextRenderData textData;
			textData.m_Color = glm::vec4(1.0f);
			textData.m_Text = "|";
			textData.m_FontID = consoleFont;
			textData.m_TextWrap = 0.0f;
			textData.m_Alignment = Alignment::Left;
			textData.m_Scale = textScale;
			textData.m_Offsets.y = -1.0f * pFont->FontHeight() * textScale;
			textData.m_Append = false;
			Utils::GenerateTextMesh(m_pInputTextCursorMesh.get(), pFont, textData);
		}

		/* Generate console text mesh */
		if (m_TextDirty)
		{
			m_pConsoleLogTextMesh->ClearIndices();
			m_pConsoleLogTextMesh->ClearVertices();

			const size_t maxVisibleLines = size_t(std::floor(consoleHeight/(textLineHeight))) - 1;
			const size_t lastLine = console.LineCount() - size_t(std::floor(m_Scroll/(textLineHeight)));
			const size_t firstLine = size_t(glm::max(int(lastLine - maxVisibleLines), 0));

			for (size_t i = 0; i < lastLine - firstLine; ++i)
			{
				TextRenderData textData;
				textData.m_Color = console.LineColor(firstLine + i);
				textData.m_Text = console.Line(firstLine + i);
				textData.m_FontID = consoleFont;
				textData.m_TextWrap = 0.0f;
				textData.m_Alignment = Alignment::Left;
				textData.m_Scale = textScale;
				textData.m_Offsets.y = -1.0f*textLineHeight*(i+1);
				textData.m_Append = true;
				Utils::GenerateTextMesh(m_pConsoleLogTextMesh.get(), pFont, textData);
				m_pConsoleLogTextMesh->SetDirty(true);
			}
			m_TextDirty = false;
		}

		if (m_InputTextDirty && m_CursorPos > 0)
		{
			TextRenderData textData;
			textData.m_Color = glm::vec4(1.0f);
			textData.m_Text = m_ConsoleInput;
			textData.m_FontID = consoleFont;
			textData.m_TextWrap = 0.0f;
			textData.m_Alignment = Alignment::Left;
			textData.m_Scale = textScale;
			textData.m_Offsets.y = -1.0f*textLineHeight;
			textData.m_Append = false;
			Utils::GenerateTextMesh(m_pInputTextMesh.get(), pFont, textData);
			m_pInputTextMesh->SetDirty(true);
			m_InputTextDirty = false;
		}

		ObjectData object;
		const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, windowHeight - consoleHeight + animatedConsoleHeight, 0.0f));
		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(float(windowWidth), float(consoleHeight), 1.0f));
		object.Model = translation*scale;
		object.Projection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight));

		pGraphics->EnableDepthTest(false);

		/* Draw background */
		Material* pMaterial = pGraphics->UseMaterial(m_pConsoleBackgroundMaterial);
		pMaterial->SetProperties(m_pEngine);
		pMaterial->SetObjectData(object);
		pGraphics->DrawMesh(m_pConsoleMesh.get(), 0, m_pConsoleMesh->VertexCount());

		/* Draw stencil mask */
		pGraphics->SetColorMask(false, false, false, false);
		pGraphics->EnableStencilTest(true);
		pGraphics->SetStencilMask(0xFF);
		pGraphics->ClearStencil(0);
		pGraphics->SetStencilOP(Func::OP_Replace, Func::OP_Replace, Func::OP_Replace);
		pGraphics->SetStencilFunc(CompareOp::OP_Always, 255, 0xFF);
		const glm::mat4 maskTranslation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, windowHeight - (consoleHeight - textLineHeight) + animatedConsoleHeight, 0.0f));
		object.Model = maskTranslation*scale;
		pGraphics->DrawMesh(m_pConsoleMesh.get(), 0, m_pConsoleMesh->VertexCount());
		pGraphics->SetColorMask(true, true, true, true);

		/* Draw text */
		pGraphics->SetStencilMask(0x00);
		pGraphics->SetStencilOP(Func::OP_Keep, Func::OP_Keep, Func::OP_Keep);
		pGraphics->SetStencilFunc(CompareOp::OP_Equal, 255, 0xFF);
		pMaterial = pGraphics->UseMaterial(m_pConsoleTextMaterial);
		pMaterial->SetProperties(m_pEngine);
		object.Model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, windowHeight + animatedConsoleHeight - textStart, 0.0f));
		pMaterial->SetObjectData(object);

		InternalTexture* pTextureData = pFont->GetGlyphTexture();
		if (!pTextureData) return;

		Texture* pTexture = pGPUResourceManager->CreateTexture((TextureData*)pTextureData);
		if (pTexture) pMaterial->SetTexture("textSampler", pTexture);

		pGraphics->DrawMesh(m_pConsoleLogTextMesh.get(), 0, m_pConsoleLogTextMesh->VertexCount());

		/* Disable stencil */
		pGraphics->SetStencilMask(0x00);
		pGraphics->EnableStencilTest(false);

		/* Draw input text bracket */
		const float inputTextHeight = windowHeight + animatedConsoleHeight - consoleHeight + textLineHeight + consolePadding;
		object.Model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, inputTextHeight, 0.0f));
		pMaterial->SetObjectData(object);
		pGraphics->DrawMesh(m_pInputTextBracketMesh.get(), 0, m_pInputTextBracketMesh->VertexCount());

		/* Draw input text */
		if (m_pInputTextMesh->VertexCount() > 0 && m_CursorPos > 0)
		{
			const size_t bracketGlyphIndex = pFont->GetGlyphIndex(']');
			const GlyphData* pBracketGlyph = pFont->GetGlyph(bracketGlyphIndex);
			const float inputTextXOffset = (pBracketGlyph->Size.x + (pBracketGlyph->Advance >> 6)) * textScale;
			object.Model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(inputTextXOffset, inputTextHeight, 0.0f));
			pMaterial->SetObjectData(object);
			pGraphics->DrawMesh(m_pInputTextMesh.get(), 0, m_pInputTextMesh->VertexCount());
		}

		/* Reset render textures and materials */
		pGraphics->UseMaterial(nullptr);
		pGraphics->EnableDepthTest(true);
	}

	void OverlayConsoleModule::OnConsoleClose()
	{
	}

	void OverlayConsoleModule::SetNextColor(const glm::vec4& color)
	{
	}

	void OverlayConsoleModule::ResetNextColor()
	{
	}

	void OverlayConsoleModule::Write(const std::string& line)
	{
		m_TextDirty = true;
	}

	void OverlayConsoleModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterAssetReference<PipelineData>("Console Background Pipeline", 113);
		settings.RegisterAssetReference<PipelineData>("Console Text Pipeline", 115);
		settings.RegisterAssetReference<FontData>("Console Font", 116);
	}

	void OverlayConsoleModule::HandleKeyboardInput(Window* pWindow, Console& console, KeyboardKey key)
	{
		switch (key)
		{
		case Glory::KeyBackQuote:
			if (m_ConsoleButtonDown) return;
			m_ConsoleButtonDown = true;
			m_ConsoleOpen = !m_ConsoleOpen;
			m_ConsoleOpenedThisFrame = m_ConsoleOpen;
			if (m_ConsoleOpen) pWindow->StartTextInput();
			else pWindow->StopTextInput();
			return;
		case Glory::KeyEscape:
			/* Clear input */
			m_CursorPos = 0;
			m_ConsoleInput[m_CursorPos] = '\0';
			m_InputTextDirty = true;
			return;
		case Glory::KeyBackSpace:
			/* Erase last character */
			if (m_CursorPos == 0) return;
			--m_CursorPos;
			m_ConsoleInput[m_CursorPos] = '\0';
			m_InputTextDirty = true;
			return;
		case Glory::KeyTab:
			/* Auto complete */
			return;
		case Glory::KeyKpEnter:
		case Glory::KeyReturn:
			/* Execute command */
			console.QueueCommand(m_ConsoleInput);
			m_CursorPos = 0;
			m_ConsoleInput[m_CursorPos] = '\0';
			m_InputTextDirty = true;
			m_HistoryRewindIndex = -1;
			m_BackedUpInput = "";
			return;
		case Glory::KeyUp:
		case Glory::KeyKpUp:
		{
			/* History up */
			if (console.HistoryCount() == 0) return;
			if (m_HistoryRewindIndex == -1) m_BackedUpInput = m_ConsoleInput;
			++m_HistoryRewindIndex;
			if (m_HistoryRewindIndex > console.HistoryCount() - 1) m_HistoryRewindIndex = console.HistoryCount() - 1;
			const std::string_view history = console.History(size_t(m_HistoryRewindIndex));

			m_CursorPos = 0;
			m_ConsoleInput[m_CursorPos] = '\0';
			for (size_t i = 0; i < history.size(); ++i)
			{
				if (m_CursorPos >= MAX_CONSOLE_INPUT - 1) break;
				m_ConsoleInput[m_CursorPos] = history[i];
				++m_CursorPos;
				m_ConsoleInput[m_CursorPos] = '\0';
			}
			m_InputTextDirty = true;
			return;
		}
		case Glory::KeyKpDown:
		case Glory::KeyDown:
		{
			/* History down */
			if (console.HistoryCount() == 0) return;
			if (m_HistoryRewindIndex > 0) --m_HistoryRewindIndex;
			else if (m_HistoryRewindIndex == 0)
			{
				m_HistoryRewindIndex = -1;
				m_CursorPos = 0;
				m_ConsoleInput[m_CursorPos] = '\0';
				for (size_t i = 0; i < m_BackedUpInput.size(); ++i)
				{
					if (m_CursorPos >= MAX_CONSOLE_INPUT - 1) break;
					m_ConsoleInput[m_CursorPos] = m_BackedUpInput[i];
					++m_CursorPos;
					m_ConsoleInput[m_CursorPos] = '\0';
				}
				m_BackedUpInput = "";
				m_InputTextDirty = true;
				return;
			}
			else return;
			const std::string_view history = console.History(size_t(m_HistoryRewindIndex));
			m_CursorPos = 0;
			m_ConsoleInput[m_CursorPos] = '\0';
			for (size_t i = 0; i < history.size(); ++i)
			{
				if (m_CursorPos >= MAX_CONSOLE_INPUT - 1) break;
				m_ConsoleInput[m_CursorPos] = history[i];
				++m_CursorPos;
				m_ConsoleInput[m_CursorPos] = '\0';
			}
			m_InputTextDirty = true;
			return;
		}

		case Glory::KeyLeft:
		case Glory::KeyKpLeft:
			/* @todo: Shift cursor left */
			return;
		case Glory::KeyRight:
		case Glory::KeyKpRight:
			/* @todo: Shift cursor right */
			return;
		case Glory::KeyHome:
		case Glory::KeyKpBegin:
		case Glory::KeyKpHome:
			/* @todo: Shift cursor to start */
			return;
		case Glory::KeyDelete:
		case Glory::KeyKpDelete:
			/* @todo: Delete next character */
			return;
		case Glory::KeyEnd:
		case Glory::KeyKpEnd:
			/* @todo: Shift cursor to end */
			return;
		}
	}
}
