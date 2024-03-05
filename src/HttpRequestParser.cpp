#include "../inc/HttpRequestParser.hpp"

// HttpRequestParser::HttpRequestParser() : content_length_(0) {}

// HttpRequestParser::~HttpRequestParser() {}

// int HttpRequestParser::parseRequest(const std::string &request) {
//     size_t pos = request.find("\r\n\r\n");
//     if (pos != std::string::npos) {
//         std::string headerLines = request.substr(0, pos);
//         body_ = request.substr(pos + 4); // Skip "\r\n\r\n"

//         size_t endOfFirstLine = headerLines.find("\r\n");
//         if (endOfFirstLine != std::string::npos) {
//             std::string firstLine = headerLines.substr(0, endOfFirstLine);
//             std::istringstream iss(firstLine);
//             iss >> method_ >> target_ >> protocol_;

//             parseHeaders(headerLines.substr(endOfFirstLine + 2)); // Skip "\r\n"
//             return parseContentLength();
//         }
//     }
//     return -1; // Parsing failed
// }

// void HttpRequestParser::parseHeaders(const std::string &headerLines) {
//     std::istringstream iss(headerLines);
//     std::string line;
//     while (std::getline(iss, line, '\n')) {
//         size_t pos = line.find(": ");
//         if (pos != std::string::npos) {
//             std::string key = line.substr(0, pos);
//             std::string value = line.substr(pos + 2);
//             headers_[key] = value;
//         }
//     }
// }

// int HttpRequestParser::parseContentLength() {
//     std::map<std::string, std::string>::iterator it = headers_.find("Content-Length");
//     if (it != headers_.end()) {
//         content_length_ = stringToInt(it->second);
//         return 0; // Successfully parsed content length
//     }
//     return -1; // Content-Length header not found
// }

// int HttpRequestParser::stringToInt(const std::string &str) {
//     std::istringstream iss(str);
//     int result;
//     iss >> result;
//     return result;
// }

// int HttpRequestParser::getContentLength() {
//     return content_length_;
// }

// std::string HttpRequestParser::getMethod() const {
//     return method_;
// }

// std::string HttpRequestParser::getTarget() const {
//     return target_;
// }

// std::string HttpRequestParser::getProtocol() const {
//     return protocol_;
// }

// std::map<std::string, std::string> HttpRequestParser::getHeaders() const {
//     return headers_;
// }

// std::string HttpRequestParser::getBody() const {
//     return body_;
// }

// HttpRequestParser.cpp
#include "HttpRequestParser.hpp"
#include "HttpRequestParser.hpp"
#include <iostream>
#include <sstream>
#include <exception> // Added for std::terminate()

HttpRequestParser::HttpRequestParser() : content_length_(0) {}

HttpRequestParser::~HttpRequestParser() {}

int HttpRequestParser::parseRequest(const std::string &request) {
    size_t pos = request.find("\r\n\r\n");
    if (pos != std::string::npos) {
        std::string headerLines = request.substr(0, pos);
        body_ = request.substr(pos + 4); // Skip "\r\n\r\n"

        size_t endOfFirstLine = headerLines.find("\r\n");
        if (endOfFirstLine != std::string::npos) {
            std::string firstLine = headerLines.substr(0, endOfFirstLine);
            std::istringstream iss(firstLine);
            if (!(iss >> method_ >> target_ >> protocol_)) {
                throw std::invalid_argument("Invalid request line format");
            }

            parseHeaders(headerLines.substr(endOfFirstLine + 2)); // Skip "\r\n"
            parseContentLength();

            return 0; // Successfully parsed
        }
    }

    throw std::invalid_argument("Invalid request format");
}

void HttpRequestParser::parseHeaders(const std::string &headerLines) {
    std::istringstream iss(headerLines);
    std::string line;
    while (std::getline(iss, line, '\r')) {
        if (!line.empty()) {
            size_t pos = line.find(": ");
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 2);
                headers_[key] = value;
            }
        }
    }
}

