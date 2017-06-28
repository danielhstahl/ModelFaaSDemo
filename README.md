# Credit Model with streaming API
WIP.  This is to demo best in class model development for the sake of validation.  Large amounts of data are streamed and the program works with the data asyncrounously.  

## Compile 

### Linux

Run `./compileAndPackage.sh`.  This will also compile and run the unit tests.  This will compile the binaries for static distribution (eg, onto any server with or without access to the standard libraries).

## Workarounds

The websocket library is class based instead of functional.  There are lots of "global" variables in that class.  Whenever a library comes on that supports functional programming and websocket while being header only and not relying on boost....then I'll switch.  

## Assumptions

The API that is hit must communicate how many messages it will send and some aggregated statistics.


