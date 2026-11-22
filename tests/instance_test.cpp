// Instances: which names are allowed, and the separate paths a named one keeps its
// files in, so two can be signed in to two Roblox accounts at once.

#include "check.h"

#include "stud/instance.h"
#include "stud/stud_paths.h"

#include <string>

namespace {

using stud::instances::kMaxNameLength;

void accepts(const std::string& name) {
    std::string error;
    check(stud::instances::select(name, &error), name + " is an instance name");
    check(error.empty(), "accepting it says nothing");
    check(stud::instances::name() == name, "the instance is " + name);
    check(stud::instances::label() == " (" + name + ")", "and is labelled with it");
}

void refuses(const std::string& name) {
    std::string error;
    check(!stud::instances::select(name, &error), "\"" + name + "\" is refused");
    check(!error.empty(), "with a reason to show");
}

}  // namespace

int main() {
    // The first instance: no name, and so no suffix on any path it had before.
    std::string error;
    check(stud::instances::select("", &error), "empty is the first instance");
    check(stud::instances::name().empty(), "which has no name");
    check(stud::instances::dir_suffix().empty(), "and no directory of its own");
    check(stud::instances::label().empty(), "and no label");
    check(stud::paths::data_dir() == stud::paths::shared_data_dir(), "data is the shared directory");

    accepts("alt");
    check(stud::instances::dir_suffix() == "/instances/alt", "a named one gets instances/alt");
    check(stud::paths::data_dir() == stud::paths::shared_data_dir() + "/instances/alt",
          "its data sits beside the first one's");
    check(stud::paths::config_dir().find("/instances/alt") != std::string::npos, "so do its settings");
    check(stud::paths::cache_dir().find("/instances/alt") != std::string::npos, "its cache");
    check(stud::paths::state_dir().find("/instances/alt") != std::string::npos, "its state");
    check(stud::paths::log_dir().find("/instances/alt/logs") != std::string::npos, "its logs");
    check(stud::paths::runtime_dir().find("/instances/alt") != std::string::npos,
          "and its runtime files, so no socket is shared");

    accepts("Second_Account.2");
    accepts(std::string(kMaxNameLength, 'x'));

    // The name is a choice, so it can be made again: empty goes back to the first one.
    check(stud::instances::select("", &error), "back to the first instance");
    check(stud::instances::dir_suffix().empty(), "with the plain paths again");
    accepts("alt");

    // Anything that would leave the instances directory, name another directory, or
    // make two instances share one.
    refuses(std::string(kMaxNameLength + 1, 'x'));
    refuses(".");
    refuses("..");
    refuses("../escape");
    refuses("a/b");
    refuses("with space");
    refuses("tab\there");
    refuses("line\nbreak");
    refuses(std::string("nul\0inside", 10));

    return finish("instance_test");
}
