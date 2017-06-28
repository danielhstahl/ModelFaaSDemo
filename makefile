INCLUDES= -I../FangOost -I../FunctionalUtilities -I../rapidjson/include/rapidjson -I../Vasicek -I../websocketpp -I../asio/asio/include

httpCreditRisk:main.o inputschema.o serverschema.o 
	g++-7 -std=c++14 -O3 $(STATIC) -g -pthread main.o inputschema.o serverschema.o $(INCLUDES) -o httpCreditRisk -fopenmp

main.o: main.cpp CreditUtilities.h 
	g++-7 -std=c++14 -O3 $(STATIC) -g -pthread -c main.cpp   $(INCLUDES) -fopenmp

clean:
	-rm *.o httpCreditRisk *.out

test: test.cpp CheckSchema.h CreditUtilities.h
	g++-7 -std=c++14 -pthread test.cpp $(INCLUDES) -o test -fopenmp

