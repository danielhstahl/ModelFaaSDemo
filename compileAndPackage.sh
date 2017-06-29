#!/bin/bash
function cloneAndCheckout {
	git clone https://github.com/phillyfan1138/$1
	cd $1
	cd ..
}
function editMake {
	if [ "$(uname)" == "Darwin" ]; then 
		sed -i '' -e "1s/^/STATIC=-static-libstdc++^M/" makefile 
		sed -i '' -e "s#\.\./#./#g" makefile 
	else 
		sed -i  "1s/^/STATIC=-static-libstdc++\n/" makefile 
		sed -i  "s#\.\./#./#g" makefile 
	fi
}
function undoMake {
	if [ "$(uname)" == "Darwin" ]; then 
		sed -i '' -e "s#STATIC=-static-libstdc++##g" makefile 
		sed -i '' -e "/^\s*$/d" makefile
		sed -i '' -e "s#\./#\.\./#g" makefile 
	else
		sed -i "s#STATIC=-static-libstdc++##g" makefile 
		sed -i "/^\s*$/d" makefile
		sed -i "s#\./#\.\./#g" makefile 
	fi
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
git clone https://github.com/chriskohlhoff/asio
compile 

rm -rf FunctionalUtilities
rm -rf FangOost
rm -rf Vasicek
rm -rf rapidjson
rm -rf websocketpp
rm -rf asio

