#foo:foo.o
	#g++ foo.o -o foo
#foo.o:foo.cpp
	#g++ -c foo.cpp
highwind:highwind.cpp
	g++ highwind.cpp -o highwind
