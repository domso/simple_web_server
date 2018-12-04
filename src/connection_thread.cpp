#include "connection_thread.h"

#include "http_parser.h"
#include "http_request.h"

void connection_thread::main(network::tcp_connection<network::ipv4_addr> connection, shared_context& context) {
    network::pkt_buffer recvBuffer(1024);   
    network::pkt_buffer sendBuffer(1024); 
    network::pkt_buffer requestBuffer(1024);    

    while(connection.recv_pkt(recvBuffer).first) {        
        requestBuffer.push_buffer(recvBuffer);        
        
        for (int i = 3; i < requestBuffer.msg_length(); i++) {
            if (requestBuffer.data()[i - 0] == '\n' &&
                requestBuffer.data()[i - 1] == '\r' &&
                requestBuffer.data()[i - 2] == '\n' &&
                requestBuffer.data()[i - 3] == '\r'                
            ) {
                std::string header((char*) requestBuffer.data(), i);
                auto request = http_parser::parse_request(header);
                auto response = http_request::handle_request(request, context);
                
                sendBuffer.push_string(build_response(response.first));
        
                connection.send_pkt(sendBuffer);
                connection.send_data<char>(response.second.data(), response.second.size());
                
                requestBuffer.clear();
                sendBuffer.clear();
            }
        }        
    }
}

std::string connection_thread::build_response(const std::unordered_map<std::string, std::string>& fieldMap) {        
    std::string result;    
    auto it = fieldMap.find("STATUS");
    
    if (it != fieldMap.end()) {
         result = "HTTP/1.1 " + it->second + "\r\n";   
    }    
    
    for (auto& keyValue : fieldMap) {
        if (keyValue.first != "STATUS") {
            result += keyValue.first + ": " + keyValue.second + "\r\n";
        }
    }
    
    result += "\r\n";
    
    return result;
}
