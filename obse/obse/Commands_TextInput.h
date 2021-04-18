#pragma once

#include "CommandTable.h"

extern CommandInfo kCommandInfo_OpenTextInput;
extern CommandInfo kCommandInfo_GetInputText;
extern CommandInfo kCommandInfo_CloseTextInput;
extern CommandInfo kCommandInfo_UpdateTextInput;
extern CommandInfo kCommandInfo_IsTextInputInUse;
extern CommandInfo kCommandInfo_GetTextInputControlPressed;
extern CommandInfo kCommandInfo_InsertInInputText;
extern CommandInfo kCommandInfo_DeleteFromInputText;
extern CommandInfo kCommandInfo_MoveTextInputCursor;
extern CommandInfo kCommandInfo_GetTextInputCursorPos;

extern CommandInfo kCommandInfo_SetTextInputDefaultControlsDisabled;
extern CommandInfo kCommandInfo_SetTextInputControlHandler;
extern CommandInfo kCommandInfo_SetInputText;

//////////////////////////////////////////////////////////////////////////
// Support for simple text input. Controls:
//  -left/right -> move cursor. Ctrl moves to beginning of closest word
//	-up/down -> in books/scrolls, moves to end/beginning of previous/next line
//  -back/del -> delete. Ctrl deletes an entire word
//  -home/end -> move to beginning/end of text
//  -tab -> inserts 4 spaces
// <, >, and ~ characters are disallowed
// HTML formatting inserted with ctrl + one of the following:
//  -c, l, r -> align center, left, or right (applies to entire line)
//  -1, 2, 3, 4, 5 -> set font (applies to entire string)
//
// Some possible TODO stuff:
//  -Figure out how to determine when page needs to be turned in books, or scroll text needs to scroll down,
//   or messagebox is full (Add scrollbars? possible...)
///////////////////////////////////////////////////////////////////////////

#if OBLIVION

#include "GameForms.h"
#include "GameObjects.h"
#include "GameMenus.h"
#include "Hooks_Gameplay.h"
#include "GameOSDepend.h"

// base class for various text input menus
// Functions which manipulate the input text return new insertion point
// Code assumes opening and closing brackets are always matched correctly and that FONT and DIV tags are not removed
// from beginning of input string (TextInputHandler::InsertInText() checks for invalid tags so this is
// a safe assumption)
class TextInputMenu
{
protected:
	enum {
		kHTMLTag_DIV_Left,
		kHTMLTag_DIV_Right,
		kHTMLTag_DIV_Center,
		kHTMLTag_BR,
		kHTMLTag_FONT,
		kHTMLTag_Other,
	};

	std::string m_promptText;
	std::string m_inputText;
	UInt32 m_maxLen;
	UInt32 m_minPos;
	bool m_bIsHTML;

	UInt32 FindHTMLMatchedBracket(UInt32 startBracketPos, bool bBackwards) const;
	UInt32 FindWordBoundary(UInt32 startPos, bool bBackwards) const;
	UInt32 FindLineStart(UInt32 fromPos) const;
	UInt32 FindLineEnd(UInt32 fromPos) const;
	bool   IsCurrentLineEmpty(UInt32 fromPos) const;
	UInt32 GetHTMLTagType(UInt32 tagStartPos) const;

public:
	TextInputMenu(UInt32 maxLen, bool bIsHTML);
	virtual	~TextInputMenu();
	virtual void Update() = 0;								//refreshes the menu when text changes
	virtual void Init() = 0;								//Initializes and displays menu after construction
	virtual UInt32 ResolveControlCode(UInt8 controlCode, UInt32 insertPos) = 0;	//translate a control-character to text
	virtual bool IsOpen() const = 0;						//returns false if menu was closed by user

