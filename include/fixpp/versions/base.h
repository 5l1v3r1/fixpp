/* base.h
   Mathieu Stefani, 15 november 2016
   
  Base macro for FIX versions
*/

#pragma once

#include <cstring>
#include <climits>

/*
 * We are using a special trick to compare versions of a FIX Message.
 * The trick is based on the fact that most of the versions are 7 bytes
 * long and thus can fit into an uint64_t.
 *
 * For such versions, we just encode the string in a simple uint64_t
 * and compare against that integers.
 *
 * For versions that are longer (FIX.5.0 SP1, FIX.5.0 SP2), we just use
 * a plain old strcmp
 *
 * Borrowed from Kestrel
 */

inline constexpr uint64_t getAsciiStringAsIntRecConst(const char* str, size_t size, size_t shift, uint64_t result)
{
    return (size == 0 ? result :
                getAsciiStringAsIntRecConst(str + 1, size - 1, shift + 1,
                    result | (static_cast<uint64_t>(*str) << (shift * CHAR_BIT))));
}

inline constexpr uint64_t getAsciiStringAsIntConst(const char* str, size_t size)
{
    return getAsciiStringAsIntRecConst(str, size, 0, 0);
}

inline uint64_t getAsciiStringAsInt(const char* str, size_t size)
{
    uint64_t value;
    std::memcpy(&value, str, size);
    return value;
}

#define FIX_BEGIN_VERSION_NAMESPACE(name, string)                               \
    namespace name                                                              \
    {                                                                           \
        struct Version                                                          \
        {                                                                       \
            static constexpr const char* Str = string;                          \
            static constexpr size_t Size = sizeof(string) - 1;                  \
            static bool equals_slow(const char* version, size_t size)           \
            {                                                                   \
                return !strncmp(Str, version, Size);                            \
            }                                                                   \
            static bool equals_fast(const char* version, size_t size)           \
            {                                                                   \
                const uint64_t value = getAsciiStringAsInt(version, size);      \
                return value == StrInt;                                         \
            }                                                                   \
            static bool equals(const char* version, size_t size)                \
            {                                                                   \
                if (Size != size) return false;                                 \
                if (size == 7)                                                  \
                    return equals_fast(version, size);                          \
                return equals_slow(version, size);                              \
            }                                                                   \
        public:                                                                 \
            static constexpr auto StrInt = getAsciiStringAsIntConst(Str, Size); \
        };                                                                      \
        constexpr const char* Version::Str;                                     \
        constexpr uint64_t Version::StrInt;                                     \
        template<typename Chars, typename... Tags>                              \
        using MessageV = VersionnedMessage<Version, Chars, Tags...>;            \
    }                                                                           \
    namespace name                                                              \

#define FIX_END_VERSION_NAMESPACE
