INCLUDES= -I../FangOost -I../FunctionalUtilities -I../rapidjson/include/rapidjson -I../Vasicek -I../websocketpp -I../asio/asio/include
GCCVAL=g++
OBJC=objcopy
TYPBIN=elf64-x86-64

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	GCCVAL=g++-7
endif
httpCreditRisk:main.o 
	$(GCCVAL) -std=c++14 -O3 $(STATIC) -g  main.o $(INCLUDES) -o httpCreditRisk -fopenmp

main.o: main.cpp CreditUtilities.h CheckSchema.h
	$(GCCVAL) -std=c++14 -O3 $(STATIC) -g  -c main.cpp   $(INCLUDES) -fopenmp

clean:
	-rm *.o httpCreditRisk *.out

test: test.cpp CheckSchema.h CreditUtilities.h
	$(GCCVAL) -std=c++14 test.cpp $(INCLUDES) -o test -fopenmp

