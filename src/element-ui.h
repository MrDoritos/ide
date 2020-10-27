#pragma once

#include "screen.h"
#include "element.h"

static void* s_screen;

struct screen;

template<typename T>
struct helperElement : public element {
	void* derived;
	helperElement(void* derived)
		:element(s_screen) {
			this->derived = derived;
			if (derived)
				handler.setReferer(derived);
			if (!s_screen)
				fprintf(stderr, "Warning! s_screen not set\r\n");
			handler.assign(FLG_CLEAR, func(T, T::onClear));
			handler.assign(FLG_CLICK, func(T, T::onClick));
			handler.assign(FLG_CLOSE, func(T, T::onClose));
			handler.assign(FLG_CREATE, func(T, T::onCreate));
			handler.assign(FLG_FOCUS, func(T, T::onFocus));
			handler.assign(FLG_FRAME, func(T, T::onFrame));
			handler.assign(FLG_KEYHELD, func(T, T::onKeyheld));
			handler.assign(FLG_KEYPRESS, func(T, T::onKeypress));
			handler.assign(FLG_KEYRELEASE, func(T, T::onKeyrelease));
			handler.assign(FLG_RESIZE, func(T, T::onResize));
			handler.assign(FLG_UPDATE, func(T, T::onUpdate));
		}
		
	virtual void onClear() {}
	virtual void onClick() {}
	virtual void onClose() {}
	virtual void onCreate() {}
	virtual void onFocus() {}
	virtual void onFrame() {}
	virtual void onKeyheld() {}
	virtual void onKeypress() {}
	virtual void onKeyrelease() {}
	virtual void onResize() {}
	virtual void onUpdate() {}
};

struct nullElement : public helperElement<nullElement> {
	nullElement():helperElement(this) {
		setBackground('.', FBLUE | BWHITE);
	}
};

struct terminalElement : public helperElement<terminalElement> {
	terminalElement():helperElement(this) {
		xCur = 0;
		yCur = 0;
		doUseScreen(false);//Signal the framebuffer to copy from an internal buffer instead of writing directly to the screen
		doUseClear(false); //Signal the framebuffer to not clear the written buffer at frametime
		doUseBorder(false);
	}
	
	int xCur, yCur;
	
	void onCreate() override {
		setc(parent->getc(0.25f,0.5f));
		clear(true);
	}
	
	void onResize() override {
		//setc(0.25f,0.25f,0.5f,0.5f);
		clear(true);
	}
	
	void terminalWriteLine(const char* str) {
		size_t linelen = strlen(str);
		char newStr[linelen + 3];
		memset(&newStr[0], 0, linelen + 3);
		memcpy(&newStr[0], str, linelen);
		newStr[linelen] = '\r';
		newStr[linelen+1] = '\n';
		terminalWrite(&newStr[0]);
	}
	
	void terminalNewLine() {
		if (yCur + 2 > getSizeY()) {
			char* nCb = new char[getCount()];
			wchar_t* nFb = new wchar_t[getCount()];
			memset(nCb, 0, getCount());
			memset(nFb, 0, getCount() * 2);
			
			//Copy current framebuffer to new buffers, omitting first row
			//TODO know if border is drawn
			for (int y = 0; y < getSizeY()-1; y++) {
				for (int x = 0; x < getSizeX(); x++) {
					nCb[(y * getSizeX()) + x] = cb[((y+1) * getSizeX()) + x];
					nFb[(y * getSizeX()) + x] = fb[((y+1) * getSizeX()) + x];
				}
			}
						
			delete cb;
			delete fb;
			
			cb = nCb;
			fb = nFb;		
		} else {			
			if (yCur - 1 < getSizeY())
				yCur++;
		}
	}
	
	void terminalWrite(const char* str) {
		size_t linelen = strlen(str);
		for (int i = 0; i < linelen; i++) {
			switch (str[i]) {
				case '\r':
					xCur = 0;
					break;
				case '\n':
					terminalNewLine();
					//Linux does not use carriage return
					xCur = 0;
					break;
				default:
					if (xCur + 1 > getSizeX()) {
						terminalNewLine();
						xCur = 0;
					}
					write(xCur++, yCur, str[i], FWHITE | BBLACK);
					break;
			}
		}
	}
	
	int test;
	
	void onUpdate() override {
		//write(0,0,char((('0' + ++test) % '9') + '0'), FWHITE | BBLACK);
		//terminalWrite((std::string("Ian's World") + std::to_string(test++%20)).c_str());
	}
};

struct programRunner : terminalElement {
	const char* path;
	const char* args;
	
	programRunner(const char* path, const char* args)
		:path(path),args(args) 
	{	}
	
	int run() {
		//fprintf(stderr, "Running %s %s\r\n", path, args);
		char buffer[128];
		std::string strPath = std::string(path);
		std::string strArgs = std::string(args);
		std::string strCmd = strPath + " " + strArgs;
		
		FILE* pipe = popen(strCmd.c_str(), "r");
		if (!pipe) {
			terminalWriteLine("Internal: popen failed");
			return -1;			
		} else {
			//fprintf(stderr, "Running...\r\n");
		}
		
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL) {
				terminalWrite(&buffer[0]);
			}
		}
		
		//fprintf(stderr, "Finished Execution of Sub Process\r\n");
		
		pclose(pipe);
		return 0;
	}
};

struct textbox : public element {
	textbox(void* screen)
		:element(screen)
		{
			tick = 0;
			tickCount = 20;
			handler.setReferer(this);
			handler.assign(FLG_UPDATE, func(textbox, textbox::update));
			handler.assign(FLG_FRAME, func(textbox, textbox::frame));
			handler.assign(FLG_KEYPRESS, func(textbox, textbox::keypress));
			handler.assign(FLG_CREATE, func(textbox, textbox::create));
			setSize(10,10);
			setOffset(0,0);
			setBackground('#', BWHITE | FBLACK);
			clear();
			doUseScreen(true);
			//doUseBorder(true);
			setBorder(FBLUE | BBLACK, false);
		}
		
	void create() {
		setc(parent->getc(0.25f, 0.5f));
	}
		
	void keypress() {
		//if (!focused())
		//	return;
		switch (NOMOD(screen->pressed)) {
			case VK_BACKSPACE:
			{
				if (sbuffer.length() > 0)
					sbuffer.pop_back();
				break;
			}			
			default:
			{
				if (NOMOD(screen->pressed) < ' ' || NOMOD(screen->pressed) > '~')
					break;
				sbuffer += NOMOD(screen->pressed);
				setBackground(' ', ((char)screen->pressed + rand()) % 255);
			}
		}
	}
		
	void update() {
	}
	
	void frame() {
		//if (focused())
		//	drawFancyBorder(CHARACTER_BORDER, FRED | BBLACK);
				
		for (int i = 0; i < sbuffer.length(); i++)
			write(i % getSizeX(), i / getSizeX(), sbuffer[i], FWHITE | BBLACK);
		
		tick++;
		if (tick > 0)
			write(sbuffer.length() % getSizeX(), sbuffer.length() / getSizeX(), '|', FWHITE | BBLACK);
		else 
			write(sbuffer.length() % getSizeX(), sbuffer.length() / getSizeX(), ' ', FWHITE | BBLACK);
		
		if (tick > tickCount)
			tick = -tickCount;
	}
	
	std::string sbuffer;
	
	int tick;
	int tickCount;
};
