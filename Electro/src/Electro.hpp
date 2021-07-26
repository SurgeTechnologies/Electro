//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved

// Core
#include "Core/Base.hpp"
#include "Core/Log.hpp"
#include "Core/Ref.hpp"
#include "Core/Application.hpp"
#include "Core/Window.hpp"
#include "Core/Log.hpp"
#include "Core/Module.hpp"
#include "Core/ModuleManager.hpp"
#include "Core/Timestep.hpp"
#include "Core/Input.hpp"
#include "Core/System/OS.hpp"
#include "Core/FileSystem.hpp"

// Asset
#include "Asset/AssetBase.hpp"
#include "Asset/AssetManager.hpp"

// Project
#include "Project/Project.hpp"
#include "Project/ProjectManager.hpp"
#include "Project/ProjectSerializer.hpp"

// Events
#include "Core/Events/Event.hpp"
#include "Core/Events/ApplicationEvent.hpp"
#include "Core/Events/KeyEvent.hpp"
#include "Core/Events/MouseEvent.hpp"

// Renderer
#include "Renderer/Interface/VertexBuffer.hpp"
#include "Renderer/Interface/IndexBuffer.hpp"
#include "Renderer/Interface/Shader.hpp"
#include "Renderer/Interface/Renderbuffer.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "Renderer/Interface/Pipeline.hpp"

#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer/Camera/Camera.hpp"

#include "Renderer/Mesh.hpp"
#include "Renderer/MeshFactory.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/RenderCommand.hpp"

// Scene
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Scene/Components.hpp"
#include "Scene/Entity.hpp"

// Maths
#include "Math/Math.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Scripting
#include "Scripting/ScriptEngine.hpp"

// Runtime Exporter
#include "RumtimeExporter/RuntimeExporter.hpp"
#include "RumtimeExporter/CurrentAppPath.hpp"

// ImGui
#include <imgui.h>
#include "UIUtils/UIUtils.hpp"

#ifdef ELECTRO_ENTRYPOINT
#   include "Core/EntryPoint.hpp"
#endif
