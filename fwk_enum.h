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
		constexpr int size = fwk::arraySize(s_strings);                                            \
		return EnumInfo<size>(s_strings);                                                          \
	}

template <class T> struct IsEnum {
	template <class C> static auto test(int) -> decltype(enumInfo(C()));
	template <class C> static auto test(...) -> void;
	enum { value = !std::is_same<void, decltype(test<T>(0))>::value };
};

template <class TEnum, class T>
using EnableForEnum = typename std::enable_if<IsEnum<TEnum>::value, T>::type;

template <class T> auto fromString(const char *str) -> EnableForEnum<T, boost::optional<T>> {
	int id = enumInfo(T()).toEnum(str);
	if(id != -1)
		return T(id);
	return boost::none;
}

template <class T> auto fromString(const std::string &str) -> EnableForEnum<T, T> {
	return fromString<T>(str.c_str());
}

template <class T> auto toString(T value) -> EnableForEnum<T, const char *> {
	return enumInfo(T()).toString((int)value);
}

template <class T> constexpr auto count() -> EnableForEnum<T, int> {
	return decltype(enumInfo(T()))::size;
}

template <class T> auto all() -> EnableForEnum<T, EnumRange<T>> {
	return EnumRange<T>(0, count<T>());
}

template <class T> auto next(T value) -> EnableForEnum<T, T> {
	return T((int(value) + 1) % count<T>());
}

template <class T> auto prev(T value) -> EnableForEnum<T, T> {
	return T((int(value) + (count<T>() - 1)) % count<T>());
}
}

#endif
