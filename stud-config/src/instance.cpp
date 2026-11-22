#include "stud/instance.h"

#include <algorithm>

namespace {

std::string& name_storage() {
    static std::string name;
    return name;
}

bool is_safe(const std::string& instance) {
    if (instance.empty() || instance.size() > stud::instances::kMaxNameLength) return false;
    // One directory, so no separators; and not a name the filesystem reads as a
    // reference to another directory.
    if (instance == "." || instance == "..") return false;
    return std::all_of(instance.begin(), instance.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
               c == '-' || c == '_' || c == '.';
    });
}

}  // namespace

namespace stud::instances {

const std::string& name() { return name_storage(); }

bool select(const std::string& instance, std::string* error) {
    // The first instance has no name: the paths it has always used.
    if (instance.empty()) {
        name_storage().clear();
        return true;
    }
    if (!is_safe(instance)) {
        *error = "\"" + instance + "\" is not an instance name: use letters, digits, "
                                  "\".\", \"-\" and \"_\", up to 64 of them";
        return false;
    }
    name_storage() = instance;
    return true;
}

std::string dir_suffix() {
    return name_storage().empty() ? std::string() : "/instances/" + name_storage();
}

std::string label() { return name_storage().empty() ? std::string() : " (" + name_storage() + ")"; }

}  // namespace stud::instances
