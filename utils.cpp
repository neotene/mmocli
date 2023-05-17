#ifdef _WIN32
# define WINVER 0x0A00
# define _WIN32_WINNT 0x0A00
#endif

#include "root_certificates.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/http/dynamic_body.hpp>

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <string>
#include <iostream>
#include <clocale>
#include <locale>
#include <vector>
#include <codecvt>
#include <cstdint>
#include <iostream>
#include <locale>
#include <string>

#include "base64.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


std::string send_register(const std::string& email, const std::string& pwd)
{
		std::string host = "mmocli.online";
        auto const  port = "5465";
        // auto const  text = "{\"type\":\"register\",\"email\":\"" + std::string(email.begin(), email.end()) + "\",\"password\":\"" + std::string(pwd.begin(), pwd.end()) + "\"}";

        // The io_context is required for all I/O
        net::io_context ioc;

        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv12_client};

        // This holds the root certificate used for verification
        load_root_certificates(ctx);

        // These objects perform our I/O
        tcp::resolver resolver{ioc};
        websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        auto ep = net::connect(get_lowest_layer(ws), results);

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str()))
            throw beast::system_error(
                beast::error_code(
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ':' + std::to_string(ep.port());

        // Perform the SSL handshake
        ws.next_layer().handshake(ssl::stream_base::client);

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));

        // Perform the websocket handshake
        ws.handshake(host, "/");

        // Send the message

        json j = {
            {"type", "register"},
            {"email", email},
            {"password", pwd}
        };

        std::string text = j.dump();

        ws.write(net::buffer(std::string(text)));

        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message into our buffer
        ws.read(buffer);

        std::string response = beast::buffers_to_string(buffer.data());

        // Close the WebSocket connection
        try {
        ws.close(websocket::close_code::normal);
        } catch (std::exception const& e) {
            spdlog::info("error: {}", e.what());
        }

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        return response;
}

std::string ws_to_s(std::wstring ws)
{
  std::setlocale(LC_ALL, "");
  const std::locale locale("");
  typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;
  const converter_type& converter = std::use_facet<converter_type>(locale);
  std::vector<char> to(ws.length() * converter.max_length());
  std::mbstate_t state;
  const wchar_t* from_next;
  char* to_next;
  const converter_type::result result = converter.out(state, ws.data(), ws.data() + ws.length(), from_next, &to[0], &to[0] + to.size(), to_next);
  if (result == converter_type::ok || result == converter_type::noconv) {
    return std::string(&to[0], to_next);
  }
  return "";
}

std::string mailgun_send_mail(std::string const& credentials,
    std::string const& domain, std::string const& mailgun_domain,
    std::string const& from, std::string const& to,
    std::string const& subject, std::string const& message)
{
    std::string const host = mailgun_domain;
    std::string const target = "/v3/" + domain + "/messages";
    int version = 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx(ssl::context::tlsv12_client);

    // This holds the root certificate used for verification
    load_root_certificates(ctx);

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);

        // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(! SSL_set_tlsext_host_name(stream.native_handle(), host.data()))
    {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    // Look up the domain name
    auto const results = resolver.resolve(host, "443");

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(stream).connect(results);

    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::post, target, version};

    req.set(beast::http::field::content_type, "application/x-www-form-urlencoded");
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    auto credentials64 = base64_encode_pem(credentials);

    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::accept, "*/*");
    req.set(http::field::authorization, "Basic " + credentials64);

    std::string body;

    body += "--------------------------2d360f43b56b5019\r\n";
    body += "Content-Disposition: form-data; name=\"from\"\r\n\r\n";
    body += from + "\r\n";
    body += "--------------------------2d360f43b56b5019\r\n";
    body += "Content-Disposition: form-data; name=\"to\"\r\n\r\n";
    body += to + "\r\n";
    body += "--------------------------2d360f43b56b5019\r\n";
    body += "Content-Disposition: form-data; name=\"subject\"\r\n\r\n";
    body += subject + "\r\n";
    body += "--------------------------2d360f43b56b5019\r\n";
    body += "Content-Disposition: form-data; name=\"text\"\r\n\r\n";
    body += message + "\r\n";
    body += "--------------------------2d360f43b56b5019--";

    req.set(http::field::content_length, std::to_string(body.size()));
    req.set(http::field::content_type, "multipart/form-data; boundary=------------------------2d360f43b56b5019");

    req.body() = body;

    req.prepare_payload();

    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    if (res.result() != http::status::ok)
        throw std::runtime_error("Mailgun error: " + std::to_string(res.result_int()));

    std::string response_body = beast::buffers_to_string(res.body().data());

    json j = json::parse(response_body);

    if (j["message"] != "Queued. Thank you.")
        throw std::runtime_error("Mailgun error: " + static_cast<std::string>(j["message"]));

    // Gracefully close the stream
    beast::error_code ec;
    stream.shutdown(ec);
    if(ec == net::error::eof)
    {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
    if(ec)
        throw beast::system_error{ec};

    // If we get here then the connection is closed gracefully

    return j["id"];
}

#include <string>
#include <fstream>
#include <streambuf>

std::string read_text_file(std::string const& path)
{
    std::ifstream t(path);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());

    return str;
}

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>
namespace pt = boost::property_tree;

std::string get_ini_value(std::string const& xml, std::string const& key)
{
    std::stringstream ss(xml);
    pt::ptree pt;
    pt::read_ini(ss, pt);
    return pt.get<std::string>(key);
}
