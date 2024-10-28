/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <memory>
#include <unordered_map>

namespace hl
{
namespace net
{
namespace plugins
{
    template<class Updatable, class Callbacks>
    struct base_plugin
    {
        using unique_t = std::unique_ptr<base_plugin>;

        using updatable_t = Updatable;
        using callbacks_t = Callbacks;

        virtual ~base_plugin() = default;

        virtual bool require_connection_on() const = 0;
        virtual void on_update(updatable_t &updatable) = 0;

        virtual callbacks_t callbacks() = 0;
    };

    template<class PluginBase>
    class plugin_manager
    {
    public:
        using base_plugin_t = PluginBase;
        using unique_plugin_t = typename base_plugin_t::unique_t;
        using updatable_t = typename base_plugin_t::updatable_t;

    private:
        std::unordered_map<std::string, unique_plugin_t> m_plugins;

        template<class T>
        static const char *gen_name() noexcept { return typeid(T).name(); }

    public:
        plugin_manager()
            : m_plugins()
        {
        }

        virtual ~plugin_manager() = default;

        template<class T, class... Args>
        void attach(updatable_t &updatable, Args... args)
        {
            HL_NET_LOG_INFO("Attaching plugin: {} to {}", gen_name<T>(), updatable->get_alias());
            unique_plugin_t plugin = unique_plugin_t(new T(std::forward<Args>(args)...));
            updatable->callbacks_register().add_layer(gen_name<T>(), plugin->callbacks());
            m_plugins[gen_name<T>()] = std::move(plugin);
        }

        template<class T>
        void detach(updatable_t &updatable)
        {
            HL_NET_LOG_INFO("Detaching plugin: {} from {}", gen_name<T>(), updatable->get_alias());
            updatable->callbacks_register().remove_layer(gen_name<T>());
            m_plugins.erase(gen_name<T>());
        }

        void update(updatable_t &updatable)
        {
            for (auto &plugin : m_plugins)
            {
                if (plugin.second->require_connection_on() && !updatable->healthy())
                {
                    continue;
                }
                else
                {
                    plugin.second->on_update(updatable);
                }
            }
        }
    };

}
}
}

