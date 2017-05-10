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

#include <boost/preprocessor/list/to_tuple.hpp>
#include <boost/preprocessor/list/transform.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>

#ifdef NDEBUG
#define DASSERT(expr) ((void)0)
#else
#define DASSERT(expr) assert(expr)
#endif

#include <boost/optional.hpp>

namespace fwk {

// You can switch this to something else
template <class T> using optional = boost::optional<T>;

#define FWK_STRINGIZE(...) FWK_STRINGIZE_(__VA_ARGS__)
#define FWK_STRINGIZE_(...) #__VA_ARGS__

#define FWK_UNLIST_(...) __VA_ARGS__
#define FWK_UNLIST(...) FWK_UNLIST_ __VA_ARGS__

#define FWK_STRINGIZE_OP_(r, data, elem) FWK_STRINGIZE(elem)
#define FWK_STRINGIZE_LIST(...)                                                                    \
	FWK_UNLIST(BOOST_PP_LIST_TO_TUPLE(                                                             \
		BOOST_PP_LIST_TRANSFORM(FWK_STRINGIZE_OP_, _, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))))

template <class T, int size> constexpr int arraySize(T (&)[size]) noexcept { return size; }

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

	EnumRange(int min, int max) : m_min(min), m_max(max) { DASSERT(min >= 0 && max >= min); }

  protected:
	int m_min, m_max;
};

template <int size_> struct StringRange {
	enum { size = size_ };
	StringRange(const char *const ptr[size]) : strings(ptr) {}

	inline int toEnum(const char *string) const {
		DASSERT(string);
		for(int n = 0; n < size; n++)
			if(strcmp(string, strings[n]) == 0)
				return n;
		return -1;
	}
	const char *fromEnum(int n) const { return strings[n]; }

  private:
	const char *const *strings;
};

// Safe enum class
// Initially initializes to 0 (first element). Converts to int, can be easily used as
// an index into some array. Can be converted to/from strings, which are automatically generated
// from enum names. Enum with strings cannot be defined at function scope. Some examples are
// available in src/test/enums.cpp.
#define DEFINE_ENUM(Type, ...)                                                                     \
	enum class Type : unsigned char { __VA_ARGS__ };                                               \
	inline auto enumStrings(Type) {                                                                \
		static const char *const s_strings[] = {FWK_STRINGIZE_LIST(__VA_ARGS__)};                  \
		constexpr int size = fwk::arraySize(s_strings);                                            \
		return StringRange<size>(s_strings);                                                       \
	}                                                                                              \
	constexpr int enumCount(Type) { return decltype(enumStrings(Type()))::size; }

template <class T> struct IsEnum {
	template <class C> static auto test(int) -> decltype(enumCount(C()));
	template <class C> static auto test(...) -> void;
	enum { value = std::is_same<int, decltype(test<T>(0))>::value };
};

template <class T> auto enumNext(T value) -> typename std::enable_if<IsEnum<T>::value, T>::type {
	return T((int(value) + 1) % enumCount(T()));
}

template <class T> auto enumPrev(T value) -> typename std::enable_if<IsEnum<T>::value, T>::type {
	return T((int(value) + (enumCount(T()) - 1)) % enumCount(T()));
}

template <class T>
static auto fromString(const char *str) ->
	typename std::enable_if<IsEnum<T>::value, optional<T>>::type {
	int id = enumStrings(T()).toEnum(str);
	if(id != -1)
		return T(id);
	return boost::none;
}

template <class T>
static auto fromString(const std::string &str) ->
	typename std::enable_if<IsEnum<T>::value, T>::type {
	return fromString<T>(str.c_str());
}

template <class T>
static auto toString(T value) -> typename std::enable_if<IsEnum<T>::value, const char *>::type {
	return enumStrings(T()).fromEnum((int)value);
}

template <class T> constexpr auto count() -> typename std::enable_if<IsEnum<T>::value, int>::type {
	return enumCount(T());
}
template <class T> auto all() -> typename std::enable_if<IsEnum<T>::value, EnumRange<T>>::type {
	return EnumRange<T>(0, count<T>());
}
}

#endif
