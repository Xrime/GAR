//
// Created by xint2 on 30/04/2026.
//

#ifndef GAR_RENDERER_H
#define GAR_RENDERER_H

#include <string>

namespace gar::renderer {

    class Renderer {
    public:
        static std::string htmlToText(const std::string& html);
    };
}

#endif //GAR_RENDERER_H