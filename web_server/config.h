#pragma once

#include <stdint.h>

#include "util/named_pair.h"
#include "util/named_options.h"
#include "util/pack_iteratable.h"

namespace web_server::configuration {
    template<util::string_literal value>
    using name = util::named_pair<"name", value>;
    template<util::string_literal value>
    using certificate = util::named_pair<"certificate", value>;
    template<util::string_literal value>
    using key = util::named_pair<"key", value>;
    template<util::string_literal value>
    using password = util::named_pair<"password", value>;

    template<uint16_t value>
    using port = util::named_pair<"port", value>;
    template<int value>
    using max_pending = util::named_pair<"max_pending", value>;

    template<bool value>
    using enable_log_status = util::named_pair<"enable_log_status", value>;
    template<bool value>
    using enable_log_warning = util::named_pair<"enable_log_warning", value>;
    template<bool value>
    using enable_log_error = util::named_pair<"enable_log_error", value>;
    template<bool value>
    using enable_log_debug = util::named_pair<"enable_log_debug", value>;

    template<size_t value>
    using num_worker = util::named_pair<"num_worker", value>;
    template<size_t value>
    using worker_poll_timeout = util::named_pair<"worker_poll_timeout", value>;
    template<size_t value>
    using worker_poll_buffer_size = util::named_pair<"worker_poll_buffer_size", value>;
    template<size_t value>
    using accept_poll_timeout = util::named_pair<"accept_poll_timeout", value>;
    template<size_t value>
    using accept_poll_buffer_size = util::named_pair<"accept_poll_buffer_size", value>;

    template<typename T, typename... Ts>
    using options = util::named_options<T, Ts...,
        name<"https://github.com/domso/simple_web_server">,
        certificate<"cert.pem">,
        key<"key.pem">,
        password<"test">,

        port<443>,
        max_pending<10>,

        enable_log_status<true>,
        enable_log_warning<true>,
        enable_log_error<true>,
        enable_log_debug<true>,

        num_worker<1>,
        worker_poll_timeout<1000>,
        worker_poll_buffer_size<10>,
        accept_poll_timeout<1000>,
        accept_poll_buffer_size<10>
    >;
}
