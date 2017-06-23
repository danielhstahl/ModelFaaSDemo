{
  "targets": [
    {
        "target_name": "buffer_example",
        "sources": [ "main.cpp" ], 
        "include_dirs" : ["<!(node -e \"require('nan')\")",
        "../FangOost",
        "../FunctionalUtilities", 
        "../rapidjson/include/rapidjson",
        "../Vasicek"     
        
        ]
    }
  ]
}