//#include "advancedConsole.h"

#include "element-ui.h"

//#ifdef __WIN32
//int wmain() {
//#elif defined __linux__
//int main() {
//#endif
int main() {
	screen wscreen;
	s_screen = &wscreen;
	//textbox n(s_screen);
	//terminalElement tE;
	programRunner tE("echo", "Hello World!");
	nullElement nE;
	
	wscreen.child->add(&nE);
	nE.setc(0.0f,1.0f);
	nE.add(tE);
	
	std::thread screen_thread((void(*)(screen*))&screen::start, &wscreen);
	//wscreen.start();
	
	
	
	char buffer[128];
	FILE* pipe = popen("echo Hello World!", "r");

	if (!pipe) {
		puts("Internal: popen failed");
		return -1;			
	}
	
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL) {
			//terminalWrite(&buffer[0]);
			printf(&buffer[0]);
		}
	}
	
	pclose(pipe);
	
	
	
	
	//std::thread terminal_thread((void(*)(programRunner*))&programRunner::run, &tE);
	tE.run();
	
	screen_thread.join();
	//terminal_thread.join();
	
	/*
	while (!adv::ready);
	
	if (console::getImage() == IMAGE_LINUX) {
		//adv::setDrawingMode(DRAWINGMODE_COMPARE);
	}
	*/

	//raw();
	//keypad(stdscr, 1);

	//adv::allocate(); //Something buggy

	//messageDialog m({});
	//m.show("Hello world!");
	
	/*
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
	*/
		
	//adv::_advancedConsoleDestruct();
	
	//puts(file.c_str());
	
	//while (console::readKey()!=27);
	
	return 1;
}
