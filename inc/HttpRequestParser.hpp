#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <set>
#include <cstdlib>

class HttpRequestParser {
private:
    std::string method_;
    std::string uri_;
    std::string protocol_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    std::string scheme_;
    std::string authority_;
    std::string path_;
    std::string query_;
    std::string frag_;
    bool isChunked_;
    size_t length_;
    int chunk_size_;

    bool extractURIComponents();
    bool isValidScheme(const std::string& scheme);
    bool isValidAuthority(const std::string& authority);
    bool isValidPath(const std::string& path);
    bool isValidQuery(const std::string& query);
    bool isValidFragment(const std::string& fragment);
    bool isAlpha(char c);
    bool isDigit(char c);
    bool isAlphaNum(char c);
    void print_uri_extracts();
    bool isUnreserved(char c);
    bool isSubDelim(char c);
    bool isHexDigit(char c);
    bool isValidIPv6(const std::string& ipv6);
    int isValidProtocol(const std::string& protocol);
    std::string trim(const std::string& str);

    unsigned int hexToDecimal(const std::string& hex);


public:
    HttpRequestParser();

    int parseRequest(const std::string &request);
    int parseHeaders(const std::string &headerLines);
    // void parseChunkedBody(const std::string &chunkedBody);
    std::string parseBody(const std::string& contentType);
    void parseChunkedBody(const std::string& chunkedBody);
    void parseContentLength();
    int parseRequestLine(std::string requestLine);
    bool isMethodCharValid(char ch) const;
    int parseMethod();
    int validateURI();
    int parseRequestLine(std::string headerLines, size_t eofFirstLine);
    int extractRequestLineData(std::string requestLine);

    std::string getMethod() const;
    std::string getURI() const;
    std::string getProtocol() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getBody() const;

    void printRequest(const std::string& request);



    bool validateJson(const std::string& jsonString);
    bool isValidClosingBracket(char opening, char closing);
    std::string extractTag(const std::string& line);
    bool validateXml(const std::string& xmlString);
    bool isValidKeyValuePair(const std::string& pair);
    bool isValidUrlEncoded(const std::string& str);
    bool validateFormData(const std::string& formData);
    bool validatePlainText(const std::string& text);
    bool isPrintableAscii(char c);
    std::vector<std::string> splitMultipartData(const std::string& data, const std::string& boundary);
    bool validateMultipartData(const std::string& data, const std::string& boundary);
    bool  validateBinaryData (std::string data);


};

#endif
