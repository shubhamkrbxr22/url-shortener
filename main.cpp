#include "crow.h"

int main() {
    crow::SimpleApp app;

    // basic route
    CROW_ROUTE(app, "/")([]() {
        return "Hello Shubham! Server chal raha hai 🚀";
    });

    // sum route
    CROW_ROUTE(app, "/sum/<int>/<int>")
    ([](int a, int b) {
        return std::to_string(a + b);
    });

    app.port(8080).multithreaded().run();
}