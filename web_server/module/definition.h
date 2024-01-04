#pragma once

#include "util/named_pair.h"
#include "util/named_options.h"
#include "util/pack_iteratable.h"

#include "network/memory_region.h"
#include "network/memory_region_view.h"
#include "network/socket_container.h"

#include "web_server/http/request.h"
#include "web_server/http/response.h"

namespace web_server::module {
    template<util::string_literal value>
    using username = util::named_pair<"username", value>;

    template<util::string_literal value>
    using password = util::named_pair<"password", value>;

    template<util::string_literal value>
    using path = util::named_pair<"path", value>;

    template<typename T, typename... Ts>
    using options = util::named_options<T, Ts...>;

    template<typename T, typename Topt>
    struct definition {
        T instance;
        typedef Topt options;
    };

    template<typename T>
    concept has_userdata_member = requires () {
        typename T::userdata;
    };

    template<typename T>
    concept has_native_support = requires (T t, size_t n, bool flag, network::memory_region region, T::userdata& data, network::socket_container_notifier notifier, std::vector<char>& buff) {
        n = t.native_recv_callback(region, data);
        flag = t.native_status_callback(data);
        t.native_update_callback(buff, data);
        t.native_open_callback(data, notifier);
    };

    template<typename T>
    concept has_get_support = requires (T t, const http::request& request, http::response response) {
        response = t.get_callback(request);
    } || requires (T t, const http::request& request, http::response response, T::userdata& data) {
        response = t.get_callback(request, data);
    };
    template<typename T>
    concept has_put_support = requires (T t, const http::request& request, http::response response) {
        response = t.put_callback(request);
    } || requires (T t, const http::request& request, http::response response, T::userdata& data) {
        response = t.put_callback(request, data);
    };
    template<typename T>
    concept has_post_support = requires (T t, const http::request& request, http::response response) {
        response = t.post_callback(request);
    } || requires (T t, const http::request& request, http::response response, T::userdata& data) {
        response = t.post_callback(request, data);
    };
    template<typename T>
    concept has_delete_support = requires (T t, const http::request& request, http::response response) {
        response = t.delete_callback(request);
    } || requires (T t, const http::request& request, http::response response, T::userdata& data) {
        response = t.delete_callback(request, data);
    };
    template<typename T>
    concept has_connect_support = requires (T t, const http::request& request, http::response response) {
        response = t.connect_callback(request);
    } || requires (T t, const http::request& request, http::response response, T::userdata& data) {
        response = t.connect_callback(request, data);
    };
    template<typename T>
    concept has_options_support = requires (T t, const http::request& request, http::response response) {
        response = t.options_callback(request);
    } || requires (T t, const http::request& request, http::response response, T::userdata& data) {
        response = t.options_callback(request, data);
    };
    template<typename T>
    concept has_trace_support = requires (T t, const http::request& request, http::response response) {
        response = t.trace_callback(request);
    } || requires (T t, const http::request& request, http::response response, T::userdata& data) {
        response = t.trace_callback(request, data);
    };
    template<typename T>
    concept has_patch_support = requires (T t, const http::request& request, http::response response) {
        response = t.patch_callback(request);
    } || requires (T t, const http::request& request, http::response response, T::userdata& data) {
        response = t.patch_callback(request, data);
    };
}
