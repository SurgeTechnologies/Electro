//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

#ifdef ELECTRO_PLATFORM_WINDOWS

extern Electro::Application* Electro::CreateApplication();

int main(int argc, char** argv)
{
    Electro::Logger::Init();
    auto app = Electro::CreateApplication();
    app->Run();
    Electro::Logger::Shutdown();
    delete app;
}

#endif // ELECTRO_WINDOWS
