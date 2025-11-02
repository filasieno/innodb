
#pragma once

#include <string_view>
#include <format>

using ib_size_t = unsigned long long int;

/// \defgroup ut_comptime_string Comptime string
/// \ingroup ut


/// \brief Compile-time fixed string helper and concatenation
/// \ingroup ut_comptime_string
template <ib_size_t N>
struct ut_comptime_string {
    static_assert(N > 0, "String length must be greater than 0");
    
    constexpr ut_comptime_string() = default;

    constexpr ut_comptime_string(const char (&str)[N]) {
        for (ib_size_t i = 0; i < N; ++i) value[i] = str[i];
    }
    
    constexpr const char* c_str() const noexcept { return value; }
    
    constexpr const char& operator[](ib_size_t i) const noexcept {
        return value[i]; // Note: no bounds checking for performance, assumes valid indices
    }
    
    constexpr operator std::string_view() const noexcept {
        return std::string_view{value};
    }

    constexpr ib_size_t size() const noexcept { return N; }

    constexpr ib_size_t length() const noexcept { return N > 0 ? N - 1 : 0; }

    constexpr bool empty() const noexcept { return N <= 1; }

    char value[N];
};

namespace std {
    template<ib_size_t N>
    struct formatter<ut_comptime_string<N>, char> {

        /// \brief Parse format specifications
        /// \param pc The parse context
        /// \return The iterator
        constexpr typename basic_format_parse_context<char>::iterator parse(basic_format_parse_context<char>& pc) {
            return fmtter.parse(pc);
        }

        /// \brief Format the string
        /// \param str The string to format
        /// \param fc The format context
        /// \return The iterator
        template<typename Out>
        typename basic_format_context<Out, char>::iterator format(const ut_comptime_string<N>& str, basic_format_context<Out, char>& fc) const {
            return fmtter.format(static_cast<std::string_view>(str), fc);
        }

    private:
        formatter<std::string_view, char> fmtter;
    };
}

/// \brief CTAD deduction guide for string literals/char arrays
/// \ingroup ut_comptime_string
template <ib_size_t N>
ut_comptime_string(const char (&)[N]) -> ut_comptime_string<N>;

/// \brief Compile-time concatenation via operator+
/// \ingroup ut_comptime_string
template <ib_size_t N1, ib_size_t N2>
consteval ut_comptime_string<N1 + N2 - 1> operator+(const ut_comptime_string<N1>& a, const ut_comptime_string<N2>& b) noexcept
{
    ut_comptime_string<N1 + N2 - 1> out{};
    for (ib_size_t i = 0; i < N1 - 1; ++i) out.value[i] = a.value[i];
    for (ib_size_t j = 0; j < N2; ++j) out.value[(N1 - 1) + j] = b.value[j];
    return out;
}

/// \brief Compile-time concatenation via operator+
/// \ingroup ut_comptime_string
template <ib_size_t N1, ib_size_t N2>
consteval ut_comptime_string<N1 + N2 - 1> operator+(const ut_comptime_string<N1>& a, const char (&b)[N2]) noexcept
{
    return a + ut_comptime_string<N2>{b};
}

/// \brief Compile-time concatenation via operator+
/// \ingroup ut_comptime_string
template <ib_size_t N1, ib_size_t N2>
consteval ut_comptime_string<N1 + N2 - 1> operator+(const char (&a)[N1], const ut_comptime_string<N2>& b) noexcept
{
    return ut_comptime_string<N1>{a} + b;
}