// bool is_token_char(char ch) {
//     // Check if the character is a valid token character
//     return (ch > 31 && ch < 127 && ch != '(' && ch != ')' && ch != '<' &&
//             ch != '>' && ch != '@' && ch != ',' && ch != ';' && ch != ':' &&
//             ch != '\\' && ch != '\"' && ch != '/' && ch != '[' && ch != ']' &&
//             ch != '?' && ch != '=' && ch != '{' && ch != '}' && ch != ' ' && ch != '\t');
// }
// std::string trim(const std::string& str) {
//     size_t start = str.find_first_not_of(" \t");
//     size_t end = str.find_last_not_of(" \t");
   
//     if (start == std::string::npos) {
//         // String is all whitespace
//         return "";
//     }
   
//     return str.substr(start, end - start + 1);
// }
// int HttpRequestParser::parseHeaders() {
//     while (true) {
//         size_t pos = receivedData.find("\r\n");
//         if (pos == std::string::npos) return 0;
//         std::string line = receivedData.substr(0, pos);
//         receivedData = receivedData.substr(pos + 2);
//         if (line.empty()) {
//             status = PREBODY;
//             break;
//         }
//         // Check for continuation lines
//         while (line.size() > 0 && (line[0] == ' ' || line[0] == '\t')) {
//             if (headers_.empty()) {
//                 // If the first line starts with a continuation, it's an error
//                 return 400;
//             }
//             // Remove leading spaces/tabs
//             line = line.substr(1);
//             headers_.rbegin()->second += " " + trim(line);
//         }
//         std::istringstream iss(line);
//         std::string header, value;
//         if (std::getline(iss, header, ':') && std::getline(iss >> std::ws, value)) {
//             // Check for invalid header field names
//             for (char ch : header) {
//                 if (!is_token_char(ch)) {
//                     return 400;
//                 }
//             }
//             // Check for empty header field name
//             if (header.empty()) {
//                 return 400;
//             }
//             // Check for empty header field value
//             if (value.empty()) {
//                 return 400;
//             }
//             // Trim the value
//             value = trim(value);
//             // Check for Content-Length header
//             if (header == "Content-Length") {
//                 try {
//                     int contentLength = std::stoi(value);
//                     if (contentLength < 0) {
//                         return 400; // Invalid negative content length
//                     }
//                 } catch (...) {
//                     return 400; // Invalid content length format
//                 }
//             }
//             headers[header] = value;
//         } else {
//             return 400;
//         }
//     }
//     return 0;
// }


bool is_token_char(char ch) {
        return (ch > 31 && ch < 127 && ch != '(' && ch != ')' && ch != '<' &&
                ch != '>' && ch != '@' && ch != ',' && ch != ';' && ch != ':' &&
                ch != '\\' && ch != '\"' && ch != '/' && ch != '[' && ch != ']' &&
                ch != '?' && ch != '=' && ch != '{' && ch != '}' && ch != ' ' && ch != '\t');
    }

    std::string trim(const std::string &str) {
        size_t start = str.find_first_not_of(" \t");
        size_t end = str.find_last_not_of(" \t");

        if (start == std::string::npos) {
            // String is all whitespace
            return "";
        }

        return str.substr(start, end - start + 1);
    }

    int parseHeaders() {
        while (true) {
            size_t pos = receivedData.find("\r\n");
            if (pos == std::string::npos) return 0;
            std::string line = receivedData.substr(0, pos);
            receivedData = receivedData.substr(pos + 2);
            if (line.empty()) {
                status = PREBODY;
                break;
            }
            // Check for continuation lines
            while (line.size() > 0 && (line[0] == ' ' || line[0] == '\t')) {
                if (headers_.empty()) {
                    // If the first line starts with a continuation, it's an error
                    return 400;
                }
                // Remove leading spaces/tabs
                line = line.substr(1);
                headers_.rbegin()->second += " " + trim(line);
            }
            std::istringstream iss(line);
            std::string header, value;
            if (std::getline(iss, header, ':') && std::getline(iss >> std::ws, value)) {
                // Check for invalid header field names
                for (char ch : header) {
                    if (!is_token_char(ch)) {
                        return 400;
                    }
                }
                // Check for empty header field name
                if (header.empty()) {
                    return 400;
                }
                // Check for empty header field value
                if (value.empty()) {
                    return 400;
                }
                // Trim the value
                value = trim(value);
                // Check for Content-Length header
                if (header == "Content-Length") {
                    try {
                        int contentLength = std::stoi(value);
                        if (contentLength < 0) {
                            return 400; // Invalid negative content length
                        }
                    } catch (...) {
                        return 400; // Invalid content length format
                    }
                }
                headers_[header] = value;
            } else {
                return 400;
            }
        }
        return 0;
    }


