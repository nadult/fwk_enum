/* Copyright (C) 2017 Krzysztof Jakubowski <nadult@fastmail.fm>

   This file is part of fwk::enum. */

#include "fwk_enum.h"
#include <iostream>
#include <stdio.h>

using namespace fwk;
using namespace std;

FWK_ENUM(MyEnum, item_one, item_two, item_three, item_four);
FWK_ENUM(MyOtherEnum, weapon, armor, pants, hat, other);

template <class T>
auto operator<<(ostream &os, T value) -> typename enable_if<IsEnum<T>::value, ostream &>::type {
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
	assert(next(MyEnum::item_four) == MyEnum::item_one);
	assert(prev(MyEnum::item_three) == MyEnum::item_two);

	// It's better to use EnumMap from libfwk instead: it's just as fast
	// and it provides some compile and run time checks
	int values[count<MyOtherEnum>()] = {1, 2, 3, 4, 5};

	for(auto elem : all<MyOtherEnum>())
		cout << elem << ": " << values[(int)elem] << endl;
	cout << "All good!" << endl;

	return 0;
}
