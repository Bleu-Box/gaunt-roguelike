// the game's GUI

class Menu {
 public:
	enum MenuItemCode {NONE, NEW_GAME, CONTINUE, EXIT};
	
	~Menu();
	
	void clear();
	void addItem(MenuItemCode code, const char* label);
	MenuItemCode pick();
	
 protected:
	struct MenuItem {
		MenuItemCode code;
		const char* label;
	};

	TCODList<MenuItem*> items;
};


class Gui {
 public:
	// messages sent to the GUI can be categorized into a few types
	// the type dictates what color the message will be
	enum MessageType {OBSERVE, ACTION, ATTACK};
	Menu menu;
	
	Gui();
	~Gui();

	void render();
	void message(MessageType, const char* text, ...);
	void clear();
	
 protected:
	TCODConsole* console;

	struct Message {
		char* text;
		TCODColor color;
		Message(const char* text, const TCODColor& color);
		~Message();
	};
	
	TCODList<Message*> log;
	
	void renderBar(int x, int y, int width, const char* name,
		       float value, float maxValue, const TCODColor& barColor,
		       const TCODColor& backColor);

	void renderMouseLook();
};
