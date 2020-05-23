#include <mutex>
#include <thread>
#include <condition_variable>

#include "advancedConsole.h"
#ifdef __WIN32
#include "dirent.h"
#elif defined __linux__
#include <dirent.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

struct asyncDialog {
	std::mutex mux;
	std::condition_variable cond;
	std::thread worker;
	bool run;
};

struct fbx {
	fbx(float offNormX, float offNormY, float sizeNormX, float sizeNormY) {
		this->offNormX = offNormX;
		this->offNormY = offNormY;
		this->sizeNormX = sizeNormX;
		this->sizeNormY = sizeNormY;
	}
	
	float offNormX, offNormY, sizeNormX, sizeNormY;
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
	
	box(mbx m) {
		box b;
		//box(b, m);
		this->sizex = m.sizex;
		this->sizey = m.sizey;
		this->offsetx = b.offsetx + b.getOffsetX(m.normx, m.sizex);
		this->offsety = b.offsety + b.getOffsetY(m.normy, m.sizey);
	}
	
	box(fbx f) {
		box b;
		
		this->sizex = b.sizex / (1.0f / f.sizeNormX);
		this->sizey = b.sizey / (1.0f / f.sizeNormY);
		this->offsetx = b.getOffsetX(f.offNormX, this->sizex);
		this->offsety = b.getOffsetY(f.offNormY, this->sizey);
	}
	
	box(box& box) {
		this->offsetx = box.offsetx;
		this->offsety = box.offsety;
		this->sizex = box.sizex;
		this->sizey = box.sizey;
	}
	box() {
		sizex = adv::width;
		sizey = adv::height;
		offsetx = 0;
		offsety = 0;
		//sizex = adv::getOffsetX(0.5f);
		//offsetx = adv::getOffsetX(0.5f, sizex);
		//sizey = adv::getOffsetY(0.5f);
		//offsety = adv::getOffsetY(0.5f, sizey);
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
	
	void fill(char c, char color) {
		//for (int x = offsetx; x < offsetx + sizex; x++) {
		//	for (int y = offsety; y < offsety + sizey; y++) {
		//		adv::write(x, y, c, color);
		//	}
		//}
		adv::fill(offsetx, offsety, offsetx + sizex, offsety + sizey, c, color);
	}
	
	void border(char color) {
		adv::border(offsetx, offsety, offsetx + sizex - 1, offsety + sizey - 1, color);
	}
	
	void title(const char* title, char color) {
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
		fill(' ', BWHITE | FBLACK);
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
	openFileDialog() {
		setDirectory(".");
		setListOffset(0);
		setSelected(0);
	}
	
	openFileDialog(box box) : dialog(box) {
		setDirectory(".");
		setListOffset(0);
		setSelected(0);
	}
	
	void setListOffset(int x) {
		listOffset = x;
	}
	int getListOffset() {
		return listOffset;
	}
	void setDirectory(std::string dir) {
		getDirectoryFiles(dir.c_str());
	}
	int getSelected() {
		return selected;
	}
	void setSelected(int sel) {
		if (sel < files.size())
			selected = sel;
	}
	void getDirectory(std::string& dir) {
		dir = directory;
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
		//listOffset = 0;
		//selected = 0;
		//getDirectoryFiles(".");
		box tb(offsetx + 1, offsety + 1, sizex - 2, 1);
		textBox search (tb);
		
		std::string buffer;
		int key = 0;
		adv::setThreadState(false);
		do {
			switch (key) {				
				case VK_BACKSPACE: {
					if (buffer.size() > 0)
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
						f = files[selected].fullpath;
						return 1;
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
					//fprintf(stderr, "%i\r\n", key);
					if (key < ' ' || key > '~')
						break;
					buffer += key;
					break;
				}
			}
			fill(' ', BWHITE | FBLACK);
			border(BRED);
			title("Open file", BRED);
			displayFiles(buffer);
			search.show(buffer.c_str(), FRED | BBLACK | 0b00001000);
			adv::draw();
		} while ((key = console::readKey()) != VK_ESCAPE);
		adv::setThreadState(true);
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

	unsigned int edit_distance(const std::string& s1, const std::string& s2) {
		const std::size_t len1 = s1.size(), len2 = s2.size();
		std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

		d[0][0] = 0;
		for(unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
		for(unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

		for(unsigned int i = 1; i <= len1; ++i)
			for(unsigned int j = 1; j <= len2; ++j)
						  // note that std::min({arg1, arg2, arg3}) works only in C++11,
						  // for C++98 use std::min(std::min(arg1, arg2), arg3)
						  d[i][j] = std::min({ d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1) });
		return d[len1][len2];
	}
	
	void displayFiles(std::string& sort) {
		std::vector<file> files = this->files;
		if (sort.size() > 0) { //Levenstehen whatever it
			 std::sort(files.begin(), files.end(), [&](file& a, file& b) {
				return (edit_distance(a.name, sort) < edit_distance(b.name, sort));
			 });
		}
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
			om = snprintf(&buf[sizex - 6 < 0 ? 0 : sizex - 6], sizex - 1, "%s", f->directory ? "D   " : toStorage(f->size).c_str());
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

struct line {
	std::string buffer;
};

struct textEditor : dialog {
	textEditor() {}
	textEditor(box box) : dialog(box) {}
	#define MAXLINELEN 4096
	void load(FILE* handle) {
		char b;
		char buffer[MAXLINELEN];
		int r = 0;
		lines.clear();
		int i = 0;
		
		while ((fread(&b, 1, 1, handle)) != EOF) {
			if (b == '\n') { //New unix line
				line bl;
				bl.buffer = std::string(&buffer[0], r);
				r = 0;
				lines.push_back(bl);
				if (i++ > 20)
					break;
			} else {
				if (r >= MAXLINELEN)
					break;
				buffer[r++] = b;
			}
		}
	}
	
	void save(FILE* handle) {
	
	}
	
	void drawLine(line* line, int lineNumber, int x, int y, int maxX) {
		char buf[maxX];
		snprintf(&buf[0], maxX, "%i : %s", lineNumber, line->buffer.c_str());
		adv::write(x, y, &buf[0], FRED | BWHITE);
	}
	
	void show() {
		int key = 0;
		do {
			fill(' ', BWHITE | FBLACK);
			border(FBLACK | BRED);
			title("File", BRED | FBLACK);
			draw();
			adv::draw();
		} while ((key = console::readKey()) != VK_ESCAPE);
	}
	
	void draw() {
		for (int y = offsety + 1; y < sizey + offsety - 2; y++) {
			int lineNumber = y - offsety - 1;
			if (lineNumber >= lines.size())
				break;
			
			drawLine(&lines[lineNumber], lineNumber, offsetx + 1, y, sizex - 2);
		}
	}
	
	std::vector<line> lines;
};
