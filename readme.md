# fwk::enum

## WTF is fwk::enum?

It's a header-only library with an improved enum class. Features:
- abitility to iterate over, enumerate in a range for loop and count all elements
- conversion to / from strings (const char*)
- no weirdness, you can use it just like you would use enum class
- VERY lightweight: only about 140 lines of code in a single file
- fast compilation and low runtime overhead
- depends only on boost (preprocessor & optional)
- safe to use in switches (assuming decent compiler)

For more goodness (including EnumMap) have a look at libfwk:
[https://github.com/nadult/libfwk](https://github.com/nadult/libfwk)

## Requirements
- compiler with C++14 support
- boost

## Limitations
- you cannot specify custom values for each enum
- you cannot define enums within class or function scope
- number of enum elements is limited to 64 (AFAIK); it depends on boost::preprocessor

## Examples

```

FWK_ENUM(MyEnum, item_one, item_two, item_three, item_four);

void someFunc() {
	fromString<MyEnum>("item_two") == MyEnum::item_two;
	fromString<MyEnum>("not an item") == boost::none;
	string("item_three") == toString(MyEnum::item_three);

	string text;
	for(auto elem : all<MyEnum>())
		text = text + toString(elem) + " ";

	// Cyclic iteration over enum values
	next(MyEnum::item_four) == MyEnum::item_one;
	prev(MyEnum::item_three) == MyEnum::item_two;

	// It's better to use EnumMap from libfwk instead: it's just as fast
	// and it provides some compile and run time checks
	int values[count<MyOtherEnum>()] = {1, 2, 3, 4, 5};

	for(auto elem : all<MyOtherEnum>())
		cout << elem << ": " << values[(int)elem] << endl;

	return 0;
}
```

## License

fwk::enum is distributed under the terms of the 2-clause BSD license (see license.txt).

