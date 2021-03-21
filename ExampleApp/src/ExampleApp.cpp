#include "Electro.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Core/ElectroEntryPoint.hpp"

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
        Electro::Renderer::SetSkybox(Electro::Skybox::Create(Electro::TextureCube::Create("Electro/assets/skybox")));
        Electro::Renderer::SetSkyboxActivationBool(true);
    }

    virtual void OnDetach() override
    {
    }

    void OnUpdate(Electro::Timestep ts) override
    {
        Electro::RenderCommand::Clear();
        mCamera.OnUpdate(ts);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

        //Render
        Electro::Renderer::BeginScene(mCamera);
        Electro::Renderer::EndScene();

        Electro::RenderCommand::BindBackbuffer();
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
};

class MyElectroApp : public Electro::Application
{
public:
    MyElectroApp()
        :Electro::Application("My Lovely App")
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