void HttpRequestParser::parseContentLength() {
    std::map<std::string, std::string>::const_iterator it = headers_.find("Content-Length");
    if (it != headers_.end()) {
        std::istringstream iss(it->second);
        if (!(iss >> content_length_)) {
            throw std::invalid_argument("Invalid Content-Length value");
        }
    }
}

std::string HttpRequestParser::getMethod() const {
    return method_;
}

std::string HttpRequestParser::getTarget() const {
    return target_;
}

std::string HttpRequestParser::getProtocol() const {
    return protocol_;
}

std::map<std::string, std::string> HttpRequestParser::getHeaders() const {
    return headers_;
}

std::string HttpRequestParser::getBody() const {
    return body_;
}

int HttpRequestParser::getContentLength() const {
    return content_length_;
}

bool HttpRequestParser::isValidMethod(const std::string &method) {
    static const std::set<std::string> validMethods = {
        "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH", "CONNECT", "TRACE"
    };

    return validMethods.find(method) != validMethods.end();
}

void HttpRequestParser::printRequest(const std::string& request) {
    HttpRequestParser parser;
    try {
        parser.parseRequest(request);
        std::cout << "Method: " << parser.getMethod() << std::endl;
        std::cout << "Target: " << parser.getTarget() << std::endl;
        std::cout << "Protocol: " << parser.getProtocol() << std::endl;
        std::cout << "Headers:" << std::endl;
        const std::map<std::string, std::string>& headers = parser.getHeaders();
        std::map<std::string, std::string>::const_iterator it;
        for (it = headers.begin(); it != headers.end(); ++it) {
            std::cout << it->first << ": " << it->second << std::endl;
        }
        std::cout << "Body: " << parser.getBody() << std::endl;
        std::cout << "Content-Length: " << parser.getContentLength() << std::endl;
        std::cout << "-------------------------------\n";
    } catch (const std::exception& e) {
        std::cerr << "Error parsing request: " << e.what() << std::endl;
    }
}









 bool isAlpha(char c) {
        return std::isalpha(c) != 0; // Checks if character is alphabetic
    }

    bool isDigit(char c) {
        return std::isdigit(c) != 0; // Checks if character is a digit
    }

    bool isAlphaNum(char c) {
        return std::isalnum(c) != 0; // Checks if character is alphanumeric
    }

    bool isUnreserved(char c) {
        return isAlphaNum(c) || c == '-' || c == '.' || c == '_' || c == '~';
    }

    bool isSubDelim(char c) {
        return c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' ||
               c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
    }

    bool isPctEncoded(const std::string &str, size_t index) {
        return index + 2 < str.length() &&
               str[index] == '%' &&
               std::isxdigit(str[index + 1]) &&
               std::isxdigit(str[index + 2]);
    }

    bool isValidScheme(const std::string &scheme) {
        if (scheme.empty() || !isAlpha(scheme[0])) {
            return false; // Scheme must start with a letter
        }
        return std::all_of(scheme.begin(), scheme.end(), [&](char c) {
            return isAlphaNum(c) || c == '+' || c == '-' || c == '.';
        });
    }

    bool isValidAuthority(const std::string &authority) {
        size_t posUserInfo = authority.find('@');
        std::string userinfo, host, port;
        if (posUserInfo != std::string::npos) {
            userinfo = authority.substr(0, posUserInfo);
            host = authority.substr(posUserInfo + 1);
        } else {
            host = authority;
        }

        size_t posPort = host.find(':');
        if (posPort != std::string::npos) {
            port = host.substr(posPort + 1);
            host = host.substr(0, posPort);
        }

        return std::all_of(userinfo.begin(), userinfo.end(), [&](char c) {
                   return isUnreserved(c) || isSubDelim(c) || c == ':';
               }) &&
               std::all_of(host.begin(), host.end(), [&](char c) {
                   return isUnreserved(c) || isSubDelim(c);
               }) &&
               std::all_of(port.begin(), port.end(), [&](char c) {
                   return isDigit(c);
               });
    }

    bool isValidPath(const std::string &path) {
        return std::all_of(path.begin(), path.end(), [&](char c) {
            return isUnreserved(c) || isSubDelim(c) || c == ':' || c == '@' || c == '/';
        });
    }

    bool isValidQuery(const std::string &query) {
        return std::all_of(query.begin(), query.end(), [&](char c) {
            return isUnreserved(c) || isSubDelim(c) || c == ':' || c == '@' || c == '/' || c == '?';
        });
    }

    bool isValidFragment(const std::string &fragment) {
        return std::all_of(fragment.begin(), fragment.end(), [&](char c) {
            return isUnreserved(c) || isSubDelim(c) || c == ':' || c == '@' || c == '/' || c == '?';
        });
    }

    bool isValidURI(const std::string &uri) {
        size_t posScheme = uri.find(':');
        if (posScheme == std::string::npos) {
            return false; // Missing scheme
        }

        std::string scheme = uri.substr(0, posScheme);
        if (!isValidScheme(scheme)) {
            return false; // Invalid scheme
        }

        size_t posAuthority = uri.find("//", posScheme + 1);
        if (posAuthority != std::string::npos) {
            size_t posEndAuthority = uri.find('/', posAuthority + 2);
            std::string authority = (posEndAuthority != std::string::npos) ? uri.substr(posAuthority + 2, posEndAuthority - (posAuthority + 2))
                                                                             : uri.substr(posAuthority + 2);

            if (!isValidAuthority(authority)) {
                return false; // Invalid authority
            }

            size_t posPath = posAuthority + authority.length() + 2;
            size_t posEndPath = uri.find('?', posPath);
            if (posEndPath == std::string::npos) {
                posEndPath = uri.find('#', posPath);
            }
            std::string path = (posEndPath != std::string::npos) ? uri.substr(posPath, posEndPath - posPath)
                                                                 : uri.substr(posPath);

            // Decoding percent-encoded characters in the path
            std::string decodedPath;
            for (size_t i = 0; i < path.size(); ++i) {
                if (isPctEncoded(path, i)) {
                    std::string hex = path.substr(i + 1, 2);
                    char decodedChar = static_cast<char>(std::stoi(hex, nullptr, 16));
                    decodedPath.push_back(decodedChar);
                    i += 2;
                } else {
                    decodedPath.push_back(path[i]);
                }
            }

            if (!isValidPath(decodedPath)) {
                return false; // Invalid path
            }

            if (posEndPath != std::string::npos && posEndPath < uri.length() - 1) {
                std::string query = uri.substr(posEndPath + 1);
                size_t posFragment = query.find('#');
                std::string fragment = (posFragment != std::string::npos) ? query.substr(posFragment + 1)
                                                                           : query;

                // Decoding percent-encoded characters in the query and fragment
                std::string decodedQuery;
                for (size_t i = 0; i < query.size(); ++i) {
                    if (isPctEncoded(query, i)) {
                        std::string hex = query.substr(i + 1, 2);
                        char decodedChar = static_cast<char>(std::stoi(hex, nullptr, 16));
                        decodedQuery.push_back(decodedChar);
                        i += 2;
                    } else {
                        decodedQuery.push_back(query[i]);
                    }
                }

                if (!isValidQuery(decodedQuery)) {
                    return false; // Invalid query
                }

                std::string decodedFragment;
                for (size_t i = 0; i < fragment.size(); ++i) {
                    if (isPctEncoded(fragment, i)) {
                        std::string hex = fragment.substr(i + 1, 2);
                        char decodedChar = static_cast<char>(std::stoi(hex, nullptr, 16));
                        decodedFragment.push_back(decodedChar);
                        i += 2;
                    } else {
                        decodedFragment.push_back(fragment[i]);
                    }
                }

                if (!isValidFragment(decodedFragment)) {
                    return false; // Invalid fragment
                }
            }
        } else {
            return false; // Missing authority
        }

        return true;
    }





    int HttpRequestParser::parsePreBody() {

//         parsePreBody Method:
// This method is responsible for parsing the initial part of the HTTP request before the body.
// It checks for the presence of required headers like "Host" and verifies their format.
// If the request uses chunked transfer encoding, it sets the parser's status to CHUNK.
// If the request specifies a Content-Length, it sets the parser's status to BODY and determines the length of the body.
// It also ensures that the HTTP method is either "POST" or "PUT".
// Returns:
// 0 on success.
// 1 if the method is not "POST" or "PUT".
// 400 for various error conditions, such as missing or invalid headers.
    bodyOffset = 0;

    if (headers.find("Host") == headers.end() || headers["Host"].empty()) {
        return 400;
    }

    if (headers["Host"].find("@") != std::string::npos) {
        return 400;
    }

    if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked") {
        status = CHUNK;
        chunkStatus = CHUNK_SIZE;
    } else if (headers.find("Content-Length") != headers.end()) {
        if (headers["Content-Length"].find_first_not_of("0123456789") != std::string::npos) {
            return 400;
        }
        try {
            length = std::stoi(headers["Content-Length"]);
            if (length < 0) {
                throw std::invalid_argument("negative content-length");
            }
        }
        catch (std::exception &e) {
            return 400;
        }
        status = BODY;
    } else {
        return 1;
    }

    if (method != "POST" && method != "PUT") {
        return 1;
    }

    return 0;
}