	UInt32 InsertChar(UInt32 insertPos, char toInsert);			//returns position one past the end of toInsert
	UInt32 InsertText(UInt32 insertPos, const char* toInsert);	//as above
	UInt32 DeleteText(UInt32 startPos, bool bBackwards = false, bool bDeleteWord = false);
	UInt32 SeekPosition(UInt32 fromPos, bool bBackwards, bool bSeekWordStart) const;
	void   Erase(UInt32 pos);
	UInt32 GetInputLength()	const		{	return m_inputText.length();	}
	bool   IsFull()	const				{	return m_inputText.length() >= m_maxLen;	}
	std::string GetInputText() const	{	return m_inputText;	}
	UInt32 GetMaxPos() const			{	return m_inputText.length() - 1;	}
	UInt32 GetMinPos() const			{	return m_minPos;	}
	bool IsHTML() const					{	return m_bIsHTML;	}
	void SetText(const std::string& text) { m_inputText = text; }
};

//A messagebox
class TextInputMessageBox : public TextInputMenu
{
	char		* m_fmtString;
	UInt32		m_scriptRefID;
	bool		m_bShortcutsDisabled;
	Script		* m_script;

public:
	TextInputMessageBox(char* fmtString, UInt32 maxLen, Script* callingScript, TESObjectREFR* callingObj);
	virtual ~TextInputMessageBox();
	virtual void Update();
	virtual void Init();
	virtual UInt32 ResolveControlCode(UInt8 controlCode, UInt32 insertPos);
	virtual bool IsOpen() const;
};

// A book or scroll
// Pseudo-html tags complicate things here. TextInputJournal only supports limited html.
// One non-erasable <FONT> tag at start of input string defines font for entire string
// <DIV> tags appear only at line breaks, max one per line (alignment affects entire line and subsequent lines)
// <IMG> tags are not supported directly but can be added by scripters via InsertInInputText command
class TextInputJournal : public TextInputMenu
{
	UInt32 SetLineStartingTag(UInt32 fromPos, UInt32 tagType);		//change alignment for current line

	TESObjectBOOK	* m_dummyBook;

public:
	TextInputJournal(char* fmtString, UInt32 maxLen, bool bAsScroll);
	virtual ~TextInputJournal();
	virtual void Update();
	virtual void Init();
	virtual UInt32 ResolveControlCode(UInt8 controlCode, UInt32 insertPos);
	virtual bool IsOpen() const;
};

//Contains and controls TextInputMenu object. Provides interface for script commands
class TextInputHandler
{
	TextInputHandler(TextInputMenu* menu);

	static TextInputHandler	* s_singleton;						//not technically a singleton but close enough I suppose
	static const char		* m_tabChars;
	static const DWORD		m_cursorDelay;

	TextInputMenu			* m_menu;
	UInt16					m_cursorPos;
	DWORD					m_cursorTimer;
	BYTE					m_prevKeyboardState[256];
	char					m_cursorChar;
	UInt8					m_lastControlCharPressed;			//scripts can check this for user-defined controls
	bool					m_bControlKeysDisabled;
	Script					* m_controlHandler;					// a function script invoked when an unhandled control key is pressed

	bool	BlinkCursor();		//returns true if cursor char changed
	void	UpdateDisplay();	// redisplay contents
public:
	~TextInputHandler()
		{	delete m_menu;	}
	static TextInputHandler * GetTextBox()
		{	return s_singleton;	}
	static bool Create(TextInputMenu* menu);
	void GetInputString(char* outString) const;
	void Update();										// check for input, update cursor, redisplay if necessary
	void Close();										// release the text input handler for use by another script
	UInt8 GetControlKeyPressed();
	void InsertInText(const char * toInsert);
	void DeleteInText(UInt32 numToDelete, bool bBackwards, bool bWholeWords);
	void MoveCursor(UInt32 numToMove, bool bBackwards);
	UInt32 GetCursorPos();
	bool IsHTML() const { return m_menu ? m_menu->IsHTML() : false; }
	void SetControlKeysDisabled(bool bDisable);			// toggle default controls like those for changing font
	void SetControlHandler(Script* handler) { m_controlHandler = handler; }
	bool SetInputText(const std::string& text, UInt32 newCursorPos);	// returns false and does not modify if newCursorPos invalid.
};

#endif
