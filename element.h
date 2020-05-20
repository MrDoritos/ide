#include <mutex>
#include <thread>
#include <condition_variable>

#include "advancedConsole.h"
#include "dirent.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

struct asyncDialog {
	std::mutex mux;
	std::condition_variable cond;
	std::thread worker;
	bool run;
};

struct mbx {
	mbx(float normx, float normy, int sizex, int sizey) {
		this->normx = normx;
		this->normy = normy;
		this->sizex = sizex;
		this->sizey = sizey;
	}
	
	float normx, normy;
	int sizex, sizey;
};

struct box {
	box(box* b, mbx m) {
		this->sizex = m.sizex;//b->getOffsetX(m.normx, m.sizex);
		this->sizey = m.sizey;//b->getOffsetY(m.normy, m.sizey);
		this->offsetx = b->offsetx + b->getOffsetX(m.normx, m.sizex);
		this->offsety = b->offsety + b->getOffsetY(m.normy, m.sizey);
	}
	
	box(box b, mbx m) {
		box(&b, m);
	}
	
	box(box& box) {
		this->offsetx = box.offsetx;
		this->offsety = box.offsety;
		this->sizex = box.sizex;
		this->sizey = box.sizey;
	}
	
	box() {
		sizex = adv::getOffsetX(0.5f);
		offsetx = adv::getOffsetX(0.5f, sizex);
		sizey = adv::getOffsetY(0.5f);
		offsety = adv::getOffsetY(0.5f, sizey);
	}
	
	box(int offsetx, int offsety, int sizex, int sizey) {
		this->offsetx = offsetx;
		this->offsety = offsety;
		this->sizex = sizex;
		this->sizey = sizey;
	}
		
	//Get position of the advancedConsole's buffer within the bounds of this box
	//Should use adv::get
	int get(int x, int y) {
		int cx = x + offsetx;
		int cy = y + offsety;
		return (cy * adv::width) + cx;
	}
	
	//check if bound to this box's size
	bool bound(int x, int y) {
		if (offsetx > x || sizex <= x)
			return false;
		if (offsety > y || sizey <= y)
			return false;
		return true;
	}
	
	int getOffsetX(float scale) {
		return getOffsetX(scale, 0);
	}
	
	int getOffsetY(float scale) {
		return getOffsetY(scale, 0);
	}
	
	int getOffsetX(float scale, float length) {
		if (length < 1) {
			return (float(sizex) * scale);
		}
		if (length > sizex)
			return 0.0f;
		
		return (float(sizex) * scale) - (length / 2);
	}
	
	int getOffsetY(float scale, float length) {
		if (length < 1) {
			return (float(sizey) * scale);
		}
		if (length > sizey)
			return 0.0f;
		
		return (float(sizey) * scale) - (length / 2);
	}
	
	void clip(int &x, int &y) {
		
	}
	
	int offsetx, offsety, sizex, sizey;
	char character, color;
};

struct dialog : public box {
	dialog() 
	{}
	
	dialog(box& mbox)
		:box(mbox)
	{}
	
	fill(char c, char color) {
		adv::fill(offsetx, offsety, offsetx + sizex, offsety + sizey, c, color);
	}
	
	border(char color) {
		adv::border(offsetx, offsety, offsetx + sizex, offsety + sizey, color);
	}
	
	title(const char* title, char color) {
		int l = strlen(title);
		{
			char buf[l + 3];
			int i = snprintf(&buf[0], l + 3, "[%s]", title);
			adv::write(offsetx + getOffsetX(0.5f, l), offsety, &buf[0], color);
		}
	}
};

struct button : dialog {
	button(box box)
		:dialog(box) 
	{}
		
	void show(const char* message, char color) {
		//adv::fill(offsetx, offsety, offsetx + sizex, offsety + sizey, ' ', BWHITE);
		fill(' ', BWHITE);
		//adv::rectangle(offsetx, offsety, offsetx + sizex, offsety + sizey, ' ', FRED | BWHITE | 0b00001000);
		border(FRED | 0b00001000 | BWHITE);
		int l = strlen(message);
		adv::write(getOffsetX(0.5f, l) + offsetx, getOffsetY(0.5f) + offsety, message, color);
	}
};

#define TEXTMODE_WRAP 0
#define TEXTMODE_CLIP 1 
#define TEXTMODE_IGNORE 2
#define TEXTMODE_END 3

//Wrapping and stuff like that
struct textBox : dialog {
	textBox(box box)
		:dialog(box)
	{}
		
	void show(const char* text, char color = FWHITE | BBLACK, int mode = TEXTMODE_END) {
		fill(' ', color);
		int l = strlen(text);
		
		if (l < sizex - 3) { //Fine, just show the text
			adv::write(offsetx, offsety, text, color);
		} else 
		if (l < sizex - 2) { //Right before the limit, show a block
			adv::write(offsetx, offsety, text, color);
			adv::write(offsetx + l, offsety, ' ', BWHITE | FBLACK);
		} else { //At the limit, clip text and show a block
			char clipped[l];
			strcpy(&clipped[0], text);
			clipped[sizex - 2] = '\0';
			adv::write(offsetx, offsety, text, color);
			adv::write(offsetx + sizex, offsety, ' ', BWHITE | FBLACK);
		}
	}
};

struct messageDialog : dialog {
	messageDialog(box box)
		:dialog(box)
	{}
	
	messageDialog() {}
	
	void show(const char* message) {
		//adv::fill(offsetx, offsety, offsetx + sizex, offsety + sizey, ' ', BWHITE);
		fill(' ', BWHITE);
		//adv::rectangle(offsetx, offsety, offsetx + sizex, offsety + sizey);
		border(FRED | 0b00001000 | BWHITE);
		button ok({this, mbx{0.5f, 0.8f, 10, 5}});
		ok.show("Ok", FRED | 0b00001000 | BWHITE);
		int key;
		textBox tb({});
		tb.show(message, FRED | 0b00001000 | BWHITE);
		while ((key = console::readKey()) != ' ' && key != VK_ENTER && key != VK_ESCAPE);
	}
	
