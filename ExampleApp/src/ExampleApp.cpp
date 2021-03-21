#include "Electro.hpp"
#include "Core/ElectroEntryPoint.hpp"

bool OnKeyPress(Electro::KeyPressedEvent e)
{
    if(e.GetKeyCode() == Electro::Key::A)
        ELECTRO_INFO("A is Pressed");
    return false;
}

class MyLayer : public Electro::Layer
{
public:
    MyLayer()
    {
    }

    virtual ~MyLayer() = default;

    virtual void OnAttach() override
    {
        ELECTRO_INFO("Attached!");
    }

    virtual void OnDetach() override
    {
        ELECTRO_INFO("Dettached!");
    }

    void OnUpdate(Electro::Timestep ts) override
    {
    }

    void OnEvent(Electro::Event& e) override
    {
        Electro::EventDispatcher dispatcher(e);
        dispatcher.Dispatch<Electro::KeyPressedEvent>(OnKeyPress);
    }
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
