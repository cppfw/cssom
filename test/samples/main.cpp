#include "../../src/cssdom/document.hpp"

#include <clargs/parser.hpp>
#include <papki/fs_file.hpp>

int main(int argc, char** argv){
	bool help = false;
	std::string out_filename = "out.css";
	std::string in_filename;

	{
		clargs::parser p;

		p.add("help", "show help information", [&help](){help = true;});
		p.add('o', "out-file", "output filename", [&out_filename](std::string&& f){out_filename = std::move(f);});

		auto extras = p.parse(argc, argv);

		if(help){
			std::cout << "CSS parser test utility. Parses CSS into DOM and then writes out the DOM to CSS file." << std::endl;
			std::cout << "Usage:" << std::endl;
			std::cout << "  " << argv[0] << " [-o <out-css-filename>] <in-css-filename>" << std::endl;
			std::cout << "Arguments:" << std::endl;
			std::cout << p.description() << std::endl;
			return 0;
		}

		if(extras.empty()){
			std::cout << "error: no input filename given" << std::endl;
			return 1;
		}
		in_filename = extras.front();
	}

	auto doc = cssdom::read(papki::fs_file(in_filename));
	ASSERT_ALWAYS(doc.stuff.empty())
	//TODO:

	return 0;
}
