

#include <Arduino.h>
#include <WiFi.h>

#include <router/router.h>

namespace routes_example
{
    class Router : public EspWeb::Router
    {
    public:
        Router()
        {
            use("/hello", auth());
            route("GET", "/hello", get_hello);
            route("GET", "/status", get_status);
            route("GET", "/example", get_example);
            route("POST", "/data", post_data);
            route("GET", "/echo/:message", get_echo);
            route("GET", "/users/:id/info", get_user_info);
        }

    private:
        static void get_hello(EspWeb::Request &req, EspWeb::Response &res);
        static void get_status(EspWeb::Request &req, EspWeb::Response &res);
        static void get_example(EspWeb::Request &req, EspWeb::Response &res);
        static void post_data(EspWeb::Request &req, EspWeb::Response &res);
        static void get_echo(EspWeb::Request &req, EspWeb::Response &res);
        static void get_user_info(EspWeb::Request &req, EspWeb::Response &res);
    };

}