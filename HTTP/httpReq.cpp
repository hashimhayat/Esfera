// httpreq.cc - HTTP Request Parser
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

#include "httpReq.h"

#include <string>
#include <string.h>
#include <iostream>
#include <errno.h>

HTTPReq::HTTPReq(const int sock_fd)
	: sock_fd_(sock_fd),
	  parsed_(false),
	  malformed_(true),
      connection_("keep-alive")
{
}

// This must be called before any of the accessors are used.
// Returns 0 if the parsing succeeded, a negative value otherwise.
int HTTPReq::parse(void) {
	parsed_ = true;
	size_t content_length = 0;

	// Parse the request header
	std::string method_line = readLine();
	if (!method_line.length() || errno == EIO) {
		std::cerr << "Failed to find request method/type line" << std::endl;
		return -1;
	}

	char method[255], uri[255];
	int rval;
	if (3 != (rval = sscanf(method_line.c_str(), "%254s %254s HTTP/%lf", 
		            method, uri, &version_))) {
		std::cerr << "Failed to scan request method/type from request (got " << rval
		          << " pieces)" << std::endl;
		return -1;
	}
	method_ = method;
	uri_ = uri;

	// Parse the following header items
	do {
		// Find the next header line
		std::string line = readLine();
		if (line.length() == 0) {
			if (!errno) {		// Got a simple \r\n
				break;
			} else {
				return -1;
			}
		}

		// Make sure it's valid
		size_t colon = line.find_first_of(':');
		if (colon == std::string::npos) {
			std::cerr << "Malformed request header item: '" << line << "'" << std::endl;
			return -1;
		}

		// Figure out what it contains
		std::string key = line.substr(0, colon - 0);
		std::string val = line.substr(colon + 2);

        if (0 == strncasecmp(key.c_str(), "connection", key.length())) {
            this->connection_ = strtol(val.c_str(), nullptr, 0);
        }
		else if (0 == strncasecmp(key.c_str(), "content-length", key.length())) {
			content_length = strtol(val.c_str(), nullptr, 0);
		} 
		// else {
		// 	std::cerr << "Ignoring header field '" << key << "'" << std::endl;
		// }

	} while (true);

	// Parse out the body, if we can
	if (content_length > 0) {
		body_ = readBytes(content_length);
		if (body_ == "") {
			perror("Failed to read body: ");
			return -1;		//errno is set
		}

	} else {
		body_ = "";
	}
	
	// Close the connection.
    if (0 == strncasecmp(connection_.c_str(), "close", connection_.length())) {
        close(sock_fd_);
    }
	malformed_ = false;
	return 0;
}

std::string HTTPReq::readLine(void) {
	errno = 0;
	int state = 0;			// 0: reading bytes; 1: read \r; 2: read \r\n
	std::string line = "";

	while(state != 2) {
		char byte;
		int rval = read(sock_fd_, (void*)&byte, 1);
		if (rval < 0) {
			// errno is set
			return "";
		} else if (rval == 1) {
			line.append(1, byte);
			if (state == 1) {
				state = (byte == '\n') ? 2 : 0;
			} else if (state == 0 && byte == '\r') {
				state = 1;
			}
		}
	}
	return line.substr(0, line.length() - 2);
}

std::string HTTPReq::readBytes(const size_t length) {
	errno = 0;
	std::string data = "";

	while(data.length() < length) {
		char byte;
		int rval = read(sock_fd_, (void*)&byte, 1);
		if (rval < 0) {
			// errno is set
			return "";
		} else if (rval == 1) {
			data.append(1, byte);
		} else if (rval != 0) {
			errno = EFAULT;
			return "";
		}
	}

	return data;
}

const std::string HTTPReq::getMethod(void) const {
	return method_;
}

const std::string HTTPReq::getURI(void) const {
	return uri_;
}

const std::string HTTPReq::getBody(void) const {
	return body_;
}

const double HTTPReq::getVersion(void) const {
	return version_;
}

const std::string HTTPReq::getConnection(void) const {
    return connection_;
}

const bool HTTPReq::isMalformed(void) const {
	return malformed_;
}

std::ostream& operator<<(std::ostream& os, const HTTPReq& req) {
	os << "Header: HTTP version " << req.version_ << ", method " << req.method_
	   << ", uri '" << req.uri_ <<  ", connection " << req.connection_ << "'. Body: " << std::endl;
	os << req.body_ << std::endl;
	return os;
}