int HttpRequestParser::parseChunkTrailer() {
//     This method is used when parsing a chunked HTTP request.
// It parses the headers of a chunk trailer, which occur after the chunk data.
// It reads lines until it finds an empty line, indicating the end of the trailer.
// Each header line is split into a key-value pair and added to the headers map.
// Returns:
// 0 on success.
// 400 if a header line is not in the expected format.
    while (true) {
        size_t pos = receivedData.find("\r\n");
        if (pos == std::string::npos) return 0;

        std::string line = receivedData.substr(0, pos);
        receivedData = receivedData.substr(pos + 2);

        if (line.empty()) {
            break;
        }

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string header = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            headers[header] = trim(value);
        } else {
            return 400;
        }
    }
    return 1;
}

int HttpRequestParser::parseChunk() {
// This method is used during chunked transfer encoding to parse individual chunks of the request body.
// It reads chunk data and processes it according to the chunked transfer encoding format.
// It switches between reading the chunk size and the chunk body.
// When the chunk size is read, it converts the hexadecimal chunk size to an integer.
// When the chunk body is read, it appends the data to requestBody and resets chunkSize to prepare for the next chunk.
// Returns:
// 0 if the end of the chunked data is not reached yet.
// 1 when the entire chunked data is parsed.
    while (true) {
        size_t pos = receivedData.find("\r\n");
        if (pos == std::string::npos) return 0;

        std::string line = receivedData.substr(0, pos);
        receivedData = receivedData.substr(pos + 2);

        if (chunkStatus == CHUNK_SIZE) {
            std::string hex = line;
            chunkSize = toHex(hex);
            chunkStatus = CHUNK_BODY;
        } else if (chunkStatus == CHUNK_BODY) {
            if (chunkSize == 0) {
                if (!receivedData.empty()) {
                    return parseChunkTrailer();
                }
                return 1;
            }
            requestBody += line;
            chunkSize = 0;
            chunkStatus = CHUNK_SIZE;
        }
    }
}

