/* Copyright (C) 2017 Krzysztof Jakubowski <nadult@fastmail.fm>

   This file is part of fwk::enum. */

#ifndef FWK_ENUM_H
#define FWK_ENUM_H

#include <cassert>
#include <cstring>
#include <string>
#include <type_traits>

#ifndef BOOST_PP_VARIADICS
#define BOOST_PP_VARIADICS 1
#endif

#if !BOOST_PP_VARIADICS
#error FWK_ENUM requires BOOST_PP_VARIADICS == 1
#endif

#include <boost/optional.hpp>
#include <boost/preprocessor/list/to_tuple.hpp>
#include <boost/preprocessor/list/transform.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>

namespace fwk {

#define FWK_STRINGIZE(...) FWK_STRINGIZE_(__VA_ARGS__)
#define FWK_STRINGIZE_(...) #__VA_ARGS__

#define FWK_UNLIST_(...) __VA_ARGS__
#define FWK_UNLIST(...) FWK_UNLIST_ __VA_ARGS__

#define FWK_STRINGIZE_OP_(r, data, elem) FWK_STRINGIZE(elem)
#define FWK_STRINGIZE_LIST(...)                                                                    \
	FWK_UNLIST(BOOST_PP_LIST_TO_TUPLE(                                                             \
		BOOST_PP_LIST_TRANSFORM(FWK_STRINGIZE_OP_, _, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))))

template <class T, int size> constexpr int arraySize(T (&)[size]) noexcept { return size; }

struct EnabledType {};
struct DisabledType;

namespace detail {
	struct ValidType {
		template <class A> using Arg = A;
	};
}

template <bool cond, class InvalidArg = DisabledType>
using EnableIf =
	typename std::conditional<cond, detail::ValidType, InvalidArg>::type::template Arg<EnabledType>;

template <class Type> class EnumRange {
  public:
	class Iter {
	  public:
		Iter(int pos) : pos(pos) {}

		auto operator*() const { return Type(pos); }
		const Iter &operator++() {
			pos++;
			return *this;
		}

		bool operator<(const Iter &rhs) const { return pos < rhs.pos; }
		bool operator==(const Iter &rhs) const { return pos == rhs.pos; }
		bool operator!=(const Iter &rhs) const { return pos != rhs.pos; }

	  private:
		int pos;
	};

	auto begin() const { return Iter(m_min); }
	auto end() const { return Iter(m_max); }
	int size() const { return m_max - m_min; }

	EnumRange(int min, int max) : m_min(min), m_max(max) { assert(min >= 0 && max >= min); }

  protected:
	int m_min, m_max;
};

template <int size_> class EnumInfo {
  public:
	enum { size = size_ };
	EnumInfo(const char *const ptr[size]) : strings(ptr) {}

	int toEnum(const char *string) const {
		assert(string);
		for(int n = 0; n < size; n++)
			if(strcmp(string, strings[n]) == 0)
				return n;
		return -1;
	}
	const char *toString(int n) const { return strings[n]; }

  private:
	const char *const *strings;
};

#define FWK_ENUM(Type, ...)                                                                        \
	enum class Type : unsigned char { __VA_ARGS__ };                                               \
	inline auto enumInfo(Type) {                                                                   \
		static const char *const s_strings[] = {FWK_STRINGIZE_LIST(__VA_ARGS__)};                  \
		static_assert(fwk::arraySize(s_strings) <= 64, "Maximum number of enum elements is 64");   \
		constexpr int size = fwk::arraySize(s_strings);                                            \
		return EnumInfo<size>(s_strings);                                                          \
	}

struct NotAnEnum;

namespace detail {
	template <class T> struct IsEnum {
		template <class C> static auto test(int) -> decltype(enumInfo(C()));
		template <class C> static auto test(...) -> void;
		enum { value = !std::is_same<void, decltype(test<T>(0))>::value };
	};
}

template <class T> constexpr bool isEnum() { return detail::IsEnum<T>::value; }

template <class T> using EnableIfEnum = EnableIf<isEnum<T>(), NotAnEnum>;

template <class T, EnableIfEnum<T>...> boost::optional<T> fromString(const char *str) {
	int id = enumInfo(T()).toEnum(str);
	if(id != -1)
		return T(id);
	return boost::none;
}

template <class T, EnableIfEnum<T>...> T fromString(const std::string &str) {
	return fromString<T>(str.c_str());
}

template <class T, EnableIfEnum<T>...> const char *toString(T value) {
	return enumInfo(T()).toString((int)value);
}

template <class T, EnableIfEnum<T>...> constexpr int count() {
	return decltype(enumInfo(T()))::size;
}

template <class T, EnableIfEnum<T>...> EnumRange<T> all() { return EnumRange<T>(0, count<T>()); }

template <class T, EnableIfEnum<T>...> T next(T value) { return T((int(value) + 1) % count<T>()); }

template <class T, EnableIfEnum<T>...> T prev(T value) {
	return T((int(value) + (count<T>() - 1)) % count<T>());
}
}

#endif
