#pragma once

#include "cli/cli_app.hpp"

namespace firelight::cli {

// Runs the headless `login` subcommand: authenticates with RetroAchievements
// (persisting the login token on success, exactly as the in-app login does) and
// returns a process exit code (0 on success). Prompts on stdin for a password
// when neither --password nor --token was supplied.
int runLogin(int argc, char **argv, const CliOptions &opts);

} // namespace firelight::cli
