// `--instance <name>`: taken out of a launch's arguments wherever it is, and refused
// when it is ambiguous.

#include "check.h"
#include "instance_option.h"

#include <optional>
#include <string>
#include <vector>

namespace {

using stud::ui::take_instance_option;

std::vector<std::string> rest;
std::optional<std::string> named;

std::string run(const std::vector<std::string>& arguments) {
    named.reset();
    rest.clear();
    std::string error;
    const bool ok = take_instance_option(arguments, &named, &rest, &error);
    return ok ? std::string() : error;
}

std::string joined(const std::vector<std::string>& parts) {
    std::string text;
    for (const std::string& part : parts) text += (text.empty() ? "" : " ") + part;
    return text;
}

}  // namespace

int main() {
    // Nothing to take: the launch is the first instance, as before.
    check(run({"stud"}).empty(), "a bare launch is accepted");
    check(!named, "and names no instance");
    check_eq(joined(rest), "stud", "and keeps its arguments");

    // Before or after the launch argument, the rest reads the same.
    check(run({"stud", "--instance", "alt", "--roblox"}).empty(), "the flag is accepted before a launch argument");
    check(named == std::optional<std::string>("alt"), "and names the instance");
    check_eq(joined(rest), "stud --roblox", "and is taken out of the arguments");
    check(run({"stud", "--roblox", "--instance", "alt"}).empty(), "and after one");
    check_eq(joined(rest), "stud --roblox", "with the same result");

    check(run({"stud", "--instance", "alt", "roblox://play?placeId=1"}).empty(), "a deep link keeps its place");
    check_eq(joined(rest), "stud roblox://play?placeId=1", "right after the program name");

    // Ambiguous: refused rather than guessed, so a Stud the user didn't mean never starts.
    check(!run({"stud", "--instance"}).empty(), "a flag with no name is refused");
    check(!run({"stud", "--instance", "alt", "--instance", "alt2"}).empty(), "so is a flag given twice");

    return finish("instance_option_test");
}
