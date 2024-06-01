/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResposeMethods.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: doduwole <doduwole@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/01 10:25:18 by doduwole          #+#    #+#             */
/*   Updated: 2024/06/01 10:39:32 by doduwole         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/HttpResponse.hpp"

pthread_mutex_t g_write;

int HttpResponse::GET()
{ 
    pthread_mutex_lock(&g_write);
    if (!file_)
    {
        std::cerr << "File not found" << std::endl;
        pthread_mutex_unlock(&g_write);
        return 500;
    }

    if (config_.getAutoIndex() && file_->is_directory())
    {
        headers_["Content-Type"] = "text/html; charset=UTF-8";
        body_ = file_->listDir(config_.getRequestTarget());
    }
    else
    {
        std::string mimeType = file_->getMimeType(file_->getMimeExt());
        if (mimeType.empty())
            mimeType = "application/octet-stream";
        headers_["Content-Type"] = mimeType;

        if (!charset_.empty())
            headers_["Content-Type"] += "; charset=" + charset_;

        body_ = file_->getContent();
    }
    headers_["Content-Length"] = ftos(body_.length());
    headers_["Cache-Control"] = "no-cache";
    pthread_mutex_unlock(&g_write);

    return 200;
}

std::string extractFilename(const std::string& body) {
    size_t filenamePos = body.find("filename=\"");
    if (filenamePos != std::string::npos) {
        filenamePos += 10;

        size_t closingQuotePos = body.find("\"", filenamePos);
        if (closingQuotePos != std::string::npos) {
            return body.substr(filenamePos, closingQuotePos - filenamePos);
        }
    }
    return "";
}

std::string extractBoundary(const std::string& contentType) {
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        return "";
    }
    return contentType.substr(boundaryPos + 9);
}

bool containsBoundary(const std::string& input) {
    return input.find("boundary") != std::string::npos;
}

std::string extractContent(const std::string& body, const std::string& boundary) {
    std::string boundaryLine = "--" + boundary;
    std::string endBoundaryLine = boundaryLine + "--";

    size_t boundaryStart = body.find(boundaryLine);
    if (boundaryStart == std::string::npos) {
        return "";
    }
    boundaryStart += boundaryLine.length() + 2;

    size_t headerEnd = body.find("\r\n\r\n", boundaryStart);
    if (headerEnd == std::string::npos) {
        return "";
    }
    headerEnd += 4;

    size_t contentEnd = body.find(boundaryLine, headerEnd);
    if (contentEnd == std::string::npos) {
        contentEnd = body.find(endBoundaryLine, headerEnd);
    }
    if (contentEnd == std::string::npos) {
        return "";
    }
    contentEnd -= 2;

    return body.substr(headerEnd, contentEnd - headerEnd);
}

int HttpResponse::POST() {
    int status_code = 500;
    body_ = config_.getBody();

    pthread_mutex_lock(&g_write);
    if (!file_->exists()) {
        file_->createFile(body_);
        status_code = 201;
    } else {
        MimeTypes mimeTypes;
        std::string contentType = config_.getHeader("content-type");
        bool isMultipart = containsBoundary(contentType);
        std::string boundary = isMultipart ? extractBoundary(contentType) : contentType;

        if (isMultipart && !boundary.empty()) {
            std::string filename = extractFilename(body_);
            std::string fileContent = extractContent(body_, boundary);
            body_.clear();
            body_.append(fileContent);
            if (!filename.empty() && !fileContent.empty()) {
                file_->appendFile(fileContent, filename);
                status_code = 201;
            } else {
                status_code = 400;
            }
        } else {
            std::cerr << "Invalid content type or boundary not found" << std::endl;
            status_code = 415;
        }
    }
    pthread_mutex_unlock(&g_write);

    headers_["Content-Length"] = ftos(body_.length());

    if (!file_->getFilePath().empty()) {
        headers_["Location"] = file_->getFilePath();
    }

    return status_code;
}


int HttpResponse::PUT()
{
    pthread_mutex_lock(&g_write);

    int status_code = 204;

    if (!file_)
    {
        std::cerr << "File not found" << std::endl;
        pthread_mutex_unlock(&g_write);
        return 500;
    }

    if (file_->exists())
    {
        file_->createFile(config_.getBody());
    }
    else
    {
        file_->createFile(config_.getBody());
        if (!file_->exists())
        {
            std::cerr << "Failed to create file" << std::endl;
            pthread_mutex_unlock(&g_write);
            return 500;
        }
        headers_["Content-Length"] = "0";
        status_code = 201;
    }
    pthread_mutex_unlock(&g_write);
    return status_code;
}

int HttpResponse::DELETE()
{
    pthread_mutex_lock(&g_write);

    int status_code = 200;

    if (!file_)
    {
        std::cerr << "File not found" << std::endl;
        status_code = 500;
    }
    else
    {
        if (!file_->exists())
        {
            std::cerr << "File does not exist" << std::endl;
            status_code = 404;
        }
        else
        {
            if (file_->deleteFile())
            {
                headers_["Content-Length"] = "0";
                status_code = 200;
            }
            else
            {
                status_code = 500;
            }
        }
    }

    pthread_mutex_unlock(&g_write);
    std::string header = "<!DOCTYPE html>\n\
                 <html>\n\
                 <body>\n";

    std::string footer = "</body>\n\
                 </html>";
    body_.append(header);
    if (status_code == 200)
        body_.append("<h1>File deleted</h1>\n");
    else
        body_ = (status_code == 404)
                    ? body_.append("<h1>File not found</h1>\n")
                    : body_.append("<h1>Internal Server Error</h1>\n");
    body_.append(footer);
    headers_["Content-Type"] = "text/html";
    headers_["Content-Length"] = ftos(body_.length());

    return status_code;
}
