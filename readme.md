# fwk::enum

## WTF is fwk::enum?

It's a header-only library with an improved enum class. Features:
- abitility to iterate over, and count all elements
- conversion from/to strings (const char*)
- no weirdness, you can use it just like you would use enum class
- small: only about 150 lines of code in a single file
- VERY lightweight: fast compilation & low overhead
- depends only on boost (preprocessor & optional)
- safe to use in switches (assuming decent compiler)

For more goodness (including EnumMap) have a look at libfwk:
[https://github.com/nadult/libfwk](https://github.com/nadult/libfwk)

## Limitations
- you cannot specify custom values for each enum
- you cannot define enums within classes or structs (namespaces are ok)
- number of enum elements is limited to 64 (AFAIK); it depends on boost::preprocessor

## Examples

```

DEFINE_ENUM(MyEnum, item_one, item_two, item_three, item_four);

void someFunc() {
	// Conversion from strings to enums and back
	fromString<MyEnum>("item_two") == MyEnum::item_two;
	fromString<MyEnum>("not an item") == boost::none;
	string("item_three") == toString(MyEnum::item_three);

	// Iterating over enums
	string text;
	for(auto elem : all<MyEnum>())
		text = text + toString(elem) + " ";
	enumNext(MyEnum::item_four) == MyEnum::item_one;
	enumPrev(MyEnum::item_three) == MyEnum::item_two;

	// Explicit conversions to/from int's
	for(int n = 0; n < count<MyOtherEnum>(); n++)
		std::cout << MyOtherEnum(n) << ": " << n;
	std::cout << "\n";
	printf("all good!\n");

	return 0;
}
```

## License

fwk::enum is distributed under the terms of the 2-clause BSD license (see license.txt).

