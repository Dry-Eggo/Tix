#include "tix.hpp"
#include <cstdio>
#include <iostream>
#include <string>


namespace Tix {
    namespace Cli
    {
	TixCompiler* Parse_Argv(int c, char** v) {
	    TixCompiler* compiler = new TixCompiler();
	    auto len = c;
	    for (int i = 1; i < len; ++i) {
		std::string arg = v[i];
		if (arg == "compile") {
		    std::string source;
		    if (i + 1 >= len) {
			std::printf("Tix Error: No Source File Given");
			exit(1);
		    }
		    source = v[++i];
		    compiler->input_path = source;
		} else if (arg.substr(0, 2) == "-") {
		    if (arg == "--output" || arg == "-o") {
			if (i + 1 >= len) {
			    std::printf("Tix Error: No Output Path given");
			    exit(1);
			}
			compiler->output_path = v[++i];
		    }
		}
	    }
	    return compiler;
	}
    }
}
