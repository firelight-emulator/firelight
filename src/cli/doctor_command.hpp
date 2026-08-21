#pragma once

#include "cli/cli_app.hpp"

namespace firelight::cli {

// TODO
/** Reports whether the install is complete and the databases are healthy */
int runDoctor(int argc, char **argv, const CliOptions &options);

} // namespace firelight::cli
