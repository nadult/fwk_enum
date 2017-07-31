// Copyright (C) Krzysztof Jakubowski <nadult@fastmail.fm>
// This file is part of fwk::enum. See license.txt for details.

#ifndef FWK_ENUM_H
#define FWK_ENUM_H

#include <cassert>
#include <cstring>
#include <string>
#include <type_traits>

namespace fwk {

template <class T, int size> constexpr int arraySize(T (&)[size]) noexcept { return size; }

struct EnabledType {};
struct DisabledType;

namespace detail {
	struct ValidType {
		template <class A> using Arg = A;
	};
}

template <bool value, class T1, class T2>
using Conditional = typename std::conditional<value, T1, T2>::type;
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

		bool operator<(Iter rhs) const { return pos < rhs.pos; }
		bool operator==(Iter rhs) const { return pos == rhs.pos; }
		bool operator!=(Iter rhs) const { return pos != rhs.pos; }

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

namespace detail {

	template <unsigned...> struct Seq { using type = Seq; };
	template <unsigned N, unsigned... Is> struct GenSeqX : GenSeqX<N - 1, N - 1, Is...> {};
	template <unsigned... Is> struct GenSeqX<0, Is...> : Seq<Is...> {};
	template <unsigned N> using GenSeq = typename GenSeqX<N>::type;

	template <int N> struct Buffer { char data[N + 1]; };
	template <int N> struct Offsets { const char *data[N]; };

	constexpr bool isWhiteSpace(char c) {
		return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',';
	}

	constexpr const char *skipWhiteSpace(const char *t) {
		while(!*t)
			t++;
		return t;
	}
	constexpr const char *skipToken(const char *t) {
		while(*t)
			t++;
		return t;
	}

	constexpr int countTokens(const char *t) {
		while(isWhiteSpace(*t))
			t++;
		if(!*t)
			return 0;

		int out = 1;
		while(*t) {
			if(*t == ',')
				out++;
			t++;
		}
		return out;
	}

	template <unsigned N> struct OffsetsGen : public OffsetsGen<N - 1> {
		constexpr OffsetsGen(const char *current_ptr)
			: OffsetsGen<N - 1>(skipWhiteSpace(skipToken(current_ptr))), ptr(current_ptr) {}
		const char *ptr;
	};

	template <> struct OffsetsGen<1> {
		constexpr OffsetsGen(const char *ptr) : ptr(ptr) {}
		const char *ptr;
	};

	template <int N, unsigned... IS>
	constexpr Buffer<N> zeroWhiteSpace(const char (&t)[N], Seq<IS...>) {
		return {{(isWhiteSpace(t[IS]) ? '\0' : t[IS])...}};
	}

	template <int N> constexpr Buffer<N> zeroWhiteSpace(const char (&t)[N]) {
		return zeroWhiteSpace(t, GenSeq<N>());
	}