int HttpRequestParser::parseChunkTrailer() {
//     This method is used when parsing a chunked HTTP request.
// It parses the headers of a chunk trailer, which occur after the chunk data.
// It reads lines until it finds an empty line, indicating the end of the trailer.
// Each header line is split into a key-value pair and added to the headers map.
// Returns:
// 0 on success.
// 400 if a header line is not in the expected format.
    while (true) {
        size_t pos = receivedData.find("\r\n");
        if (pos == std::string::npos) return 0;

        std::string line = receivedData.substr(0, pos);
        receivedData = receivedData.substr(pos + 2);

        if (line.empty()) {
            break;
        }

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string header = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            headers[header] = trim(value);
        } else {
            return 400;
        }
    }
    return 1;
}

int HttpRequestParser::parseChunk() {
// This method is used during chunked transfer encoding to parse individual chunks of the request body.
// It reads chunk data and processes it according to the chunked transfer encoding format.
// It switches between reading the chunk size and the chunk body.
// When the chunk size is read, it converts the hexadecimal chunk size to an integer.
// When the chunk body is read, it appends the data to requestBody and resets chunkSize to prepare for the next chunk.
// Returns:
// 0 if the end of the chunked data is not reached yet.
// 1 when the entire chunked data is parsed.
    while (true) {
        size_t pos = receivedData.find("\r\n");
        if (pos == std::string::npos) return 0;

        std::string line = receivedData.substr(0, pos);
        receivedData = receivedData.substr(pos + 2);

        if (chunkStatus == CHUNK_SIZE) {
            std::string hex = line;
            chunkSize = toHex(hex);
            chunkStatus = CHUNK_BODY;
        } else if (chunkStatus == CHUNK_BODY) {
            if (chunkSize == 0) {
                if (!receivedData.empty()) {
                    return parseChunkTrailer();
                }
                return 1;
            }
            requestBody += line;
            chunkSize = 0;
            chunkStatus = CHUNK_SIZE;
        }
    }
}

