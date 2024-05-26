#pragma once
#include <string>

namespace firelight::db {
    struct LibraryContentDirectory {
        int id = -1;
        std::string path;
        int numGameFiles = 0;
    };
}
