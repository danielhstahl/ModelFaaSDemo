#!/bin/bash
function cloneAndCheckout {
	git clone https://github.com/phillyfan1138/$1
	cd $1
	cd ..
}
function editMake {
	sed -i "1s/^/STATIC=-static-libstdc++\n/" makefile 
	sed -i "s#\.\./#./#g" makefile 
}
function undoMake {
	sed -i "s#STATIC=-static-libstdc++##g" makefile 
	sed -i "/^\s*$/d" makefile
	sed -i "s#\./#\.\./#g" makefile 
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
cloneAndCheckout Vasicek 
git clone https://github.com/miloyip/rapidjson
git clone https://github.com/yhirose/cpp-httplib
#git clone https://github.com/dhbaird/easywsclient
#wget https://raw.github.com/dhbaird/easywsclient/master/easywsclient.hpp
#wget https://raw.github.com/dhbaird/easywsclient/master/easywsclient.cpp
compile 

rm -rf FunctionalUtilities
rm -rf CharacteristicFunctions
rm -rf FangOost
rm -rf Vasicek
rm -rf rapidjson
rm -rf cpp-httplib
#rm -rf easywsclient
#rm -rf asio
#serverless deploy -v