int HttpRequestParser::parseBody() {
// This method is used for non-chunked encoding to parse the request body.
// It checks if the length of the received data is equal to the expected length specified in the Content-Length header.
// If the length matches, it extracts the body from the received data and sets it to requestBody.
// Returns:
// 0 if there is not enough data for the body.
// 1 when the body is successfully parsed.
    std::cout << "Parsing Body. Length: " << length << ", Received: " << receivedData.length() << std::endl;
    std::cout << "Received Data: " << receivedData << std::endl;
    
    if (receivedData.length() >= length) {
        requestBody = receivedData.substr(0, length);
        receivedData = receivedData.substr(length);

        if (requestBody.length() == length) {
            std::cout << "Body parsed successfully." << std::endl;
            std::cout << "Request Body: " << requestBody << std::endl;
            return 1;
        } else {
            std::cout << "Body length mismatch." << std::endl;
            return 400;
        }
    }

    std::cout << "Not enough data for body." << std::endl;
    return 0;
}

int HttpRequestParser::toHex(const std::string &hexStr) {
    std::stringstream ss;
    ss << std::hex << hexStr;
    int result;
    ss >> result;
    return result;
}


// ILLUSTRATION on how to couple preBody etc...
// // Assume receivedData contains the entire HTTP request data

// // Step 1: Parse initial part of the request
// int result = parsePreBody();
// if (result == 0) {
//     // Step 2: Parsing chunked transfer if needed
//     if (status == CHUNK) {
//         result = parseChunk();
//         if (result == 1) {
//             // All chunks parsed, proceed to next step
//             result = parseChunkTrailer();
//             if (result != 0) {
//                 // Handle error in parsing chunk trailer
//             }
//         } else if (result != 0) {
//             // Handle error in parsing chunk
//         }
//     }
//     // Step 3: Parsing non-chunked request body
//     else if (status == BODY) {
//         result = parseBody();
//         if (result == 1) {
//             // Request body parsed successfully
//         } else if (result == 0) {
//             // Not enough data for the body, wait for more data
//         } else {
//             // Error in parsing body, handle accordingly
//         }
//     }
// } else if (result == 1) {
//     // Method is not "POST" or "PUT"
// } else {
//     // Handle other errors from parsePreBody()
// }

void HttpRequestParser::parseQueryString(const std::string &queryString) {
    std::istringstream iss(queryString);
    std::string token;
    while (std::getline(iss, token, '&')) {
        size_t pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            queryParameters[key] = value;
        }
    }
}

void HttpRequestParser::parseCookies(const std::string &cookieHeader) {
    std::istringstream iss(cookieHeader);
    std::string token;
    while (std::getline(iss, token, ';')) {
        size_t pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            cookies[key] = value;
        }
    }
}

void HttpRequestParser::parseContentEncoding(const std::string &encodingHeader) {
    std::istringstream iss(encodingHeader);
    std::string token;
    while (std::getline(iss, token, ',')) {
        contentEncodings.push_back(trim(token));
    }
}

void HttpRequestParser::parseTransferEncoding(const std::string &encodingHeader) {
    std::istringstream iss(encodingHeader);
    std::string token;
    while (std::getline(iss, token, ',')) {
        transferEncodings.push_back(trim(token));
    }
}
