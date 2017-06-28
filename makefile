INCLUDES= -I../FangOost -I../FunctionalUtilities -I../rapidjson/include/rapidjson -I../Vasicek -I../websocketpp -I../asio/asio/include
GCCVAL=g++
OBJC=objcopy
TYPBIN=elf64-x86-64

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	GCCVAL=g++-7
	OBJC=gobjcopy
	TYPBIN=mach-o-x86-64
endif
httpCreditRisk:main.o inputschema.o serverschema.o metaschema.o
	$(GCCVAL) -std=c++14 -O3 $(STATIC) -g  main.o inputschema.o serverschema.o metaschema.o $(INCLUDES) -o httpCreditRisk -fopenmp

main.o: main.cpp CreditUtilities.h 
	$(GCCVAL) -std=c++14 -O3 $(STATIC) -g  -c main.cpp   $(INCLUDES) -fopenmp

inputschema.o: inputschema.json
	$(OBJC) --input binary --output $(TYPBIN) --binary-architecture i386 inputschema.json inputschema.o

serverschema.o: serverschema.json
	$(OBJC) --input binary --output $(TYPBIN) --binary-architecture i386 serverschema.json serverschema.o

metaschema.o: metaschema.json
	$(OBJC) --input binary --output $(TYPBIN) --binary-architecture i386 metaschema.json metaschema.o


clean:
	-rm *.o httpCreditRisk *.out

test: test.cpp CheckSchema.h CreditUtilities.h
	$(GCCVAL) -std=c++14 test.cpp $(INCLUDES) -o test -fopenmp

