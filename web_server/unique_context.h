#pragma once

#include <vector>
#include <string>
#include <stdint.h>

#include "util/pack_instances.h"
#include "util/pack_iteratable.h"
#include "util/tagged_type.h"
#include "web_server/module/definition.h"
#include "network/socket_container.h"

#include "network/pkt_buffer.h"
#include "util/logger.h"

namespace web_server {    
    template<typename... Ts>
    struct unique_context {        
        ~unique_context() {
            util::logger::log_debug("Delete context of " + name);            
        }
        network::pkt_buffer recv_buffer = {1024 * 16};
        std::vector<char> response_data;
        std::string name;

        bool is_native = false;
        size_t native_module_id;
        
        decltype(util::pack_iteratable<Ts...>::filter([]<typename T>() constexpr {
            return web_server::module::has_userdata_member<decltype(T::instance)>;
        }).map([]<typename T>() constexpr {
            return util::pack_iteratable<util::tagged_type<typename decltype(T::instance)::userdata, T>>();
        }))::instance userdata;
    }; 
}

