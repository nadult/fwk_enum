## fwk::enum ?

It's a header-only C++14 library with an improved enum class. Features:  

- ability to iterate over, enumerate in a range-for and count all elements  
- flag support without any additional code
- conversion to / from strings (const char*)  
- no weirdness, you can use it just like you would use enum class  
- VERY lightweight: less than 250 lines of code in a single file  
- fast compilation and low runtime overhead  
- depends only on boost (preprocessor & optional)  
- enums safe to use in switches (assuming decent compiler)  


For more goodness (including EnumMap) have a look at libfwk:  
[https://github.com/nadult/libfwk](https://github.com/nadult/libfwk) 

## Requirements
- compiler with C++14 support
- boost

## Limitations
- you cannot specify custom values for each enum
- you cannot define enums within class or function scope
- number of enum elements is limited to 64

## Examples

```

FWK_ENUM(MyEnum, item_one, item_two, item_three, item_four);

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

// Flag operations work out of the box:
flags1 = MyEnum::item_one | MyEnum::item_two | MyEnum::item_three;
flags1 == ~MyEnum::item_four;
flags1.bits == 1 + 2 + 4;
flag(MyEnum::item_four) == 8;

// MyFlags uses unsigned char as underlying type because MyEnum has only 4 elements
using MyFlags = EnumFlags<MyEnum>;

MyFlags flags = flags1 ^ MyEnum::item_one;

```

## License

fwk::enum is distributed under the terms of Boost Software License (version 1.0).  
See license.txt for details.
