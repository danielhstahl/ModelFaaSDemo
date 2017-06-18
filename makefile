INCLUDES= -I ../FangOost -I ../FunctionalUtilities -I../rapidjson/include/rapidjson -I../cpp-httplib -I../websocketpp -I../asio/asio/include -I../Vasicek

httpCreditRisk:main.o inputschema.o
	g++ -std=c++14 -O3 $(STATIC) -pthread main.o inputschema.o $(INCLUDES) -o httpCreditRisk -fopenmp

main.o: main.cpp CreditUtilities.h 
	g++ -std=c++14 -O3 $(STATIC) -pthread -c main.cpp   $(INCLUDES) -fopenmp

inputschema.o: inputschema.json
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 inputschema.json inputschema.o
clean:
	-rm *.o httpCreditRisk *.out

test: test.cpp CheckSchema.h
	g++ -std=c++14 -pthread test.cpp $(INCLUDES) -o test -fopenmp