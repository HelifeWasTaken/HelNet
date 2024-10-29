/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#define _HL_INTERNAL_CALLBACK_IMPL_BASE(CALLBACK_TYPE, POOL, CALLBACK_MUTEX, CALLBACKS) \
    void unsafe_start_pool(void) { POOL.start(); } \
    void unsafe_stop_pool(void) { POOL.stop(); } \
    void add_layer(const std::string &layer, const CALLBACK_TYPE& callback=CALLBACK_TYPE()) \
    { \
        std::lock_guard<std::mutex> lock(CALLBACK_MUTEX); \
        HL_NET_LOG_INFO("Adding layer: {}", layer); \
        CALLBACKS[layer] = callback; \
    } \
    void remove_layer(const std::string &layer) \
    { \
        std::lock_guard<std::mutex> lock(CALLBACK_MUTEX); \
        HL_NET_LOG_INFO("Removing layer: {}", layer); \
        CALLBACKS.erase(layer); \
    } \
    std::vector<std::string> get_layers(void) const \
    { \
        std::lock_guard<std::mutex> lock(CALLBACK_MUTEX); \
        std::vector<std::string> layers; \
        for (const auto& layer : CALLBACKS) \
        { \
            layers.push_back(layer.first); \
        } \
        return layers; \
    } \
    void clear_layers(void) \
    { \
        std::lock_guard<std::mutex> lock(CALLBACK_MUTEX); \
        CALLBACKS.clear(); \
        HL_NET_LOG_INFO("Cleared all layers"); \
    }

