// httpresp.hpp - HTTP Request Generator
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

#pragma once

#include <string>
#include <sstream>
#include <unordered_map>

class HTTPResp {
    const char* header_sep = "\r\n";
    const char* HTTP_VERSION = "1.1";

public:
    HTTPResp(const unsigned int code, const std::string& body = "", const bool keep_alive = true);
    const std::string getResponse();
    const bool isMalformed();

private:
    unsigned int code_;
    bool keep_alive_;
    bool malformed_;
    std::string response_;

    void buildResponse(const std::string& body);
    static const std::unordered_map<unsigned int, std::string> status_phrase;
};

