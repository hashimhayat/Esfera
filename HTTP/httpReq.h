// httpreq.hpp - HTTP Request Parser
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
#include <unistd.h>

// Parses the headers and body out of a HTTP request
class HTTPReq {
	const std::string header_sep = "\r\n";

  public:
	HTTPReq(const int sock_fd);
	int parse(void);

	std::string readLine(void);
	std::string readBytes(const size_t length);

	const std::string getMethod(void) const;
	const std::string getURI(void) const;
	const std::string getBody(void) const;
	const std::string getConnection(void) const;
	const double getVersion(void) const;
	const bool isMalformed(void) const;

	friend std::ostream& operator<<(std::ostream& os, const HTTPReq& req);  

  private:
	const int sock_fd_;
	bool parsed_;
	bool malformed_;
	
	std::string method_;
	std::string uri_;
	std::string body_;
	std::string connection_;
	double version_;
};

std::ostream& operator<<(std::ostream& os, const HTTPReq& req);
