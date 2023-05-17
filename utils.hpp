#pragma once

#include <string>

std::string send_register(const std::string& email, const std::string& pwd);
std::string ws_to_s(std::wstring ws);
std::string mailgun_send_mail(std::string const& credentials,
    std::string const& domain, std::string const& mailgun_domain,
    std::string const& from, std::string const& to,
    std::string const& subject, std::string const& message);
std::string read_text_file(std::string const& path);
std::string get_ini_value(std::string const& xml, std::string const& key);

inline std::wstring s_to_ws( const std::string& as )
{
    // wchar_t* buf = new wchar_t[as.size() * 2 + 2];
    // swprintf( buf, L"%S", as.c_str() );
    // std::wstring rval = buf;
    // delete[] buf;
    // return rval;
    return std::wstring(as.begin(), as.end());
}
