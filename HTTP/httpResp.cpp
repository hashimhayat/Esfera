// httpresp.cc - HTTP Request Generator
// (c) 2017 Christopher Mitchell, Ph.D.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "httpResp.h"

const std::unordered_map<unsigned int, std::string> HTTPResp::status_phrase = {
        {200, "OK"},
        {400, "Bad Request"},
        {404, "Not Found"},
        {418, "I'm a teapot"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"}
};

HTTPResp::HTTPResp(const unsigned int code, const std::string& body, const bool keep_alive)
        : code_(code)
        , keep_alive_(keep_alive)
        , malformed_(true)
{
    buildResponse(body);
}

void HTTPResp::buildResponse(const std::string& body) {
    auto status = status_phrase.find(code_);
    if (status == status_phrase.end()) {
        return;
    }

    std::ostringstream response_builder;
    response_builder << "HTTP/" << HTTP_VERSION << ' ' << code_ << ' ' << status->second << header_sep;
    response_builder << "Content-Length: " << body.length() << header_sep;
    response_builder << "Connection: " << (keep_alive_ ? "keep-alive" : "close") << header_sep;
    response_builder << header_sep;
    response_builder << body;
    response_ = response_builder.str();

    malformed_ = false;
}

const std::string HTTPResp::getResponse(void) {
    return response_;
}

const bool HTTPResp::isMalformed(void) {
    return malformed_;
}

