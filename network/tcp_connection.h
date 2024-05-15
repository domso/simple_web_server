#ifndef tcp_connection_h
#define tcp_connection_h

#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <cstring>
#include "ip_addr.h"
#include "base_socket.h"
#include "pkt_buffer.h"

namespace network {
/**
* @brief A wrapper for the classical tcp-connection-sockets
*
*/
template<typename IP_ADDR_TYPE>
class tcp_connection : public base_socket<IP_ADDR_TYPE> {
public:
    tcp_connection() {
        // create new socket
        this->m_skt = socket(this->m_addr.family(), SOCK_STREAM, 0);
    }
    tcp_connection(const int fd) {
        // create new socket
        this->m_skt = fd;
    }
    tcp_connection(const tcp_connection& that) = delete;
    tcp_connection(tcp_connection&& conn) : base_socket<IP_ADDR_TYPE>(std::move(conn)) {

    }
    tcp_connection& operator=(tcp_connection && conn) {
        base_socket<IP_ADDR_TYPE>::operator=(std::move(conn));

        return *this;
    }

    /**
    * @brief closes the socket
    *
    */
    ~tcp_connection() {

    }

    /**
    * @brief tries to open a connection to the host specified by @addr
    *
    * @param addr host-address
    * @return bool: success
    */
    bool connect_to(const IP_ADDR_TYPE& addr) {
        this->m_addr = addr;


        if (this->m_skt == -1) {
            return false;
        }

        if (connect(
                    this->m_skt,
                    (struct sockaddr*) this->m_addr.internal_handle(),
                    sizeof(*(this->m_addr.internal_handle()))) != 0
           ) {
            return false;
        }

        socklen_t addrlen = static_cast<socklen_t>(sizeof(*(this->m_addr.internal_handle())));
        if (getsockname(
                    this->m_skt,
                    reinterpret_cast<struct sockaddr*>(this->m_addr.internal_handle()),
                    &addrlen) != 0
           ) {
            return false;
        }

        return true;
    }

    /**
    * @brief sends data
    *
    * @param region memory region view to send
    * @param flags see 'man send()'
    * @return {success, errno}
    */
    std::pair<bool, int> send_data(const memory_region_view& region, const int flags = MSG_NOSIGNAL) const {
        int result = send(this->m_skt, region.data(), region.size(), flags);
        
        return this->check_error(result);
    }

    /**
    * @brief receives data
    *
    * @param region memory region for recv
    * @param flags see 'man recv()'
    * @return {success, errno}
    */
    std::pair<bool, int> recv_data(memory_region& region, const int flags = 0) const {
        int result = recv(this->m_skt, region.data(), region.size(), flags);
        return this->check_error(result);
    }

    /**
    * @brief sends buffer.msgLen()-Bytes
    *
    * @param buffer buffer containing the data
    * @param flags see 'man send()'
    * @return {success, errno}
    */
    std::pair<bool, int> send_pkt(pkt_buffer& buffer, const int flags = MSG_NOSIGNAL) const {
        auto region = buffer.readable_region();
        int result = send(this->m_skt, region.data(), region.size(), flags);
        if (result > 0) {
            region = region.splice(0, result);
            buffer.read(region);
        }
        
        return this->check_error(result);
    }

    /**
    * @brief receives maximal buffer.capacity-Bytes and stores them into the buffer
    *
    * @param buffer reference to the destination buffer
    * @param flags see 'man recv'
    * @return {success, errno}
    */
    std::pair<bool, int> recv_pkt(pkt_buffer& buffer, const int flags = 0) const {
        auto region = buffer.writeable_region();
        int result = recv(this->m_skt, region.data(), region.size(), flags);
        if (result > 0) {
            region = region.splice(0, result);
            buffer.write(region);
        }       
        
        return this->check_error(result);
    }
};
}

#endif
