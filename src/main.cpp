#include "application/application.hpp"

#include "logger/logger_macros.hpp"

using namespace application;

int main() {
    Application application;

    if (!application.initialise()) {
        LOG_ERROR("Failed to initialise application");
        return EXIT_FAILURE;
    }

    application.load();

    application.run();

    return EXIT_SUCCESS;
}
