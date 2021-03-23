#define ELECTRO_ENTRYPOINT
#include "Electro.hpp"
#include <imgui.h>

class MyLayer : public Electro::Layer
{
public:
    MyLayer()
    {
    }

    virtual ~MyLayer() = default;

    virtual void OnAttach() override
    {
        mCamera = Electro::EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
        mMesh = Electro::MeshFactory::CreateCube({ 1.0f, 1.0f, 1.0f });
        mMesh->GetMaterial()->SetColor({ 0.1f, 0.8f, 0.3f });

        auto skybox = Electro::Skybox::Create(Electro::TextureCube::Create("Electro/assets/skybox"));
        Electro::Renderer::SetSkybox(skybox);
    }

    virtual void OnDetach() override
    {
    }

    void OnUpdate(Electro::Timestep ts) override
    {
        Electro::Renderer::UpdateStats();
        Electro::Renderer2D::UpdateStats();

        Electro::RenderCommand::Clear();
        mCamera.OnUpdate(ts);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

        auto transform = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 0.0f }) * scale;
        Electro::RenderCommand::BindBackbuffer();

        //Render
        Electro::Renderer::BeginScene(mCamera);
        Electro::Renderer::SubmitMesh(mMesh, transform);
        Electro::Renderer::EndScene();
    }

    void OnImGuiRender() override
    {
        ImGui::Begin("Electro");

        if (ImGui::Button("Press me!"))
            ELECTRO_INFO("Button was pressed!");

        ImGui::End();
    }

    bool OnResize(Electro::WindowResizeEvent e)
    {
        mCamera.SetViewportSize(e.GetWidth(), e.GetHeight());
        Electro::RenderCommand::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
        return false;
    }

    bool OnKeyPress(Electro::KeyPressedEvent e)
    {
        if (e.GetKeyCode() == Electro::Key::A)
            ELECTRO_INFO("A is Pressed");
        return false;
    }

    void OnEvent(Electro::Event& e) override
    {
        Electro::EventDispatcher dispatcher(e);
        mCamera.OnEvent(e);
        dispatcher.Dispatch<Electro::KeyPressedEvent>(ELECTRO_BIND_EVENT_FN(OnKeyPress));
        dispatcher.Dispatch<Electro::WindowResizeEvent>(ELECTRO_BIND_EVENT_FN(OnResize));
    }
private:
    Electro::EditorCamera mCamera;
    Electro::Ref<Electro::Mesh> mMesh;
};

class MyElectroApp : public Electro::Application
{
public:
    MyElectroApp()
        : Electro::Application("My Lovely App")
    {
        PushLayer(new MyLayer());
    }

    ~MyElectroApp()
    {
    }
};

Electro::Application* Electro::CreateApplication()
{
    return new MyElectroApp();
}
