#include "instance_option.h"

namespace stud::ui {

bool take_instance_option(const std::vector<std::string>& arguments, std::optional<std::string>* named,
                          std::vector<std::string>* rest, std::string* error) {
    rest->clear();
    for (std::size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0 && arguments[i] == "--instance") {
            if (i + 1 >= arguments.size()) {
                *error = "--instance needs a name";
                return false;
            }
            if (*named) {
                *error = "--instance is given twice";
                return false;
            }
            *named = arguments[++i];
            continue;
        }
        rest->push_back(arguments[i]);
    }
    return true;
}

}  // namespace stud::ui
