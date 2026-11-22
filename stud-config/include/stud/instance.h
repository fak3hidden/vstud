#pragma once

#include <string>

// A copy of Stud with its own config, data, cache and state, so several can run
// at once each signed in to a different Roblox account. The first one was made
// before there was a choice, so it keeps the plain ~/.config/stud paths and the
// named ones live beside it under instances/.

namespace stud::instances {

// The instance this process is, as STUD_INSTANCE names it; empty for the first one.
const std::string& name();

// Picks the instance for this process; empty is the first one. Refused (false, with
// `error` set) for a name that is not a single safe path component.
bool select(const std::string& instance, std::string* error);

// Appended to the config, data, cache and state directories: "" for the first
// instance, "/instances/<name>" for the others.
std::string dir_suffix();

// " (alt)" for a named instance, "" for the first one.
std::string label();

// The largest name a directory can be told apart by without crowding the others.
inline constexpr std::size_t kMaxNameLength = 64;

}  // namespace stud::instances
