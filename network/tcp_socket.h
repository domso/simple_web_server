#ifndef tcp_socket_h
#define tcp_socket_h

#include <memory>
#include <optional>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "ip_addr.h"
#include "base_socket.h"
#include "tcp_connection.h"

namespace network {
    /**
    * @brief A wrapper for the posix accept-functions of the posix-tcp-sockets
    * 
    */
    template<typename IP_ADDR_TYPE>
    class tcp_socket : public network::base_socket<IP_ADDR_TYPE> {
    public:
        tcp_socket() {

        }
        tcp_socket(const tcp_socket& that) = delete;
        
        tcp_socket(tcp_socket&& conn) : base_socket<IP_ADDR_TYPE>(std::move(conn)) {

        }
        /**
        * @brief closes the socket
        * 
        */
        ~tcp_socket() {

        }
        
        /**
        * @brief initializes a new tcp-socket, binds this socket to the given port and start listing for incoming connections
        * 
        * @param port port for accepting new connections
        * @param backlog max number of pending connections
        * @param blocking enable/disable blocking-calls
        * @return success
        */
        bool accept_on(const uint16_t port, const int backlog, const bool blocking = true) {
            // initialize the socket address
            this->m_addr.init("", port);

            // create new socket
            int socketType = SOCK_STREAM;
            if (!blocking) {
                socketType |= SOCK_NONBLOCK;
            }
            this->m_skt = socket(this->m_addr.family(), socketType, 0);

            if (this->m_skt == -1) {
                return false;
            }

            // set some further options
            int optval = 1;
            setsockopt(this->m_skt, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval , sizeof(int));

            // bind the socket to an address
            if (bind(this->m_skt, (struct sockaddr*) this->m_addr.internal_handle(), sizeof(*(this->m_addr.internal_handle()))) != 0) {
                return false;
            }
            
            unsigned int size = sizeof(*(this->m_addr.internal_handle()));
            if (getsockname(this->m_skt, (struct sockaddr*) this->m_addr.internal_handle(), &size) != 0) {
                return false;
            }

            // start listening for new incoming connections
            return listen(this->m_skt, backlog) == 0;
        }
               
        /**
        * @brief accepts new connections on the socket and emplace the socket in the given argument
        * 
        * @param connection destination-socket
        * @return success
        */
        bool accept_connection(network::tcp_connection<IP_ADDR_TYPE>& connection) const {
            socklen_t clientlen = sizeof(connection.get_addr().internal_handle());

            // accept new incoming connection
            int skt = accept(this->m_skt, (struct sockaddr*) connection.get_addr().internal_handle(), &clientlen);

            if (skt == -1) {
                return false;
            }

            // combine socket and connection-object
            connection.set_socket(skt);
            return true;
        }
        
        /**
        * @brief accepts new connections on the socket and emplace the socket in the given argument
        * 
        * @param connection destination-socket
        * @return success
        */
        std::optional<network::tcp_connection<IP_ADDR_TYPE>> accept_connection() const {
            network::tcp_connection<IP_ADDR_TYPE> connection(0);
            socklen_t clientlen = sizeof(connection.get_addr().internal_handle());

            // accept new incoming connection
            int skt = accept(this->m_skt, (struct sockaddr*) connection.get_addr().internal_handle(), &clientlen);

            if (skt == -1) {
                return std::nullopt;
            }

            // combine socket and connection-object
            connection.set_socket(skt);
            return std::move(connection);
        }
    };
}

#endif
