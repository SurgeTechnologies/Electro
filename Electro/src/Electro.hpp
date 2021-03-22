//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved

//Core
#include "Core/ElectroBase.hpp"
#include "Core/ElectroLog.hpp"
#include "Core/ElectroRef.hpp"
#include "Core/ElectroApplication.hpp"
#include "Core/ElectroWindow.hpp"
#include "Core/ElectroLog.hpp"
#include "Core/ElectroLayer.hpp"
#include "Core/ElectroLayerStack.hpp"
#include "Core/ElectroTimestep.hpp"
#include "Core/ElectroInput.hpp"

//Events
#include "Core/Events/ElectroEvent.hpp"
#include "Core/Events/ElectroApplicationEvent.hpp"
#include "Core/Events/ElectroKeyEvent.hpp"
#include "Core/Events/ElectroMouseEvent.hpp"

//Renderer
#include "Renderer/ElectroRenderer.hpp"
#include "Renderer/ElectroRenderer2D.hpp"
#include "Renderer/ElectroRenderCommand.hpp"
#include "Renderer/ElectroVertexBuffer.hpp"
#include "Renderer/ElectroIndexBuffer.hpp"
#include "Renderer/ElectroShader.hpp"
#include "Renderer/ElectroFramebuffer.hpp"
#include "Renderer/ElectroTexture.hpp"
#include "Renderer/ElectroPipeline.hpp"
#include "Renderer/ElectroMesh.hpp"
#include "Renderer/ElectroMeshFactory.hpp"
#include "Renderer/ElectroConstantBuffer.hpp"
#include "Renderer/ElectroEditorCamera.hpp"
#include "Renderer/ElectroCamera.hpp"

//Maths
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>