	template <class T, unsigned... IS> constexpr auto makeOffsets(T t, Seq<IS...>) {
		constexpr const char *start = skipWhiteSpace(t());
		constexpr unsigned num_tokens = sizeof...(IS);
		constexpr const auto oseq = OffsetsGen<num_tokens>(start);
		return Offsets<sizeof...(IS)>{{((const OffsetsGen<num_tokens - IS> &)oseq).ptr...}};
	}
}

#define FWK_ENUM(id, ...)                                                                          \
	enum class id : unsigned char { __VA_ARGS__ };                                                 \
	inline auto enumInfo(id) {                                                                     \
		using namespace detail;                                                                    \
		static constexpr const auto s_buffer = zeroWhiteSpace(#__VA_ARGS__);                       \
		static constexpr const auto s_offsets =                                                    \
			makeOffsets([] { return s_buffer.data; }, GenSeq<countTokens(#__VA_ARGS__)>());        \
		constexpr int size = fwk::arraySize(s_offsets.data);                                       \
		static_assert(size <= 64, "Maximum number of enum elements is 64");                        \
		return EnumInfo<size>(s_offsets.data);                                                     \
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

template <class T> struct MaybeEnum {
	constexpr MaybeEnum() : value(invalid_value) {}
	constexpr MaybeEnum(T value) : value(static_cast<unsigned char>(value)) {}

	// TODO: add support for None
	constexpr explicit operator bool() const { return value != invalid_value; }
	constexpr bool valid() const { return value != invalid_value; }

	constexpr T operator*() const {
		assert(value != invalid_value);
		return value;
	}

	constexpr bool operator==(MaybeEnum rhs) const { return value == rhs.value; }
	constexpr bool operator!=(MaybeEnum rhs) const { return value != rhs.value; }

  private:
	enum { invalid_value = 255 };
	unsigned char value;
};

template <class T> constexpr bool operator==(T lhs, MaybeEnum<T> rhs) { return rhs == lhs; }
template <class T> constexpr bool operator!=(T lhs, MaybeEnum<T> rhs) { return rhs != lhs; }

template <class T, EnableIfEnum<T>...> MaybeEnum<T> fromString(const char *str) {
	int id = enumInfo(T()).toEnum(str);
	if(id != -1)
		return T(id);
	return MaybeEnum<T>();
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

template <class T, EnableIfEnum<T>...>
using BestFlagsBase =
	Conditional<count<T>() <= 8, unsigned char,
				Conditional<count<T>() <= 16, unsigned short,
							Conditional<count<T>() <= 32, unsigned int, unsigned long long>>>;

template <class T, class Base_ = BestFlagsBase<T>> struct EnumFlags {
	enum { max_flags = fwk::count<T>() };
	using Base = Base_;
	static constexpr const Base mask =
		(Base(1) << (max_flags - 1)) - Base(1) + (Base(1) << (max_flags - 1));

	static_assert(isEnum<T>(), "EnumFlags<> should be based on fwk-enum");
	static_assert(std::is_unsigned<Base>::value, "Base type for EnumFlags<> should be unsigned");
	static_assert(fwk::count<T>() <= sizeof(Base) * 8, "Base type not big enough");

	constexpr EnumFlags() : bits(0) {}
	constexpr EnumFlags(T value) : bits(Base(1) << uint(value)) {}
	constexpr explicit EnumFlags(Base bits) : bits(bits) {}
	template <class TBase> constexpr EnumFlags(EnumFlags<T, TBase> rhs) : bits(rhs.bits) {}

	constexpr EnumFlags operator|(EnumFlags rhs) const { return EnumFlags(bits | rhs.bits); }
	constexpr EnumFlags operator&(EnumFlags rhs) const { return EnumFlags(bits & rhs.bits); }
	constexpr EnumFlags operator^(EnumFlags rhs) const { return EnumFlags(bits ^ rhs.bits); }
	constexpr EnumFlags operator~() const { return EnumFlags((~bits) & (mask)); }

	constexpr void operator|=(EnumFlags rhs) { bits |= rhs.bits; }
	constexpr void operator&=(EnumFlags rhs) { bits &= rhs.bits; }
	constexpr void operator^=(EnumFlags rhs) { bits ^= rhs.bits; }

	constexpr bool operator==(T rhs) const { return bits == (Base(1) << int(rhs)); }
	constexpr bool operator==(EnumFlags rhs) const { return bits == rhs.bits; }
	constexpr bool operator!=(EnumFlags rhs) const { return bits != rhs.bits; }

	constexpr explicit operator bool() const { return bits != 0; }

	static constexpr EnumFlags all() { return EnumFlags(mask); }

	Base bits;
};

template <class T, EnableIfEnum<T>...> constexpr EnumFlags<T> flag(T val) {
	return EnumFlags<T>(val);
}

template <class T, EnableIfEnum<T>...> constexpr EnumFlags<T> operator|(T lhs, T rhs) {
	return EnumFlags<T>(lhs) | rhs;
}

template <class T, EnableIfEnum<T>...> constexpr EnumFlags<T> operator&(T lhs, T rhs) {
	return EnumFlags<T>(lhs) & rhs;
}

template <class T, EnableIfEnum<T>...> constexpr EnumFlags<T> operator^(T lhs, T rhs) {
	return EnumFlags<T>(lhs) ^ rhs;
}

template <class T, EnableIfEnum<T>...> constexpr EnumFlags<T> operator~(T bit) {
	return ~EnumFlags<T>(bit);
}

namespace detail {
	template <class T> struct IsEnumFlags {
		enum { value = 0 };
	};
	template <class T, class Base> struct IsEnumFlags<EnumFlags<T, Base>> {
		enum { value = 1 };
	};
}

template <class T> constexpr bool isEnumFlags() { return detail::IsEnumFlags<T>::value; }

template <class T> using EnableIfEnumFlags = EnableIf<detail::IsEnumFlags<T>::value>;
}

#endif
