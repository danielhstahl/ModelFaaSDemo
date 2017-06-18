#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "CheckSchema.h"
#include "document.h" //rapidjson
#include "writer.h" //rapidjson
#include "stringbuffer.h" //rapidjson
#include "schema.h"//rapidjson
#include "error/en.h" //rapidjson
#include <string>

TEST_CASE("Returns false when not given proper schema", "[schema]"){
    std::string badSchema="hello";
    std::string goodJson="{\"url\":\"localhost\",\"port\":3000,\"endpoint\":\"hello\",\"params\":{\"xMin\":0,\"xMax\":4,\"y0\":[1,2,3],\"sigma\":[0.2, 0.3, 0.1],\"alpha\":[0.4,0.5,0.6],\"rho\":[[1,0.5,0.3],[-0.3,1,-0.4],[0.3,0.5,1]],\"tau\":1,\"lambda\":0.003,\"q\":400,\"uSteps\":256,\"xSteps\":1024,\"alphL\":0.4,\"bL\":0.5,\"sigL\":0.2,\"loans\":[{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02}]}}";
    rapidjson::Document parms;
    REQUIRE(handleSchema(badSchema.c_str(), goodJson.c_str(), parms)==false); 
}
TEST_CASE("Returns false when not given proper json", "[schema]"){
    std::string goodSchema="{\"$schema\":\"http://json-schema.org/draft-04/schema#\",\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\"},\"port\":{\"type\":\"integer\"},\"endpoint\":{\"type\":\"string\"},\"params\":{\"type\":\"object\",\"properties\":{\"xMin\":{\"type\":\"number\"},\"xMax\":{\"type\":\"number\"},\"y0\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"sigma\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"alpha\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"rho\":{\"type\":\"array\",\"items\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}}},\"tau\":{\"type\":\"number\"},\"lambda\":{\"type\":\"number\"},\"q\":{\"type\":\"number\"},\"uSteps\":{\"type\":\"integer\"}, \"xSteps\":{\"type\":\"integer\"}, \"alphL\":{\"type\":\"number\"},\"bL\":{\"type\":\"number\"},\"sigL\":{\"type\":\"number\"},\"loans\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"w\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"pd\":{\"type\":\"number\"},\"l\":{\"type\":\"number\"}}}}},\"additionalProperties\":false,\"required\":[\"xMin\",\"xMax\",\"y0\",\"alpha\",\"sigma\", \"rho\",\"tau\",\"lambda\",\"q\",\"alphL\",\"bL\",\"sigL\",\"loans\"]}},\"additionalProperties\":false,\"required\":[\"url\",\"port\",\"endpoint\",\"params\"]}";
    std::string badJson="Hello";
    rapidjson::Document parms;
    REQUIRE(handleSchema(goodSchema.c_str(), badJson.c_str(), parms)==false); 
}
TEST_CASE("Returns false when not given json that conforms to schema", "[schema]"){
    std::string goodSchema="{\"$schema\":\"http://json-schema.org/draft-04/schema#\",\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\"},\"port\":{\"type\":\"integer\"},\"endpoint\":{\"type\":\"string\"},\"params\":{\"type\":\"object\",\"properties\":{\"xMin\":{\"type\":\"number\"},\"xMax\":{\"type\":\"number\"},\"y0\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"sigma\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"alpha\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"rho\":{\"type\":\"array\",\"items\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}}},\"tau\":{\"type\":\"number\"},\"lambda\":{\"type\":\"number\"},\"q\":{\"type\":\"number\"},\"uSteps\":{\"type\":\"integer\"}, \"xSteps\":{\"type\":\"integer\"}, \"alphL\":{\"type\":\"number\"},\"bL\":{\"type\":\"number\"},\"sigL\":{\"type\":\"number\"},\"loans\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"w\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"pd\":{\"type\":\"number\"},\"l\":{\"type\":\"number\"}}}}},\"additionalProperties\":false,\"required\":[\"xMin\",\"xMax\",\"y0\",\"alpha\",\"sigma\", \"rho\",\"tau\",\"lambda\",\"q\",\"alphL\",\"bL\",\"sigL\",\"loans\"]}},\"additionalProperties\":false,\"required\":[\"url\",\"port\",\"endpoint\",\"params\"]}";
    std::string badJson="{\"hello\":\"world\"}";
    rapidjson::Document parms;
    REQUIRE(handleSchema(goodSchema.c_str(), badJson.c_str(), parms)==false); 
}
TEST_CASE("Returns true when  given json that conforms to schema", "[schema]"){
    std::string goodSchema="{\"$schema\":\"http://json-schema.org/draft-04/schema#\",\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\"},\"port\":{\"type\":\"integer\"},\"endpoint\":{\"type\":\"string\"},\"params\":{\"type\":\"object\",\"properties\":{\"xMin\":{\"type\":\"number\"},\"xMax\":{\"type\":\"number\"},\"y0\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"sigma\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"alpha\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"rho\":{\"type\":\"array\",\"items\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}}},\"tau\":{\"type\":\"number\"},\"lambda\":{\"type\":\"number\"},\"q\":{\"type\":\"number\"},\"uSteps\":{\"type\":\"integer\"}, \"xSteps\":{\"type\":\"integer\"}, \"alphL\":{\"type\":\"number\"},\"bL\":{\"type\":\"number\"},\"sigL\":{\"type\":\"number\"},\"loans\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"w\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"pd\":{\"type\":\"number\"},\"l\":{\"type\":\"number\"}}}}},\"additionalProperties\":false,\"required\":[\"xMin\",\"xMax\",\"y0\",\"alpha\",\"sigma\", \"rho\",\"tau\",\"lambda\",\"q\",\"alphL\",\"bL\",\"sigL\",\"loans\"]}},\"additionalProperties\":false,\"required\":[\"url\",\"port\",\"endpoint\",\"params\"]}";
    std::string goodJson="{\"url\":\"localhost\",\"port\":3000,\"endpoint\":\"hello\",\"params\":{\"xMin\":0,\"xMax\":4,\"y0\":[1,2,3],\"sigma\":[0.2, 0.3, 0.1],\"alpha\":[0.4,0.5,0.6],\"rho\":[[1,0.5,0.3],[-0.3,1,-0.4],[0.3,0.5,1]],\"tau\":1,\"lambda\":0.003,\"q\":400,\"uSteps\":256,\"xSteps\":1024,\"alphL\":0.4,\"bL\":0.5,\"sigL\":0.2,\"loans\":[{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02}]}}";
    rapidjson::Document parms;
    REQUIRE(handleSchema(goodSchema.c_str(), goodJson.c_str(), parms)==true); 
}