#!/bin/bash
function cloneAndCheckout {
	git clone https://github.com/phillyfan1138/$1
	cd $1
	cd ..
}
function editMake {
	sed -i "1s/^/STATIC=-static-libstdc++\n/" makefile 
}
function compile {
	editMake
	make 
	make test
	./test
}
rm -rf bin
mkdir bin

cloneAndCheckout FunctionalUtilities 
cloneAndCheckout CharacteristicFunctions 
cloneAndCheckout FangOost 
cloneAndCheckout TupleUtilities 
git clone https://github.com/miloyip/rapidjson
git clone https://github.com/yhirose/cpp-httplib
git clone https://github.com/yhirose/websocketpp
git clone https://github.com/chriskohlhoff/asio
compile 

rm -rf FunctionalUtilities
rm -rf CharacteristicFunctions
rm -rf FangOost
rm -rf TupleUtilities
rm -rf rapidjson
rm -rf cpp-httplib
rm -rf websocketpp
rm -rf asio
#serverless deploy -v
