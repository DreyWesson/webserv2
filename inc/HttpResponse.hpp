#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "./AllHeaders.hpp"
#include "./File.hpp"

class HttpRequest;
class RequestConfig;
class File;

extern pthread_mutex_t g_write;

struct CharsetAndQ {
    std::string charset;
    double qValue;
};

class HttpResponse {
public:
    HttpResponse(RequestConfig &config, int error_code = 100);
    ~HttpResponse();

    typedef int (HttpResponse::*type)();
    void initMethodMap();

    enum LogLevel {
        NONE,
        INFO,
        DEBUG
    };


    void cleanUp();
    int buildErrorPage(int status_code);
    void redirectToErrorPage(const std::string& target, int status_code);
    std::string buildDefaultErrorPage(int status_code);
    void build();
    int handleMethods();
    int handleDirectoryRequest();
    int handleFileRequest();
    int handlePutPostRequest();
    void handleAcceptLanguage(std::vector<std::string> &matches);
    void handleAcceptCharset(std::vector<std::string> &matches);
    int handleOtherMethods();
    void createResponse();
    bool localization(std::vector<std::string> &matches);
    std::string accept_charset(std::vector<std::string> &matches);
    bool getRedirect();
    std::string redirect_target();
    bool shouldDisconnect();
    // std::string response_log(LogLevel level);
    // std::string getCurrentDateTime();
    // void logError(const std::string& message);

    std::pair<std::string, double> extractLangAndQ(const std::string& langAndQ);
    std::string findBestLanguage(const std::vector<std::string>& matches, const std::vector<std::pair<std::string, double> >& langQPairs);

    CharsetAndQ extractCharsetAndQ(const std::string& charsetAndQ);
    std::string handleDefaultCharset(const std::string& bestCharset);
    std::string findBestCharset(const std::vector<CharsetAndQ>& charsetAndQValues, const std::vector<std::string>& matches);
    void buildDebugger (std::string method);

    std::string response_log(LogLevel level);

    int GET();
    int POST();
    int PUT();
    int DELETE();

    int getStatus();
    std::string getResponseBody();
    int sendResponse(int fd);



private:
    RequestConfig &config_;
    File *file_;
    int error_code_;
    int worker_id_;
    size_t total_sent_;
    int status_code_;
    std::string response_;
    std::string body_;
    bool redirect_;
    std::string redirect_target_;
    int redirect_code_;
    size_t header_size_;
    size_t body_size_;
    std::string charset_;
    std::map<std::string, HttpResponse::type> methods_;
    std::map<std::string, std::string> headers_;   

    std::string buildMethodList();
    bool checkAuth();
};

std::string getMimeType(const std::string& ext);
std::string getHttpStatusCode(int code);


#endif