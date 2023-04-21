#pragma once

#include "tcp_connection.h"
#include "status.h"
#include <openssl/ssl.h>
#include <openssl/bio.h>

#include <iostream>

namespace network {
/**
* @brief A wrapper for SSL-connections
*
*/
template<typename IP_ADDR_TYPE>
class ssl_connection {
public:
    ssl_connection() : m_ssl(nullptr) {
        
    }
    
    ssl_connection(tcp_connection<IP_ADDR_TYPE>&& connection, SSL* sslHandle) : m_connection(std::move(connection)), m_ssl(sslHandle) {
        
    }
    
    ssl_connection(const ssl_connection& that) = delete;
    ssl_connection(ssl_connection&& conn) : m_connection(std::move(conn.m_connection)), m_ssl(std::move(conn.m_ssl)) {
        conn.m_ssl = nullptr;
    }
    ssl_connection& operator=(ssl_connection && conn) {
        m_connection(std::move(conn.m_connection));
        m_ssl = std::move(conn.m_ssl);
        conn.m_ssl = nullptr;
        
        return *this;
    }

    /**
    * @brief closes the connection
    *
    */
    ~ssl_connection() {
        if (m_ssl != nullptr) {
            SSL_free(m_ssl);
        }
    }

    /**
    * @brief gets current socket
    */
    int get_socket() {
        return m_connection.get_socket();
    }
        
    /**
    * @brief sends bytes
    *
    * @param region memory_region_view to send
    * @return {status, SSL_write return value}
    */
    std::pair<status, int> send_data(const memory_region_view& region) {        
        return build_return(SSL_write(m_ssl, region.data(), region.size()));
    }

    /**
    * @brief receives bytes
    *
    * @param region memory for recv
    * @return {status, SSL_read return value}
    */
    std::pair<status, int> recv_data(memory_region& region) {        
        return build_return(SSL_read(m_ssl, region.data(), region.size()));      
    }

    /**
    * @brief sends buffer.msgLen()-Bytes
    *
    * @param buffer buffer containing the data
    * @return {status, SSL_write return value}
    */
    std::pair<status, int> send_pkt(pkt_buffer& buffer) {
        auto region = buffer.readable_region();
        int result = SSL_write(m_ssl, region.data(), region.size());
        
        if (result > 0) {
            region = region.splice(0, result);
            buffer.read(region);
        }
        
        return build_return(result);
    }

    /**
    * @brief receives maximal buffer.capacity-Bytes and stores them into the buffer
    *
    * @param buffer reference to the destination buffer
    * @return {status, SSL_read return value}
    */
    std::pair<status, int> recv_pkt(pkt_buffer& buffer) {       
        auto region = buffer.writeable_region();
        int result = SSL_read(m_ssl, region.data(), region.size());
        
        if (result > 0) {
            region = region.splice(0, result);
            buffer.write(region);
        }
        
        return build_return(result);
    }

    std::pair<status, int> accept() {    
        return build_return(SSL_accept(m_ssl));
    }
    
    std::string to_string() {
        return m_connection.get_addr().ip() + ":" + std::to_string(m_connection.get_addr().port());
    }
private:
    std::pair<status, int> build_return(const int result) const {
        if (result > 0) {
            return {status::ok, result};
        } else {
            int error = SSL_get_error(m_ssl, result);
            
            if (error == SSL_ERROR_WANT_READ) {
                return {status::retry_read, result};
            } else if (error == SSL_ERROR_WANT_WRITE) {
                return {status::retry_write, result};
            } else {
                return {status::error, result};
            }
        }
    }
    
    tcp_connection<IP_ADDR_TYPE> m_connection;
    SSL* m_ssl;
};
}



