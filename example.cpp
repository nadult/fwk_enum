// Copyright (C) Krzysztof Jakubowski <nadult@fastmail.fm>
// This file is part of libfwk. See license.txt for details.

#include "fwk_enum.h"
#include <iostream>
#include <stdio.h>

using namespace fwk;
using namespace std;

FWK_ENUM(MyEnum, item_one, item_two, item_three, item_four);
FWK_ENUM(MyOtherEnum, weapon, armor, pants, hat, other);

template <class T, EnableIfEnum<T>...> ostream &operator<<(ostream &os, T value) {
	return os << toString(value);
}

template <class TFlags, EnableIfEnumFlags<TFlags>...> constexpr int countBits(const TFlags &flags) {
	int out = 0;
	for(int i = 0; i < TFlags::max_flags; i++)
		out += flags.bits & (typename TFlags::Base(1) << i) ? 1 : 0;
	return out;
}

using MyFlags = EnumFlags<MyEnum>;

// Here we also specify underlying type
using MyOtherFlags = EnumFlags<MyOtherEnum, unsigned int>;

void func(MyOtherFlags flags) { cout << "Called func(MyOtherFlags)!\n"; }

void func(MyFlags flags) {
	cout << "Called func(MyFlags) with following flags:\n";
	for(auto elem : all<MyEnum>())
		if(flags & elem)
			cout << toString(elem) << "\n";
	cout << "\n";
}

int main() {
	assert(fromString<MyEnum>("item_two") == MyEnum::item_two);
	assert(MyEnum::item_three != fromString<MyEnum>("item_four"));
	assert(!fromString<MyEnum>("not an item"));
	assert(string("pants") == toString(MyOtherEnum::pants));

	string text;
	for(auto elem : all<MyEnum>())
		text = text + toString(elem) + " ";
	assert(text == "item_one item_two item_three item_four ");

	assert(count<MyEnum>() == 4);
	assert(next(MyEnum::item_four) == MyEnum::item_one);
	assert(prev(MyEnum::item_three) == MyEnum::item_two);

	// It's better to use EnumMap from libfwk instead: it's just as fast
	// and it provides some compile and run time checks
	int values[count<MyOtherEnum>()] = {1, 2, 3, 4, 5};

	constexpr auto flags1 = MyEnum::item_one | MyEnum::item_two | MyEnum::item_three;
	constexpr auto flags2 = MyEnum::item_two | MyEnum::item_three | MyEnum::item_four;

	static_assert(flags1 == ~MyEnum::item_four, "");
	static_assert(flags1.bits == 1 + 2 + 4, "");
	static_assert((flags2 ^ flags1) == (MyEnum::item_one | MyEnum::item_four), "");
	static_assert(countBits(flags1) == 3, "");
	static_assert(sizeof(flags1) == 1, "");

	auto flags3 = MyOtherEnum::armor | MyOtherEnum::pants;

	func(flags1);
	func(flags3);

	cout << "\nEnumerating all item types:\n";
	for(auto elem : all<MyOtherEnum>())
		cout << elem << ": " << values[(int)elem] << endl;
	cout << "All good!" << endl;

	return 0;
}