#define _HL_INTERNAL_CALLBACK_REGISTER_IMPL_SETTERS(NAME, CALLBACK_TYPE, CALLBACKS, POOL, MUTEX) \
    void set_##NAME(const CALLBACK_TYPE##_##NAME##_callback& NAME##_callback, const std::string &layer = DEFAULT_REGISTER_LAYER) \
    { \
        std::lock_guard<std::mutex> lock(MUTEX, std::adopt_lock); \
        auto it = CALLBACKS.find(layer); \
        if (it != CALLBACKS.end()) \
        { \
            it->second.NAME##_callback = NAME##_callback; \
            HL_NET_LOG_INFO("Callback {} set on layer {}", #NAME, layer); \
        } \
        else if (layer == DEFAULT_REGISTER_LAYER) \
        { \
            HL_NET_LOG_INFO("Default Layer {} does not exist, creating it", layer); \
            add_layer(layer); \
            set_##NAME(NAME##_callback, layer); \
        } \
        else \
        { \
            HL_NET_LOG_ERROR("Cannot set callback {} on layer {} because the layer does not exist", #NAME, layer); \
        } \
    } \
    \
    void set_##NAME##_async(bool async, const std::string &layer = DEFAULT_REGISTER_LAYER) \
    { \
        std::lock_guard<std::mutex> lock(MUTEX, std::adopt_lock); \
        auto it = CALLBACKS.find(layer); \
        if (it != CALLBACKS.end()) \
        { \
            it->second.NAME##_is_async = async; \
            HL_NET_LOG_INFO("Async set to {} for callback {} on layer {}", async, #NAME, layer); \
        } \
        else if (layer == DEFAULT_REGISTER_LAYER) \
        { \
            HL_NET_LOG_INFO("Default Layer {} does not exist, creating it", layer); \
            add_layer(layer); \
            set_##NAME##_async(async, layer); \
        } \
        else \
        { \
            HL_NET_LOG_ERROR("Cannot set async for callback {} on layer {} because the layer does not exist", #NAME, layer); \
        } \
    }

// TODO: Fix this macro for callback async (Is sync for now)
#define _HL_INTERNAL_CALLBACK_REGISTER_IMPL(NAME, CALLBACK_TYPE, CALLBACKS, POOL, MUTEX, GET_SHARABLE) \
    template<typename ...Args> void NAME(Args&&... args) \
    { \
        std::lock_guard<std::mutex> lock(MUTEX, std::adopt_lock); \
        auto sharable = GET_SHARABLE(); \
        HL_NET_LOG_INFO("Calling sharable callback: {}", #NAME); \
        for (const auto& layer  : CALLBACKS) \
        { \
            const auto& callback_reg = layer.second; \
            if (callback_reg.NAME##_callback) \
            { \
                const auto& callback = callback_reg.NAME##_callback; \
                const bool& is_async = callback_reg.NAME##_is_async; \
                if (is_async) \
                { \
                    HL_NET_LOG_WARN("Async callback not implemented yet will be called synchronously"); \
                    callback(sharable, std::forward<Args>(args)...); \
                } \
                else \
                { \
                    callback(sharable, std::forward<Args>(args)...); \
                } \
            } \
        } \
        HL_NET_LOG_INFO("CallbackSharable: {} called", #NAME); \
    } \
    _HL_INTERNAL_CALLBACK_REGISTER_IMPL_SETTERS(NAME, CALLBACK_TYPE, CALLBACKS, POOL, MUTEX)

#define _HL_INTERNAL_CALLBACK_REGISTER_IMPL_NO_SHARABLE(NAME, CALLBACK_TYPE, CALLBACKS, POOL, MUTEX) \
    template<typename ...Args> void NAME(Args&&... args) \
    { \
        std::lock_guard<std::mutex> lock(MUTEX, std::adopt_lock); \
        HL_NET_LOG_INFO("Calling non sharable callback: {}", #NAME); \
        for (const auto& layer  : CALLBACKS) \
        { \
            const auto& callback_reg = layer.second; \
            if (callback_reg.NAME##_callback) \
            { \
                const auto& callback = callback_reg.NAME##_callback; \
                const bool& is_async = callback_reg.NAME##_is_async; \
                if (is_async) \
                { \
                    HL_NET_LOG_WARN("Async callback not implemented yet will be called synchronously"); \
                    callback(std::forward<Args>(args)...); \
                } \
                else \
                { \
                    callback(std::forward<Args>(args)...); \
                } \
            } \
        } \
        HL_NET_LOG_INFO("NonSharableCallback: {} called", #NAME); \
    } \
    _HL_INTERNAL_CALLBACK_REGISTER_IMPL_SETTERS(NAME, CALLBACK_TYPE, CALLBACKS, POOL, MUTEX)

#define _HL_INTERNAL_UNHEALTHY_CASES_CLIENT \
    case boost::asio::error::eof: \
    case boost::asio::error::connection_reset: \
    case boost::asio::error::operation_aborted: \
    case boost::asio::error::connection_aborted: \
    case boost::asio::error::broken_pipe: \
    case boost::asio::error::not_connected: \
    case boost::asio::error::bad_descriptor: \
    case boost::asio::error::fault: \
    case boost::asio::error::host_not_found: \
    case boost::asio::error::host_unreachable: \
    case boost::asio::error::network_down: \
    case boost::asio::error::network_reset: \
    case boost::asio::error::network_unreachable: \
    case boost::asio::error::no_recovery: \
    case boost::asio::error::message_size: \
    case boost::asio::error::connection_refused: \
    case boost::asio::error::timed_out: \
    case boost::asio::error::no_buffer_space: \
    case boost::asio::error::no_memory: \
    case boost::asio::error::no_protocol_option: \
    case boost::asio::error::no_such_device: \
    case boost::asio::error::operation_not_supported

#define _HL_INTERNAL_UNHEALTHY_CASES_CONNECTION_UNHEALTHY \
    case boost::asio::error::eof: \
    case boost::asio::error::connection_reset: \
    case boost::asio::error::operation_aborted: \
    case boost::asio::error::connection_aborted: \
    case boost::asio::error::broken_pipe: \
    case boost::asio::error::not_connected: \
    case boost::asio::error::host_not_found: \
    case boost::asio::error::connection_refused

#define _HL_INTERNAL_UNHEALTHY_CASES_SERVER_FROM_CONNECTION \
    case boost::asio::error::bad_descriptor: \
    case boost::asio::error::fault: \
    case boost::asio::error::host_unreachable: \
    case boost::asio::error::network_down: \
    case boost::asio::error::network_reset: \
    case boost::asio::error::network_unreachable: \
    case boost::asio::error::no_recovery: \
    case boost::asio::error::message_size: \
    case boost::asio::error::timed_out: \
    case boost::asio::error::no_buffer_space: \
    case boost::asio::error::no_memory: \
    case boost::asio::error::no_protocol_option: \
    case boost::asio::error::no_such_device: \
    case boost::asio::error::operation_not_supported

#define _HL_INTERNAL_UNHEALTHY_CASES_FOR_SERVER \
    _HL_INTERNAL_UNHEALTHY_CASES_CONNECTION_UNHEALTHY: \
    _HL_INTERNAL_UNHEALTHY_CASES_SERVER_FROM_CONNECTION
