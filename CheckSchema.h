#ifndef __CHECKSCHEMA_H_INCLUDED__
#define __CHECKSCHEMA_H_INCLUDED__
#include "document.h" //rapidjson
#include "writer.h" //rapidjson
#include "stringbuffer.h" //rapidjson
#include "schema.h"//rapidjson
#include "error/en.h" //rapidjson
#include <iostream>



bool handleSchema(const char* schemaJson, const char* inputJson, rapidjson::Document& d, bool supressError){
    rapidjson::Document sd;
    if (sd.Parse(schemaJson).HasParseError()) {
        if(!supressError){
            std::cout << "parse error for schema:" << rapidjson::GetParseError_En(sd.GetParseError()) << " offset:" << sd.GetErrorOffset() << std::endl;
        }
        return false;
    }
    rapidjson::SchemaDocument schema(sd); // Compile a Document to SchemaDocument

    if (d.Parse(inputJson).HasParseError()) {
        if(!supressError){
            std::cout << "parse error for json:" << rapidjson::GetParseError_En(sd.GetParseError()) << " offset:" << sd.GetErrorOffset() << std::endl;
        }
        return false;
        // the input is not a valid JSON.
        // ...       
    }
    rapidjson::SchemaValidator validator(schema);

    if (!d.Accept(validator)) {
        // Input JSON is invalid according to the schema
        // Output diagnostic information
         if(!supressError){
            rapidjson::StringBuffer sb;
            validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
       
            std::cout<<"Invalid schema: "<<sb.GetString()<<std::endl;
            std::cout<<"Invalid keyword: "<< validator.GetInvalidSchemaKeyword()<<std::endl;

            sb.Clear();
            validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
            std::cout<<"Invalid document: "<<sb.GetString()<<std::endl;
            sb.Clear();
        }
        
        //validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
            
        return false;
    }
    return true;
}

bool handleSchema(const char* schemaJson, const char* inputJson, rapidjson::Document& d){
    return handleSchema(schemaJson, inputJson, d, false);
}


#endif