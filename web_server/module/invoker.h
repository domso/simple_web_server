#pragma once

#include <vector>
#include <type_traits>

#include "network/memory_region.h"
#include "definition.h"
    
namespace web_server::module {       
    template<typename T, typename... Ts>
    class invoker {
    public:
        template<typename Treq>
        auto& get() {
            return m_instances.template get<Treq>().instance;
        }
        template<typename Treq>
        const auto& get() const {
            return m_instances.template get<Treq>().instance;
        }

        template<typename Tcontext>
        auto call(
            http::request& request,
            Tcontext& context,
            network::socket_container_notifier& notifier
        ) {
            http::response result;
            result.code = 404;

            m_instances.for_each([&]<typename Tdef>(Tdef& definition) {
                Tdef::options::template get<"path">([&]<util::string_literal Tpath>() {
                    if (Tpath.check(request.module.c_str())) {
                        result = call_module_callback(definition, request, context);
                        if (result.code == 101) {
                            if constexpr (web_server::module::has_native_support<decltype(definition.instance)>) {
                                context->is_native = true;                
                                context->native_module_id = m_instances.template id<Tdef>();
                                auto& userdata = context->userdata.template get<util::tagged_type<typename decltype(definition.instance)::userdata, Tdef>>().instance;
                                definition.instance.native_open_callback(userdata, notifier);
                            } else {
                                result.code = 404;
                            }
                        }
                    }
                });
            });
            if (result.code == 404) {
                m_instances.for_each([&]<typename Tdef>(Tdef& definition) {
                    Tdef::options::template get<"path">([&]<util::string_literal Tpath>() {
                        if (Tpath.check("/")) {
                            result = call_module_callback(definition, request, context);
                            if (result.code == 101) {
                                if constexpr (web_server::module::has_native_support<decltype(definition.instance)>) {
                                    context->is_native = true;                
                                    context->native_module_id = m_instances.template id<Tdef>();
                                    auto& userdata = context->userdata.template get<util::tagged_type<typename decltype(definition.instance)::userdata, Tdef>>().instance;
                                    definition.instance.native_open_callback(userdata, notifier);
                                } else {
                                    result.code = 404;
                                }
                            }
                        }
                    });
                });
            }

            return result;
        }

        template<typename Tcontext>
        auto call_recv_by_number(
            const size_t id,
            network::memory_region read_region,
            Tcontext& context
        ) {
            size_t result = read_region.size();

            m_instances.for_id(id, [&]<typename Tdef>(Tdef& definition) {
                if constexpr (web_server::module::has_native_support<decltype(definition.instance)>) {
                    auto& userdata = context->userdata.template get<util::tagged_type<typename decltype(definition.instance)::userdata, Tdef>>().instance;
                    result = definition.instance.native_recv_callback(read_region, userdata);
                }
            });

            return result;
        }

