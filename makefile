INCLUDES= -I../FangOost -I../FunctionalUtilities -I../rapidjson/include/rapidjson -I../websocketpp -I../asio/asio/include -I../Vasicek

httpCreditRisk:main.o inputschema.o serverschema.o 
	g++ -std=c++14 -O3 $(STATIC) -pthread main.o inputschema.o serverschema.o $(INCLUDES) -o httpCreditRisk -fopenmp

main.o: main.cpp CreditUtilities.h 
	g++ -std=c++14 -O3 $(STATIC) -pthread -c main.cpp   $(INCLUDES) -fopenmp

inputschema.o: inputschema.json
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 inputschema.json inputschema.o

serverschema.o: serverschema.json
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 serverschema.json serverschema.o

#easywsclient.o: easywsclient.cpp easywsclient.hpp
	#g++ -std=c++14 -O3 $(STATIC) -c easywsclient.cpp -o easywsclient.o

clean:
	-rm *.o httpCreditRisk *.out

test: test.cpp CheckSchema.h
	g++ -std=c++14 -pthread test.cpp $(INCLUDES) -o test -fopenmp