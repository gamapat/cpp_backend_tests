#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include <set>
#include <sstream>

// Begin DTOs
#include OATPP_CODEGEN_BEGIN(DTO)

class RequestInfoDto : public oatpp::DTO {
    DTO_INIT(RequestInfoDto, DTO)
    
    DTO_FIELD(String, method);
    DTO_FIELD(String, body);
    DTO_FIELD(Object<oatpp::Fields<oatpp::Any>>, params);
};

#include OATPP_CODEGEN_END(DTO)

// Controller definition
class MyController : public oatpp::web::server::api::ApiController {
public:
    typedef MyController __ControllerType;
    
    MyController(OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper)
    {}
    
    static std::shared_ptr<MyController> createShared(
        OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper)
    ) {
        return std::make_shared<MyController>(objectMapper);
    }
    
    ENDPOINT("GET", "/", root) {
        return createResponse(Status::CODE_200, "Hello world");
    }
    
    ENDPOINT("GET", "/echo/{message}", echo, PATH(String, message)) {
        return createResponse(Status::CODE_200, message);
    }
    
    ENDPOINT("GET", "/request", requestAsJson, REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        auto dto = RequestInfoDto::createShared();
        dto->method = request->getMethod();
        dto->body = request->readBodyToString();
        
        // Process query parameters
        auto params = oatpp::Fields<oatpp::Any>::createShared();
        auto queryParams = request->getQueryParameters();
        
        for (const auto& param : *queryParams) {
            params->put(param.first.toString(), oatpp::String(param.second.toString()));
        }
        
        dto->params = params;
        
        return createDtoResponse(Status::CODE_200, dto);
    }
    
    ENDPOINT("GET", "/info", requestAsHtml, REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        std::stringstream os;
        
        os << "<style>"
             << "body { padding-left: 20px; padding-right: 20px; }"
             << "table { border-collapse: collapse; width: 100%; border: 1px solid #ddd; }"
             << "th, td { padding: 8px; text-align: left; }"
             << "th { border-bottom: 1px solid #ddd; }"
             << "tr:nth-child(even) { background-color: #f2f2f2; }"
             << "tr:nth-child(odd) { background-color: #fff; }"
             << "</style>";
        
        os << "<h1>Request info</h1>";
        os << "<p><strong>Method:</strong> " << request->getMethod() << "</p>";
        os << "<p><strong>Body:</strong> " << request->readBodyToString() << "</p>";
        os << "<p><strong>Remote IP:</strong> " << request->getConnection()->getRemoteAddress() << "</p>";
        
        os << "<p><strong>Headers:</strong></p><table>"
             << "<thead><tr><th>Key</th><th>Value</th></tr></thead><tbody>";
        
        for (const auto& header : *request->getHeaders()) {
            os << "<tr><td>" << header.first.toString() << "</td><td>" << header.second.toString() << "</td></tr>";
        }
        
        os << "</tbody></table>";
        os << "<p><strong>URL:</strong> " << request->getUrl() << "</p>";
        os << "<p><strong>URL params:</strong></p><table>"
             << "<thead><tr><th>Key</th><th>Value</th></tr></thead><tbody>";
        
        for (const auto& param : *request->getQueryParameters()) {
            os << "<tr><td>" << param.first.toString() << "</td><td>" << param.second.toString() << "</td></tr>";
        }
        
        os << "</tbody></table>";
        
        auto response = createResponse(Status::CODE_200, os.str());
        response->putHeader("Content-Type", "text/html");
        return response;
    }
};

class AppComponent {
public:
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
        return oatpp::web::server::HttpRouter::createShared();
    }());
    
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpConnectionHandler>, httpConnectionHandler)([] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
        return oatpp::web::server::HttpConnectionHandler::createShared(router);
    }());
    
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
        return oatpp::network::tcp::server::ConnectionProvider::createShared({"localhost", 18080, oatpp::network::Address::IP_4});
    }());
    
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
        return oatpp::parser::json::mapping::ObjectMapper::createShared();
    }());
};

int main() {
    oatpp::base::Environment::init();
    
    AppComponent components;
    
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
    auto myController = MyController::createShared();
    myController->addEndpointsToRouter(router);
    
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpConnectionHandler>, connectionHandler);
    
    oatpp::network::Server server(connectionProvider, connectionHandler);
    
    OATPP_LOGI("MyApp", "Server running on port 18080");
    
    server.run();
    
    oatpp::base::Environment::destroy();
    
    return 0;
}