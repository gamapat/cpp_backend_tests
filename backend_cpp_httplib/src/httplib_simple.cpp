#include <httplib.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <set>

std::string echo_parameter(const std::string &message)
{
    return message;
}

nlohmann::json; request_as_json(const httplib::Request &req)
{
    nlohmann::json x;
    x["method"] = req.method;
    x["body"] = req.body;
    nlohmann::json params_dict;
    for (const auto &param : req.params)
    {
        params_dict[param.first] = param.second;
    }
    x["params"] = params_dict;
    return x;
}

std::string request_as_html(const httplib::Request &req)
{
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
    os << "<p><strong>Method:</strong> " << req.method << "</p>";
    os << "<p><strong>Body:</strong> " << req.body << "</p>";
    os << "<p><strong>Remote IP:</strong> " << req.remote_addr << "</p>";
    os << "<p><strong>HTTP version:</strong> " << req.version << "</p>";
    os << "<p><strong>Headers:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    for (const auto &header : req.headers)
    {
        os << "<tr><td>" << header.first << "</td><td>" << header.second << "</td></tr>";
    }
    os << "</tbody></table>";
    os << "<p><strong>URL:</strong> " << req.path << "</p>";
    os << "<p><strong>URL params:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    for (const auto &param : req.params)
    {
        os << "<tr><td>" << param.first << "</td><td>" << param.second << "</td></tr>";
    }
    os << "</tbody></table>";
    return os.str();
}

int main()
{
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request &req, httplib::Response &res) {
        res.set_content("Hello world", "text/plain");
    });

    svr.Get(R"(/echo/(.+))", [](const httplib::Request &req, httplib::Response &res) {
        // Extract the message from the path
        std::string message;
        if (!req.matches.empty() && req.matches.size() > 1) {
            message = req.matches[1];
        }
        res.set_content(echo_parameter(message), "text/plain");
    });

    svr.Get("/request", [](const httplib::Request &req, httplib::Response &res) {
        std::string output = request_as_json(req).dump();
        res.set_content(output, "application/json");
    });

    svr.Get("/info", [](const httplib::Request &req, httplib::Response &res) {
        res.set_content(request_as_html(req), "text/html");
    });

    svr.listen("0.0.0.0", 18090);
}
