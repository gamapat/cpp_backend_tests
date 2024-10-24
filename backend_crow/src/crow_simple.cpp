#include "crow.h"
#include <set>

std::string echo_parameter(const std::string& message){
    return message;
}

crow::json::wvalue request_as_json(const crow::request& req) {
    crow::json::wvalue x;
    x["method"] = crow::method_name(req.method);
    x["body"] = req.body;
    crow::json::wvalue params_dict;
    auto params_keys = req.url_params.keys();
    auto param_keys_uniq = std::set(params_keys.begin(), params_keys.end());
    for(const auto& key : param_keys_uniq)
    {
        auto value_list = req.url_params.get_list(key, false);
        if (value_list.size() == 1)
        {
            params_dict[key] = value_list[0];
        }
        else
        {
            crow::json::wvalue::list list;
            for(auto& value : value_list)
            {
                list.push_back(crow::json::wvalue(value));
            }
            params_dict[key] = std::move(list);
        }
    }
    x["params"] = std::move(params_dict);
    return x;
}

std::string request_as_html(const crow::request& req) {
    std::ostringstream os;
    os << "<style>";
    os << "body { padding-left: 20px; padding-right: 20px; }";
    os << "table { border-collapse: collapse; width: 100%; border: 1px solid #ddd; }";
    os << "th, td { padding: 8px; text-align: left; }";
    os << "th { border-bottom: 1px solid #ddd; }";
    os << "tr:nth-child(even) { background-color: #f2f2f2; }";
    os << "tr:nth-child(odd) { background-color: #fff; }";
    os << "</style>";

    os << "<h1>Request info</h1>";
    os << "<p><strong>Method:</strong> " << crow::method_name(req.method) << "</p>";
    os << "<p><strong>Body:</strong> " << req.body << "</p>";
    os << "<p><strong>Remote IP:</strong> " << req.remote_ip_address << "</p>";
    os << "<p><strong>HTTP version:</strong> " << (int)req.http_ver_major << "." << (int)req.http_ver_minor << "</p>";
    os << "<p><strong>Keep alive:</strong> " << (req.keep_alive ? "yes" : "no") << "</p>";
    os << "<p><strong>Close connection:</strong> " << (req.close_connection ? "yes" : "no") << "</p>";
    os << "<p><strong>Upgrade:</strong> " << (req.upgrade ? "yes" : "no") << "</p>";
    os << "<p><strong>Headers:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    for(const auto& header : req.headers)
    {
        os << "<tr><td>" << header.first << "</td><td>" << header.second << "</td></tr>";
    }
    os << "</tbody></table>";
    os << "<p><strong>URL:</strong> " << req.url << "</p>";
    os << "<p><strong>URL params:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    auto params_keys = req.url_params.keys();
    auto param_keys_uniq = std::set(params_keys.begin(), params_keys.end());
    for(const auto& key : param_keys_uniq)
    {
        auto value_list = req.url_params.get_list(key, false);
        for(const auto& value : value_list)
        {
        os << "<tr><td>" << key << "</td><td>" << value << "</td></tr>";
        }
    }
    os << "</tbody></table>";
    return os.str();
}

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

    // Echo route
    CROW_ROUTE(app, "/echo/<string>")
    (echo_parameter);

    // Route that will return parameters and body as json
    CROW_ROUTE(app, "/request")
    (request_as_json);

    // Route that will return html page with request parameters, body and headers
    CROW_ROUTE(app, "/info")
    (request_as_html);

    app.port(18080).multithreaded().run();
}