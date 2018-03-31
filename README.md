# Credit Model with streaming API
WIP.  This is to demo best in class model development for the sake of validation.  Large amounts of data are streamed and the program works with the data asynchronously.  

## Compile 

### Linux

Run `./compileAndPackage.sh`.  This will also compile and run the unit tests.  This will compile the binaries for static distribution (eg, onto any server with or without access to the standard libraries).

### Mac

It should work when using a gcc compatible with fopenmp...install using homebrew.  `brew install gcc --without-multilib`.  Then follow the linux commands.


## Assumptions

The API that is hit must communicate how many messages it will send and some aggregated statistics.  For an example, see `testServer.js`.

## Run

Run the testServer using `node testServer`.  Then run `node creditWrapper`.  The results will be written to `results.json`.  