	void show(const std::string string) {
		show(string.c_str());
	}
		
	private:
		
};

struct openFileDialog : public dialog {
	openFileDialog() {}
	
	openFileDialog(box box) : dialog(box) {
		
	}
	struct file {
		bool selected;
		bool highlighted;
		long size;
		std::string name;
		std::string fullpath;
		bool directory;
	};
	std::string directory;
	int listOffset;
	int selected;
	
	int getFile(std::string& f) {
		listOffset = 0;
		selected = 0;
		getDirectoryFiles(".");
		box tb(offsetx + 1, offsety + 1, sizex - 1, 1);
		textBox search (tb);
		
		std::string buffer;
		int key = 0;
		
		do {
			switch (key) {				
				case 8: {
					buffer.pop_back();
					break;
				}			
				case VK_UP: {
					if (--selected < 0)
						selected = files.size() - 1;
					break;
				}
				case VK_DOWN: {
					selected = ++selected % files.size();
					break;
				}
				case VK_RETURN: {
					if (files[selected].directory) {				
						//messageDialog b;
						//b.show("Opened: " + files[selected].fullpath);	
						getDirectoryFiles(files[selected].fullpath.c_str());
						selected = 0;
					} else {
						//We selected files
					}
					break;
				}
				case VK_LEFT:
				case VK_RIGHT: {
					files[selected].selected = !files[selected].selected;
					break;
				}
				case 0: {
					break;
				}
				default: {
					if (key < ' ' || key > '~')
						break;
					buffer += key;
					break;
				}
			}
			fill(' ', BWHITE);
			border(BRED);
			title("Open file", BRED);
			displayFiles();
			search.show(buffer.c_str(), FRED | BBLACK | 0b00001000);			
		} while ((key = console::readKey()) != 27);
		
		return 0;
	}	
	
	std::string toStorage(size_t length) {
		std::string l = std::to_string(length);
		std::string ot;
		for (int i = 0; i < 3; i++)
			if (i < l.length())
				ot += l[i];
			else 
				ot += " ";
		if (length < 1000)		   ot += "B";
		else if (length < 1000000) ot += "K";
		else if (length < 1000000000) ot += "M";
		else if (length < 1000000000000) ot += "G";
		else if (length < 1000000000000000) ot += "T";
		else ot += "P";
		return ot;
	}
	
	void displayFiles() {
		char buf[sizex + 2];
		buf[sizex + 1] = '\0';
		
		if (listOffset > sizey - 2 - files.size()) listOffset = 0;
		else listOffset = selected > 5 ? selected - 5 : 0;
		
		for (int i = 0; i < sizey - 3 && i < files.size() - listOffset; i++) {
			file* f = &files[i + listOffset];
			char sel = f->selected ? 'x' : ' ';
			memset(&buf, ' ', sizex + 1);
			buf[sizex - 1] = '\0';
			int om = snprintf(&buf[0], sizex - 3, "[%c] %s", sel, f->name.c_str());
			buf[om] = ' ';
			om = snprintf(&buf[sizex - 5], sizex - 1, "%s", f->directory ? "D   " : toStorage(f->size).c_str());
			char color = i == selected - listOffset ? BGREEN | FBLACK : BWHITE | FBLACK;
			adv::write(offsetx + 1, offsety + i + 2, &buf[0], color);
		}
	}
	
	std::vector<file> files;
	
	std::string getMultipleFiles() {
		
	}
	
	private:
	void getDirectoryFiles(const char* pname) {
		DIR* dir;
		struct dirent* entry;
		
		auto getPath = [](const char* path) {
			char resolvedPath[PATH_MAX];
			#ifdef __linux__
			realpath(path, resolvedPath);
			#elif defined __WIN32
			GetFullPathNameA(path, PATH_MAX, &resolvedPath[0], 0);
			#endif
			return std::string(resolvedPath);
		};
		
		std::string nnn = getPath(pname);
		const char* name = nnn.c_str();
		
		if (!(dir = opendir(name))) {
			messageDialog p;
			p.show("Could not open directory: " + std::string(name));
			p.show("Could not open directory: " + nnn);
			return;
		}
		
		files.clear();
				
		file dup;
		file dthis;
		dthis.directory = dup.directory = true;
		
		dup.name = std::string("..");
		dup.fullpath = getPath((std::string(name) + "/..").c_str());
		dthis.name = std::string(".");
		dthis.fullpath = getPath((std::string(name) + "/.").c_str());
		
		dup.selected = dthis.selected = false;
		dup.highlighted = dthis.highlighted = false;
		dup.size = 0; dthis.size = 0;
		files.push_back(dthis);
		files.push_back(dup);
				
		directory = getPath(name);
		
		
		while ((entry = readdir(dir)) != nullptr) {
			file f;
			struct stat statt;
			f.name = std::string(entry->d_name);
			if (f.name == "." || f.name == "..")
				continue;
			//if (f.directory = entry->d_type == DT_DIR)
			//	f.name += "/";
			f.directory = entry->d_type == DT_DIR;
			f.selected = f.highlighted = false;			
			f.fullpath = std::string(name) + "/" + f.name;
			stat(f.fullpath.c_str(), &statt);
			f.size = statt.st_size;
			files.push_back(f);
		}
				
		if (files.size() > 0)
			files[0].highlighted = true;
			
		closedir(dir);
	}
};

struct saveFileDialog {
	saveFileDialog() {
		
	}
	
	std::string getFile() {
		
	}
};