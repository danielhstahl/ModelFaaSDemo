{
  "targets": [
    {
        "target_name": "buffer_example",
        "sources": [ "main.cpp", "<(module_root_dir)/serverschema.o", "<(module_root_dir)/inputschema.o"],
        #"dependencies":["inputschema", "serverschema"],
       # "link_settings":{
        #     "libraries":["./inputschema.o", "./serverschema.o"]
        #},
        "include_dirs" : ["<!(node -e \"require('nan')\")",
        "../FangOost",
        "../FunctionalUtilities", 
        "../rapidjson/include/rapidjson",
        "../Vasicek"     
        
        ],
        'conditions': [
          ['OS=="win"', {
            'cflags': [
              '',
            ],
          }, { # OS != "win"
            'cflags_cc': [
              '-std=c++14',
              '-O3',
              '-pthread',
              '-fopenmp'
            ],
          }],
        ],
    }#,{
     #   "target_name":"inputschema",
     #   "type": "<(library)",
     #   "sources":["<(module_root_dir)/inputschema.o"]
    #},{
    #    "target_name":"serverschema",
    #    "type": "<(library)",
    #    "sources":["<(module_root_dir)/serverschema.o"]
    #}
  ]
}