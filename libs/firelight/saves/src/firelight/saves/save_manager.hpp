#pragma once
// When this PRIVATE-path file is found instead of the public
// include/firelight/saves/save_manager.hpp, pull in both the interface
// (via isave_manager.hpp, which has no conflicting name) and the concrete impl
#include <firelight/saves/isave_manager.hpp>
#include <firelight/saves/save_manager_impl.hpp>
