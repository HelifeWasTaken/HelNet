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
        }
    }
}