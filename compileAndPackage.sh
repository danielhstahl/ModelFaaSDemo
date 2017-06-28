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
<<<<<<< HEAD
=======
git clone https://github.com/miloyip/rapidjson
git clone https://github.com/chriskohlhoff/asio

>>>>>>> be35eff39c3793f2585c7c10b7d4669774ba8e7e
compile 

rm -rf FunctionalUtilities
rm -rf FangOost
rm -rf Vasicek
rm -rf rapidjson
<<<<<<< HEAD
#rm -rf easywsclient
#rm -rf asio
#serverless deploy -v
=======
rm -rf websocketpp
rm -rf asio

>>>>>>> be35eff39c3793f2585c7c10b7d4669774ba8e7e
