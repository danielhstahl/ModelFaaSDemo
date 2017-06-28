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
cloneAndCheckout FangOost 
cloneAndCheckout Vasicek 
git clone https://github.com/miloyip/rapidjson
git clone https://github.com/miloyip/rapidjson
git clone https://github.com/chriskohlhoff/asio

compile 

rm -rf FunctionalUtilities
rm -rf FangOost
rm -rf Vasicek
rm -rf rapidjson
rm -rf websocketpp
rm -rf asio

