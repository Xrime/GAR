//
// Created by xint2 on 11/06/2026.
//
#include <iostream>
#include  <windows.h>
#include "../../include/ui/tui_app.h"
#include "../../include/ui/settings.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::cout << "\n";
    std::cout << "  ██████╗  █████╗ ██████╗ " << std::endl;
    std::cout << " ██╔════╝ ██╔══██╗██╔══██╗" << std::endl;
    std::cout << " ██║  ███╗███████║██████╔╝ " << std::endl;
    std::cout << " ██║   ██║██╔══██║██╔══██╗ " << std::endl;
    std::cout << " ╚██████╔╝██║  ██║██║  ██║ " << std::endl;
    std::cout << "  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ " << std::endl;

    auto settings = gar::ui::LoadSettingsOrDefault("gar_tui.conf");
    gar::ui::tuiApp app(settings);
    return app.Run();
}

