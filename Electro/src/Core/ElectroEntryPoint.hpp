//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

#ifdef ELECTRO_WINDOWS

extern Electro::Application* Electro::CreateApplication();

int main(int argc, char** argv)
{
    Electro::ELogger::EInit();
    auto app = Electro::CreateApplication();
    app->Run();
    Electro::ELogger::EShutdown();
    delete app;
}

#endif // ELECTRO_WINDOWS
