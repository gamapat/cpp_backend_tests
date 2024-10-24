#include <drogon/drogon.h>
#include <set>

std::string echo_parameter(const std::string &message)
{
    return message;
}

Json::Value request_as_json(const drogon::HttpRequestPtr &req)
{
    Json::Value x;
    x["method"] = req->getMethodString();
    x["body"] = std::string(req->getBody());
    Json::Value params_dict;
    auto params_keys = req->getParameters();
    for (const auto &[key, values] : params_keys)
    {
        params_dict[key] = values;
    }
    x["params"] = params_dict;
    return x;
}

std::string drogon_version_to_string(const drogon::Version& ver)
{
    switch (ver)
    {
        case drogon::Version::kUnknown:
            return "Unknown";
        case drogon::Version::kHttp10:
            return "HTTP/1.0";
        case drogon::Version::kHttp11:
            return "HTTP/1.1";
    }
    return "";
}

std::string request_as_html(const drogon::HttpRequestPtr &req)
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
    os << "<p><strong>Method:</strong> " << req->getMethodString() << "</p>";
    os << "<p><strong>Body:</strong> " << req->getBody() << "</p>";
    os << "<p><strong>Remote IP:</strong> " << req->getPeerAddr().toIp() << "</p>";
    os << "<p><strong>HTTP version:</strong> " << drogon_version_to_string(req->getVersion()) << "</p>";
    os << "<p><strong>Headers:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    for (const auto &header : req->getHeaders())
    {
        os << "<tr><td>" << header.first << "</td><td>" << header.second << "</td></tr>";
    }
    os << "</tbody></table>";
    os << "<p><strong>URL:</strong> " << req->getPath() << "</p>";
    os << "<p><strong>URL params:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    auto params = req->getParameters();
    for (const auto &[key, values] : params)
    {
        os << "<tr><td>" << key << "</td><td>" << values << "</td></tr>";
    }
    os << "</tbody></table>";
    return os.str();
}

int main()
{
    drogon::app().registerHandler("/", [](const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setBody("Hello world");
        callback(resp);
    });

    drogon::app().registerHandler("/echo/{1}", [](const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string message)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setBody(message);
        callback(resp);
    });

    drogon::app().registerHandler("/request", [](const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
    {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(request_as_json(req));
        callback(resp);
    });

    drogon::app().registerHandler("/info", [](const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setContentTypeCode(drogon::CT_TEXT_HTML);
        resp->setBody(request_as_html(req));
        callback(resp);
    });

    drogon::app().addListener("0.0.0.0", 18090).run();
}