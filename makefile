INCLUDES= -I../FangOost -I../FunctionalUtilities -I../cfdistutilities -I../rapidjson/include/rapidjson -I../Vasicek -I../websocketpp -I../asio/asio/include -I../GaussNewton -I../AutoDiff -I../TupleUtilities
GCCVAL=g++

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	GCCVAL=g++-7
endif
httpCreditRisk:main.o 
	$(GCCVAL) -std=c++14 -O3 $(STATIC) -g  main.o $(INCLUDES) -o httpCreditRisk -fopenmp

main.o: main.cpp CreditUtilities.h CheckSchema.h metaschema.json inputschema.json serverschema.json
	$(GCCVAL) -std=c++14 -O3 -D VERBOSE_FLAG=1 $(STATIC) -g  -c main.cpp   $(INCLUDES) -fopenmp

clean:
	-rm *.o httpCreditRisk *.out

test: test.cpp CheckSchema.h CreditUtilities.h metaschema.json inputschema.json serverschema.json
	$(GCCVAL) -std=c++14 test.cpp $(INCLUDES) -o test -fopenmp

