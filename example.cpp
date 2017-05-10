/* Copyright (C) 2017 Krzysztof Jakubowski <nadult@fastmail.fm>

   This file is part of fwk::enum. */

#include "fwk_enum.h"
#include <iostream>
#include <stdio.h>

using namespace fwk;
using namespace std;

DEFINE_ENUM(MyEnum, item_one, item_two, item_three, item_four);
DEFINE_ENUM(MyOtherEnum, weapon, armor, pants, hat, other);

template <class T>
auto operator<<(std::ostream &os, T value) ->
	typename std::enable_if<IsEnum<T>::value, std::ostream &>::type {
	return os << toString(value);
}

int main() {
	assert(fromString<MyEnum>("item_two") == MyEnum::item_two);
	assert(fromString<MyEnum>("not an item") == boost::none);
	assert(string("pants") == toString(MyOtherEnum::pants));

	string text;
	for(auto elem : all<MyEnum>())
		text = text + toString(elem) + " ";
	assert(text == "item_one item_two item_three item_four ");

	assert(count<MyEnum>() == 4);
	assert(enumNext(MyEnum::item_four) == MyEnum::item_one);
	assert(enumPrev(MyEnum::item_three) == MyEnum::item_two);

	for(int n = 0; n < count<MyOtherEnum>(); n++)
		std::cout << MyOtherEnum(n) << ' ';
	std::cout << "\n";
	printf("all good!\n");

	return 0;
}