        template<typename Tcontext>
        void call_update_by_number(
            const size_t id,
            std::vector<char>& buffer,
            Tcontext& context
        ) {
            m_instances.for_id(id, [&]<typename Tdef>(Tdef& definition) {
                if constexpr (web_server::module::has_native_support<decltype(definition.instance)>) {
                    auto& userdata = context->userdata.template get<util::tagged_type<typename decltype(definition.instance)::userdata, Tdef>>().instance;
                    definition.instance.native_update_callback(buffer, userdata);
                }
            });
        }
        template<typename Tcontext>
        bool call_status_by_number(
            const size_t id,
            Tcontext& context
        ) {
            bool result = false;
            m_instances.for_id(id, [&]<typename Tdef>(Tdef& definition) {
                if constexpr (web_server::module::has_native_support<decltype(definition.instance)>) {
                    auto& userdata = context->userdata.template get<util::tagged_type<typename decltype(definition.instance)::userdata, Tdef>>().instance;
                    result = definition.instance.native_status_callback(userdata);
                }
            });

            return result;
        }
        template<typename Tdef, typename Tcontext>
        auto call_module_callback(
            Tdef& definition,
            const http::request& request,
            Tcontext& context
        ) {
            typedef decltype(definition.instance) Tcurrent;
            auto& module = definition.instance;
            http::response response;
            response.code = 404;

            Tdef::options::template get<"username">([&]<util::string_literal username>() {
                Tdef::options::template get<"password">([&]<util::string_literal password>() {
                    Tdef::options::template get<"path">([&]<util::string_literal path>() {
                        if (username.length() > 0) {
                            if (
                                request.fields.count("Authorization") == 0 || 
                                request.fields.at("Authorization") != " Basic " + util::base64::to_base64(std::string(username.data()) + ":" + std::string(password.data())) + "\r\n"
                            ) {                
                                response.fields["WWW-Authenticate"] = "Basic realm=\"" + std::string(path.data()) + "\"";
                                response.code = 401;
                            }
                        }
                    });
                });
            });

            if (response.code == 401) {
                return response;
            }

            if (request.method == "GET") {
                if constexpr (web_server::module::has_get_support<Tcurrent>) {
                    if constexpr (web_server::module::has_userdata_member<Tcurrent>) {
                        return module.get_callback(request, context->userdata.template get<util::tagged_type<typename Tcurrent::userdata, Tdef>>().instance);
                    } else {
                        return module.get_callback(request);
                    }
                } else {
                    return response;
                }
            } else if (request.method == "PUT") {
                if constexpr (web_server::module::has_put_support<Tcurrent>) {
                    if constexpr (web_server::module::has_userdata_member<Tcurrent>) {
                        return module.put_callback(request, context->userdata.template get<util::tagged_type<typename Tcurrent::userdata, Tdef>>().instance);
                    } else {
                        return module.put_callback(request);
                    }
                } else {
                    return response;
                }
            } else if (request.method == "POST") {
                if constexpr (web_server::module::has_post_support<Tcurrent>) {
                    if constexpr (web_server::module::has_userdata_member<Tcurrent>) {
                        return module.post_callback(request, context->userdata.template get<util::tagged_type<typename Tcurrent::userdata, Tdef>>().instance);
                    } else {
                        return module.post_callback(request);
                    }
                } else {
                    return response;
                }
            } else if (request.method == "DELETE") {
                if constexpr (web_server::module::has_delete_support<Tcurrent>) {
                    if constexpr (web_server::module::has_userdata_member<Tcurrent>) {
                        return module.delete_callback(request, context->userdata.template get<util::tagged_type<typename Tcurrent::userdata, Tdef>>().instance);
                    } else {
                        return module.delete_callback(request);
                    }
                } else {
                    return response;
                }
            } else if (request.method == "PATCH") {
                if constexpr (web_server::module::has_options_support<Tcurrent>) {
                    if constexpr (web_server::module::has_userdata_member<Tcurrent>) {
                        return module.options_callback(request, context->userdata.template get<util::tagged_type<typename Tcurrent::userdata, Tdef>>().instance);
                    } else {
                        return module.options_callback(request);
                    }
                } else {
                    return response;
                }
            } else if (request.method == "OPTIONS") {
                if constexpr (web_server::module::has_patch_support<Tcurrent>) {
                    if constexpr (web_server::module::has_userdata_member<Tcurrent>) {
                        return module.patch_callback(request, context->userdata.template get<util::tagged_type<typename Tcurrent::userdata, Tdef>>().instance);
                    } else {
                        return module.patch_callback(request);
                    }
                } else {
                    return response;
                }
            } else if (request.method == "CONNECT") {
                if constexpr (web_server::module::has_connect_support<Tcurrent>) {
                    if constexpr (web_server::module::has_userdata_member<Tcurrent>) {
                        return module.connect_callback(request, context->userdata.template get<util::tagged_type<typename Tcurrent::userdata, Tdef>>().instance);
                    } else {
                        return module.connect_callback(request);
                    }
                } else {
                    return response;
                }
            } else if (request.method == "TRACE") {
                if constexpr (web_server::module::has_trace_support<Tcurrent>) {
                    if constexpr (web_server::module::has_userdata_member<Tcurrent>) {
                        return module.trace_callback(request, context->userdata.template get<util::tagged_type<typename Tcurrent::userdata, Tdef>>().instance);
                    } else {
                        return module.trace_callback(request);
                    }
                } else {
                    return response;
                }
            }

            return response;
        }
    private:
        util::pack_instances<T, Ts...> m_instances;
    };
}
