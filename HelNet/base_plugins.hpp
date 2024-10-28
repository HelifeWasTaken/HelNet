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

        template<class T>
        static const char *name() noexcept { return typeid(T).name(); }

        virtual ~base_plugin() = default;

        virtual bool require_connection_on() const = 0;
        virtual size_t on_update(Updatable &updatable) = 0;

        virtual Callbacks callbacks() = 0;
    };

    template<class PluginBase>
    struct plugin_manager
    {
        using unique_plugin_t = typename PluginBase::unique_t;
        using updatable_t = typename PluginBase::updatable_t;

        std::unordered_map<std::string, unique_plugin_t> m_plugins;

        virtual ~plugin_manager() = default;

        template<class T, class... Args>
        void attach(updatable_t &updatable, Args... args)
        {
            unique_plugin_t plugin = unique_plugin_t(new T(std::forward<Args>(args)...));
            m_plugins[T::name()] = std::move(plugin);
            updatable->add_layer(T::name(), plugin->callbacks());
        }

        template<class T>
        void detach(updatable_t &updatable)
        {
            updatable->remove_layer(T::name());
            m_plugins.erase(T::name());
        }
    };

}
}
}

