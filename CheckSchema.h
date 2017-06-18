#include "document.h" //rapidjson
#include "writer.h" //rapidjson
#include "stringbuffer.h" //rapidjson
#include "schema.h"//rapidjson
#include "error/en.h" //rapidjson
#include <iostream>

bool handleSchema(const char* schemaJson, const char* inputJson, rapidjson::Document& d){
    rapidjson::Document sd;
    if (sd.Parse(schemaJson).HasParseError()) {
        std::cout << "parse error for schema:" << rapidjson::GetParseError_En(sd.GetParseError()) << " offset:" << sd.GetErrorOffset() << std::endl;
        return false;
    }
    rapidjson::SchemaDocument schema(sd); // Compile a Document to SchemaDocument

    if (d.Parse(inputJson).HasParseError()) {
        std::cout << "parse error for json:" << rapidjson::GetParseError_En(sd.GetParseError()) << " offset:" << sd.GetErrorOffset() << std::endl;
        return false;
        // the input is not a valid JSON.
        // ...       
    }
    rapidjson::SchemaValidator validator(schema);

    if (!d.Accept(validator)) {
        // Input JSON is invalid according to the schema
        // Output diagnostic information
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        std::cout<<"Invalid schema: %s\n"<<sb.GetString()<<std::endl;
        printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        printf("Invalid document: %s\n", sb.GetString());
        return false;
    }
    return true;
}