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
#include "Core/ElectroVault.hpp"
#include "Core/System/ElectroOS.hpp"

//Events
#include "Core/Events/ElectroEvent.hpp"
#include "Core/Events/ElectroApplicationEvent.hpp"
#include "Core/Events/ElectroKeyEvent.hpp"
#include "Core/Events/ElectroMouseEvent.hpp"

//Device
#include "EDevice/EDevice.hpp"

//Renderer
#include "Renderer/Interface/ElectroVertexBuffer.hpp"
#include "Renderer/Interface/ElectroIndexBuffer.hpp"
#include "Renderer/Interface/ElectroShader.hpp"
#include "Renderer/Interface/ElectroFramebuffer.hpp"
#include "Renderer/Interface/ElectroTexture.hpp"
#include "Renderer/Interface/ElectroConstantBuffer.hpp"
#include "Renderer/Interface/ElectroPipeline.hpp"

#include "Renderer/Camera/ElectroEditorCamera.hpp"
#include "Renderer/Camera/ElectroCamera.hpp"

#include "Renderer/ElectroMesh.hpp"
#include "Renderer/ElectroMeshFactory.hpp"
#include "Renderer/ElectroRenderer.hpp"
#include "Renderer/ElectroRenderer2D.hpp"
#include "Renderer/ElectroRenderCommand.hpp"

//Maths
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef ELECTRO_ENTRYPOINT
#   include "Core/ElectroEntryPoint.hpp"
#endif