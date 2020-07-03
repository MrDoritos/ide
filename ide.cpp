#include "advancedConsole.h"
#include "element.h"

#ifdef __WIN32
int wmain() {
#elif defined __linux__
int main() {
#endif
	while (!adv::ready);
	
	if (console::getImage() == IMAGE_LINUX) {
		//adv::setDrawingMode(DRAWINGMODE_COMPARE);
	}

	//raw();
	//keypad(stdscr, 1);

	//adv::allocate(); //Something buggy

	//messageDialog m({});
	//m.show("Hello world!");
	
	adv::clear();
	int sel = 0,off = 0;
	//openFileDialog o({{}, mbx{0.5f, 0.5f, 30, 30}});
	openFileDialog o({fbx{0.5f,0.5f,0.75f,0.75f}});
	//openFileDialog o({10, 10, 40, 30});
	//openFileDialog o({adv::width / 2, adv::height / 2, 20, 20});
	std::string file;
	//adv::setThreadState(false);
	while (o.getFile(file)) {
		//We gots a file
		FILE* handle = fopen(file.c_str(), "rw");
		textEditor b(handle, {fbx{0.7f, 0.5f, 0.5f, 0.5f}});
		b.load();
		b.show();
		adv::clear();
	
	}
		
	//adv::_advancedConsoleDestruct();
	
	//puts(file.c_str());
	
	//while (console::readKey()!=27);
	
	return 1;
}
