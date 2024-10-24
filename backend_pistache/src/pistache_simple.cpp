#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/net.h>
#include <sstream>
#include <set>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace Pistache;

std::string echo_parameter(const std::string& message) {
    return message;
}

Rest::Route::Result request_as_json(const Rest::Request& req, Http::ResponseWriter response) {
    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    doc.AddMember("method", rapidjson::Value(methodString(req.method()), allocator).Move(), allocator);
    doc.AddMember("body", rapidjson::Value(req.body().c_str(), allocator).Move(), allocator);

    rapidjson::Value params_dict(rapidjson::kObjectType);
    const auto& query = req.query();
    auto params = query.parameters();
    for (const auto& key : params) {
        params_dict.AddMember(rapidjson::Value(key.c_str(), allocator).Move(), rapidjson::Value(query.get(key)->c_str(), allocator).Move(), allocator);
    }
    doc.AddMember("params", params_dict, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    response.send(Http::Code::Ok, buffer.GetString(), MIME(Application, Json));
    return Rest::Route::Result::Ok;
}

std::string pistache_version_to_string(const Http::Version& ver)
{
    switch (ver)
    {
        case Http::Version::Http10:
            return "HTTP/1.0";
        case Http::Version::Http11:
            return "HTTP/1.1";
        default:
            return "Unknown";
    }
}


Rest::Route::Result request_as_html(const Rest::Request& req, Http::ResponseWriter response) {
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
    os << "<p><strong>Method:</strong> " << req.method() << "</p>";
    os << "<p><strong>Body:</strong> " << req.body() << "</p>";
    os << "<p><strong>Remote IP:</strong> " << req.address().host() << "</p>";
    os << "<p><strong>HTTP version:</strong> " << pistache_version_to_string(req.version()) << "</p>";
    os << "<p><strong>Headers:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    for (const auto& header : req.headers().rawList()) {
        os << "<tr><td>" << header.second.name() << "</td><td>" << header.second.value() << "</td></tr>";
    }
    os << "</tbody></table>";
    os << "<p><strong>URL:</strong> " << req.resource() << "</p>";
    os << "<p><strong>URL params:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    const auto& query = req.query();
    for (const auto& key : query.parameters()) {
        os << "<tr><td>" << key << "</td><td>" << *query.get(key) << "</td></tr>";
    }
    os << "</tbody></table>";

    response.send(Http::Code::Ok, os.str(), MIME(Text, Html));
    return Rest::Route::Result::Ok;
}

int main() {
    Http::Endpoint server(Address(Ipv4::any(), Port(18100)));
    auto opts = Http::Endpoint::options().threads(1);
    server.init(opts);

    Rest::Router router;

    Rest::Routes::Get(router, "/", [](const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "Hello world");
        return Rest::Route::Result::Ok;
    });

    Rest::Routes::Get(router, "/echo/:message", [](const Rest::Request& req, Http::ResponseWriter response) {
        auto message = req.param(":message").as<std::string>();
        response.send(Http::Code::Ok, echo_parameter(message));
        return Rest::Route::Result::Ok;
    });

    Rest::Routes::Get(router, "/request", request_as_json);
    Rest::Routes::Post(router, "/request", request_as_json);

    Rest::Routes::Get(router, "/info", request_as_html);
    Rest::Routes::Post(router, "/info", request_as_html);

    server.setHandler(router.handler());
    server.serve();
}