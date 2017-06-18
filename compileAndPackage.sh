#!/bin/bash
function cloneAndCheckout {
	git clone https://github.com/phillyfan1138/$1
	cd $1
	cd ..
}
function editMake {
	sed -i "1s/^/STATIC=-static-libstdc++\n/" makefile 
	sed -i "../=./" makefile 
}
function undoMake {
	sed -i "-static-libstdc++\n=/1s/^/STATIC/" makefile 
	sed -i "./=../" makefile 
}
function compile {
	editMake
	make 
	make test
	./test
	undoMake
}

cloneAndCheckout FunctionalUtilities 
cloneAndCheckout CharacteristicFunctions 
cloneAndCheckout FangOost 
git clone https://github.com/miloyip/rapidjson
git clone https://github.com/yhirose/cpp-httplib
git clone https://github.com/yhirose/websocketpp
git clone https://github.com/chriskohlhoff/asio
compile 

rm -rf FunctionalUtilities
rm -rf CharacteristicFunctions
rm -rf FangOost
rm -rf rapidjson
rm -rf cpp-httplib
rm -rf websocketpp
rm -rf asio
#serverless deploy -v
