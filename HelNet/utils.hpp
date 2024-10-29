/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/
#pragma once

#include <type_traits>

namespace hl
{
    namespace net
    {
        namespace utils
        {
#if __cplusplus >= 201703L
            template<bool B, class T = void>
            using enable_if_t = std::enable_if_t<B, T>;
#else
            template<bool B, class T = void>
            using enable_if_t = typename std::enable_if<B, T>::type;
#endif

            template<class T, class U>
            using is_same = std::is_same<T, U>;

#if __GNUC__ || __clang__
    #define HL_NET_DIAGNOSTIC_PUSH() _Pragma("GCC diagnostic push")
    #define HL_NET_DIAGNOSTIC_POP() _Pragma("GCC diagnostic pop")
    #define HL_NET_DIAGNOSTIC_UNUSED_PARAMETER_IGNORED() _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")
    #define HL_NET_DIAGNOSTIC_UNUSED_VARIABLE_IGNORED() _Pragma("GCC diagnostic ignored \"-Wunused-variable\"")
    #define HL_NET_DIAGNOSTIC_UNUSED_FUNCTION_IGNORED() _Pragma("GCC diagnostic ignored \"-Wunused-function\"")
    #define HL_NET_DIAGNOSTIC_UNUSED_LABEL_IGNORED() _Pragma("GCC diagnostic ignored \"-Wunused-label\"")
    #define HL_NET_DIAGNOSTIC_IMPLICIT_FALLTHROUGH_IGNORED() _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
    #define HL_NET_DIAGNOSTIC_NON_VIRTUAL_DESTRUCTOR_IGNORED() _Pragma("GCC diagnostic ignored \"-Wnon-virtual-dtor\"")
#elif _MSC_VER
    // TODO: Check if all the diagnostic macros are correct
    #define HL_NET_DIAGNOSTIC_PUSH __pragma(warning(push))
    #define HL_NET_DIAGNOSTIC_POP __pragma(warning(pop))
    #define HL_NET_DIAGNOSTIC_UNUSED_PARAMETER_IGNORED __pragma(warning(disable: 4100))
    #define HL_NET_DIAGNOSTIC_UNUSED_VARIABLE_IGNORED __pragma(warning(disable: 4101))
    #define HL_NET_DIAGNOSTIC_UNUSED_FUNCTION_IGNORED __pragma(warning(disable: 4505))
    #define HL_NET_DIAGNOSTIC_UNUSED_LABEL_IGNORED __pragma(warning(disable: 4102))
    #define HL_NET_DIAGNOSTIC_IMPLICIT_FALLTHROUGH_IGNORED __pragma(warning(disable: 26495))
    #define HL_NET_DIAGNOSTIC_NON_VIRTUAL_DESTRUCTOR_IGNORED __pragma(warning(disable: 4265))
#else
    #define HL_NET_DIAGNOSTIC_PUSH
    #define HL_NET_DIAGNOSTIC_POP
    #define HL_NET_DIAGNOSTIC_UNUSED_PARAMETER_IGNORED
    #define HL_NET_DIAGNOSTIC_UNUSED_VARIABLE_IGNORED
    #define HL_NET_DIAGNOSTIC_UNUSED_FUNCTION_IGNORED
    #define HL_NET_DIAGNOSTIC_UNUSED_LABEL_IGNORED
    #define HL_NET_DIAGNOSTIC_IMPLICIT_FALLTHROUGH_IGNORED
    #define HL_NET_DIAGNOSTIC_NON_VIRTUAL_DESTRUCTOR_IGNORED
    #warning "Compiler not supported for hlnet diagnostic macros"
#endif
        }
    }
}