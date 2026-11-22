#pragma once

#include <optional>
#include <string>
#include <vector>

namespace stud::ui {

// `--instance <name>` from a launch, and the arguments without it. False with the
// reason when the flag is given twice or with nothing after it: either would run a
// Stud the person asking did not mean.
bool take_instance_option(const std::vector<std::string>& arguments, std::optional<std::string>* named,
                          std::vector<std::string>* rest, std::string* error);

}  // namespace stud::ui
