#include "advancedConsole.h"
#include "element.h"

int main() {
	while (!adv::ready);
	
	if (console::getImage() == IMAGE_LINUX) {
		adv::setDrawingMode(DRAWINGMODE_COMPARE);
	}
	
	//messageDialog m({});
	//m.show("Hello world!");
	
	adv::clear();
	
	openFileDialog o({0,0,99,99});
	std::string file;
	if (!o.getFile(file))
		puts("No file");
		
	adv::_advancedConsoleDestruct();
	
	//puts(file.c_str());
	
	//while (console::readKey()!=27);
	
	return 0;
}