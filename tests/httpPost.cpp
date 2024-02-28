
#include "../inc/HttpRequestParser.hpp"
#include <iostream>

void testHttpPostRequests() {
    // Form URL Encoded Data
    std::string formUrlEncodedRequest =
        "POST /submit-form HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "\r\n"
        "username=johndoe&password=secret123";

    // JSON Data
    std::string jsonRequest =
        "POST /api/users HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "\r\n"
        "{\n"
        "  \"name\": \"John Doe\",\n"
        "  \"email\": \"johndoe@example.com\",\n"
        "  \"age\": 30,\n"
        "  \"active\": true\n"
        "}";
    std::string nestedJsonRequest =
        "POST /api/nested-data HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "\r\n"
        "{\n"
        "  \"name\": \"John Doe\",\n"
        "  \"email\": \"johndoe@example.com\",\n"
        "  \"age\": 30,\n"
        "  \"active\": true,\n"
        "  \"address\": {\n"
        "    \"street\": \"123 Main St\",\n"
        "    \"city\": \"Anytown\",\n"
        "    \"zipcode\": \"12345\"\n"
        "  }\n"
        "}";


    // Multipart Form Data
    std::string multipartFormDataRequest =
        "POST /upload-file HTTP/1.1\r\n"
        "Host: upload.example.com\r\n"
        "Content-Type: multipart/form-data; boundary=---------------------------1234567890\r\n"
        "\r\n"
        "-----------------------------1234567890\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"example.txt\"\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "(contents of the file)\r\n"
        "-----------------------------1234567890--";

    // XML Data
    std::string xmlRequest =
        "POST /api/xml-data HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/xml\r\n"
        "\r\n"
        "<user>\n"
        "  <name>John Doe</name>\n"
        "  <email>johndoe@example.com</email>\n"
        "  <age>30</age>\n"
        "  <active>true</active>\n"
        "</user>";

    // Binary Data (Image Upload)
    std::string binaryDataRequest =
        "POST /upload-image HTTP/1.1\r\n"
        "Host: images.example.com\r\n"
        "Content-Type: image/jpeg\r\n"
        "Content-Length: (size)\r\n"
        "\r\n"
        "(binary image data)";

    HttpRequestParser parser;

    std::cout << "Testing Form URL Encoded Data:\n";
    parser.parseRequest(formUrlEncodedRequest);
    std::cout << "-------------------------------\n";

    std::cout << "Testing JSON Data:\n";
    parser.parseRequest(jsonRequest);
    std::cout << "-------------------------------\n";

    parser.parseRequest(nestedJsonRequest);
    std::cout << "-------------------------------\n";

    std::cout << "Testing Multipart Form Data:\n";
    parser.parseRequest(multipartFormDataRequest);
    std::cout << "-------------------------------\n";

    std::cout << "Testing XML Data:\n";
    parser.parseRequest(xmlRequest);
    std::cout << "-------------------------------\n";

    std::cout << "Testing Binary Data (Image Upload):\n";
    parser.parseRequest(binaryDataRequest);
    std::cout << "-------------------------------\n";
}

