#include "file_loader.h"

web_server::http::response web_server::modules::file_loader::get_callback(const http::request& request, userdata& local) {
    return load_file(request.resource);
}

int web_server::modules::file_loader::hex_to_int(const char c) {
    if (c < 65) {
        return c - 48;
    } else if (c < 97) {
        return c - 55;
    } else {
        return c - 87;
    }
}

std::string web_server::modules::file_loader::utf8_convert(const std::string& input) {
    std::string result;
    
    int hex_found = 0;
    int hex_value = 0;

    for (const char c : input) {
        if (c != '%') {
            if (hex_found == 0) {
                result += c;
            } else if (hex_found == 2) {
                hex_value += 16 * hex_to_int(c);
                hex_found -= 1;
            } else {
                hex_value += hex_to_int(c);
                hex_found -= 1;
                result += (char)hex_value;
            }
        } else {
            hex_value = 0;
            hex_found = 2;
        }             
    }
    
    return result;
}

std::string web_server::modules::file_loader::filter_filename(const std::string& filename) {
    std::string result = filename;
        
    if (result.find("..") != std::string::npos) {
        result = "";
    }
    
    if (result.find("~/") != std::string::npos) {
        result = "";
    }
    
    if (result == "/") {
        result = "/index.html";
    }
    
    return result;
}

web_server::http::response web_server::modules::file_loader::load_file(const std::string& filename) {
    http::response result;
    std::string unescaped_filename = utf8_convert(filter_filename(filename));
    
    result.code = 404;                
    if (unescaped_filename != "") {
        std::ifstream file_stream("../www" + unescaped_filename, std::ios::binary);

        if (file_stream.is_open()) {
            if (file_stream.good()) {
                // really, this is the most stupid thing ever
                // if the filename is a directory, the size is wrong...
                // TODO fix this shit
                try {
                    result.data = std::vector<char>(std::istreambuf_iterator<char>(file_stream), std::istreambuf_iterator<char>());
                    result.code = 200;              
                } catch (const std::exception& e) {
                    
                }
            }
        }
    }

    return result;
}
