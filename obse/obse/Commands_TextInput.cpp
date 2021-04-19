#include "Commands_TextInput.h"
#include "ParamInfos.h"
#include "Script.h"
#include "ScriptUtils.h"

#if OBLIVION
#include "StringVar.h"
#include "FunctionScripts.h"
#include <obse/Hooks_Input.h>
#include <obse/Hooks_Input.h>

enum {
	kInputMenuType_Message,
	kInputMenuType_Book,
	kInputMenuType_Scroll,
	kInputMenuType_Line,			//not yet supported
};

//////////////////////////////////////
// TextInputMenu (base)
/////////////////////////////////////

TextInputMenu::TextInputMenu(UInt32 maxLen, bool bIsHTML)
	: m_maxLen(maxLen), m_bIsHTML(bIsHTML)
{
	//
}

TextInputMenu::~TextInputMenu()
{
	//
}

UInt32 TextInputMenu::SeekPosition(UInt32 fromPos, bool bBackwards, bool bSeekWordStart) const
{
	if ((fromPos <= GetMinPos() && bBackwards) || (fromPos > GetMaxPos() && !bBackwards))
		return fromPos;						//bounds check

	char curChar = bBackwards ? m_inputText[fromPos - 1] : m_inputText[fromPos];
	if (bSeekWordStart || curChar == '<' || curChar == '>')
		return FindWordBoundary(fromPos, bBackwards);

	return fromPos + (bBackwards ? -1 : 1);
}

UInt32 TextInputMenu::FindWordBoundary(UInt32 startPos, bool bBackwards) const
{
	UInt32 curPos = bBackwards ? startPos - 1 : startPos;
	SInt32 stepVal = bBackwards ? -1 : 1;
	UInt32 terminalPos = bBackwards ? m_minPos : GetInputLength();

	char curChar = m_inputText[curPos];
	if (curChar == '>')
		return FindHTMLMatchedBracket(curPos - 1, true);
	else if (curChar == '<')
		return FindHTMLMatchedBracket(curPos + 1, false) + 1;

	UInt32 nextPos = curPos + stepVal;
	bool bPatternStart = bBackwards ? true : false;		//seek isgraph() first or !isgraph() first?
	bool bPrevIsGraph = isgraph(curChar) ? true : false;

	while (nextPos != terminalPos)
	{
		char nextChar = m_inputText[nextPos];

		if (nextChar == '<' || nextChar == '>')
			return bBackwards ? curPos : nextPos;

		bool bNextIsGraph = isgraph(nextChar) ? true : false;
		if (bPrevIsGraph == bPatternStart && bNextIsGraph != bPatternStart)
			break;

		curPos = nextPos;
		nextPos += stepVal;
		bPrevIsGraph = bNextIsGraph;
	}

	if (nextPos == terminalPos)
		return terminalPos;

	return bBackwards ? nextPos + 1 : nextPos;
}

//Assumes scripters don't go inserting erroneous tags (why would they? OB doesn't support anything else...)
UInt32 TextInputMenu::GetHTMLTagType(UInt32 tagStartPos) const
{
	switch (m_inputText[tagStartPos + 1])
	{
	case 'D':
	case 'd':
		{
			switch (m_inputText[tagStartPos + 12])		//offset of left, center, or right string
			{
			case 'l':
			case 'L':
				return kHTMLTag_DIV_Left;
			case 'r':
			case 'R':
				return kHTMLTag_DIV_Right;
			case 'c':
			case 'C':
				return kHTMLTag_DIV_Center;
			default:
				return kHTMLTag_Other;
			}
		}
	case 'B':
	case 'b':
		return kHTMLTag_BR;
	case 'F':
	case 'f':
		return kHTMLTag_FONT;
	default:
		return kHTMLTag_Other;
	}
}

UInt32 TextInputMenu::FindLineStart(UInt32 fromPos) const
{
	if (fromPos <= GetMinPos())
		return GetMinPos();

	UInt32 lBracPos = fromPos;
	while ( (lBracPos = m_inputText.rfind('<', lBracPos)) != std::string::npos ) 
	{
		if (GetHTMLTagType(lBracPos) != kHTMLTag_Other)
			return FindHTMLMatchedBracket(lBracPos + 1, false) + 1;

		lBracPos--;
	}

	return GetMinPos();
}

UInt32 TextInputMenu::FindLineEnd(UInt32 fromPos) const
{
	if (fromPos >= GetMaxPos())
		return GetInputLength();

	UInt32 lBracPos = fromPos;
	while ( (lBracPos = m_inputText.find('<', lBracPos)) != std::string::npos )
	{
		if (GetHTMLTagType(lBracPos) != kHTMLTag_Other)
			return lBracPos;

		lBracPos++;
	}

	return GetInputLength();
}

bool TextInputMenu::IsCurrentLineEmpty(UInt32 fromPos) const
{
	if (FindLineEnd(fromPos) - FindLineStart(fromPos) == 1)
		return true;
	else
		return false;
}

UInt32 TextInputMenu::InsertChar(UInt32 insertPos, char toInsert)
{
	if (IsFull())
		return insertPos;

	if (m_bIsHTML)		// check for < > chars and disallow in book input
	{
		if (toInsert == '<' || toInsert == '>')
			return insertPos;
	}

	if (insertPos == GetInputLength())
		m_inputText.append(1, toInsert);
	else
		m_inputText.insert(insertPos, 1, toInsert);

	return insertPos + 1;
}

void TextInputMenu::Erase(UInt32 pos)
{
	if (pos < GetInputLength())
		m_inputText.erase(pos, 1);
}

UInt32 TextInputMenu::InsertText(UInt32 insertPos, const char* toInsert)
{
	UInt32 insertLen = strlen(toInsert);

	if (GetInputLength() + insertLen >= m_maxLen)		//not enough room for the text
		return insertPos;

	if (insertPos == m_inputText.length())
		m_inputText.append(toInsert);
	else
		m_inputText.insert(insertPos, toInsert);

	return insertPos + insertLen;
}

UInt32 TextInputMenu::FindHTMLMatchedBracket(UInt32 startBracketPos, bool bBackwards) const
{
	UInt32 endBracketPos;

	if (bBackwards)
	{
		endBracketPos = m_inputText.rfind('<', startBracketPos);
		if (endBracketPos == std::string::npos)
			endBracketPos = m_minPos;
	}
	else
	{
		endBracketPos = m_inputText.find('>', startBracketPos);
		if (endBracketPos == std::string::npos)
			endBracketPos = GetMaxPos();
	}

	return endBracketPos;
}

UInt32 TextInputMenu::DeleteText(UInt32 startPos, bool bBackwards, bool bDeleteWord)
{
	UInt32 delPos = SeekPosition(startPos, bBackwards, bDeleteWord);
	if (bBackwards)
	{
		m_inputText.erase(delPos, startPos - delPos);
		return delPos;
	}
	else
	{
		m_inputText.erase(startPos, delPos - startPos);
		return startPos;
	}
}

/////////////////////////////////
//  TextInputMessageBox
////////////////////////////////

TextInputMessageBox::TextInputMessageBox(char* fmtString, UInt32 maxLen, Script* callingScript, TESObjectREFR* callingObj)
	: TextInputMenu(maxLen, false), m_bShortcutsDisabled(false), m_script(callingScript)
{
	m_minPos = 0;
	UInt32 fmtStringLen = strlen(fmtString);
	m_fmtString = new char[fmtStringLen + 1];
	strcpy_s(m_fmtString, fmtStringLen + 1, fmtString);		//don't parse format string until Init()

	m_scriptRefID = callingScript->refID;
	if (callingObj && !callingObj->IsTemporary())		//not a temporary script
		m_scriptRefID = callingObj->refID;
}

TextInputMessageBox::~TextInputMessageBox()
{
	ToggleMenuShortcutKeys(true);

	MessageMenu* menu = (MessageMenu*)GetMenuByType(kMenuType_Message);
	if (menu)
		menu->HandleClick(menu->kButtonID_Close, 0);		//closes the menu

	delete[] m_fmtString;
}

void TextInputMessageBox::Init()
{
	char* buttons[10] = { NULL };
	UInt32 numButtons = 0;
	UInt32 fmtStringLen = strlen(m_fmtString);
	char* fmtString = new char[fmtStringLen + 1];
	strcpy_s(fmtString, fmtStringLen + 1, m_fmtString);

	//separate prompt text and button text
	for (UInt32 strPos = 0; strPos < fmtStringLen && numButtons < 10; strPos++)
	{
		if (fmtString[strPos] == GetSeparatorChar(m_script) && (strPos + 1 < fmtStringLen))
		{
			fmtString[strPos] = '\0';
			buttons[numButtons++] = fmtString + strPos + 1;
		}
	}

	m_promptText = fmtString;
	if (!buttons[0])				//supply default button if none specified
		buttons[0] = "Finished";

	//now display the messagebox
	ShowMessageBox(fmtString, ShowMessageBox_Callback, 0, buttons[0], buttons[1], buttons[2], buttons[3], buttons[4],
		buttons[5], buttons[6], buttons[7], buttons[8], buttons[9], 0);

	//Register it with the game so scripts can pick up button pressed
	*ShowMessageBox_pScriptRefID = m_scriptRefID;
	*ShowMessageBox_button = -1;

	//Disable menu shortcut keys so they don't interfere with typing
	if (!m_bShortcutsDisabled)
	{
		m_bShortcutsDisabled = true;
		ToggleMenuShortcutKeys(false, GetMenuByType(kMenuType_Message));
	}
}

bool TextInputMessageBox::IsOpen() const
{
	return true;		//scripts use GetButtonPressed to determine when messagebox is closed.
						//we can always create a new messagebox if needed, so this always returns true
}

//returns -1 if not handled, otherwise returns position one-past-the-end of inserted text
UInt32 TextInputMessageBox::ResolveControlCode(UInt8 controlCode, UInt32 insertPos)
{
	if (controlCode == DIK_RETURN)
		return InsertChar(insertPos, '\n');
	else
		return -1;
}

void TextInputMessageBox::Update()
{
	std::string text = m_promptText + m_inputText;

	MessageMenu* msgBox = (MessageMenu*)GetMenuByType(kMenuType_Message);
	if (!msgBox)	//original msgBox was closed/overwritten
	{
		Init();														//so re-display it
		msgBox = (MessageMenu*)GetMenuByType(kMenuType_Message);
	}
	else if (*ShowMessageBox_pScriptRefID != m_scriptRefID || !msgBox->IsScriptMessageBox())
		return;	// someone else is displaying a messagebox, let's not interfere

	msgBox->messageText->SetStringValue(kTileValue_string, text.c_str());
	msgBox->messageText->UpdateField(kTileValue_string, 0.0, text.c_str());
}

/////////////////////////////////////
//  TextInputJournal
/////////////////////////////////////

// use kHTMLTag_XXX enum as index
static const std::string kTagStrings[6] =
{
	"<DIV align=\"left\">",	
	"<DIV align=\"right\">",
	"<DIV align=\"center\">",
	"<BR>",
	"<FONT face=\"1\">",
	""
};
TextInputJournal::TextInputJournal(char* fmtString, UInt32 maxLen, bool bAsScroll)
: TextInputMenu(maxLen, true)
{
	m_inputText = kTagStrings[kHTMLTag_FONT] + kTagStrings[kHTMLTag_DIV_Left];
	m_minPos = m_inputText.length();		// don't allow starting tags to be edited by user directly
	m_inputText += fmtString;

	// If fmtString already contains starting tags, use them instead of defaults
	std::string defaultText(fmtString);
	if (defaultText.length() >= m_minPos)
		if (!_stricmp("<FONT face=\"", defaultText.substr(0, 12).c_str()))
			if (!_stricmp("<DIV align=\"", defaultText.substr(15, 12).c_str()))
				m_inputText = defaultText;

	DEBUG_PRINT("Text = %s, minPos = %d", m_inputText.c_str(), m_minPos);

	//Create a proxy book object (required by some of the BookMenu code)
	m_dummyBook = (TESObjectBOOK*)FormHeap_Allocate(sizeof(TESObjectBOOK));
	m_dummyBook->Constructor();
	m_dummyBook->SetCantBeTaken(true);
	m_dummyBook->SetIsScroll(bAsScroll);
}

TextInputJournal::~TextInputJournal()
{
	ToggleMenuShortcutKeys(true);

	BookMenu* menu = (BookMenu*)GetMenuByType(kMenuType_Book);
	if (menu)
		menu->HandleClick(menu->kButtonID_Exit, 0);		//closes the menu

	m_dummyBook->Destroy(false);
}

void TextInputJournal::Init()
{
	//TODO: Turn creation of menus into a template function?

	InterfaceManager* intfc = InterfaceManager::GetSingleton();

	// Is BookMenu already open?
	BookMenu* bookMenu = (BookMenu*)GetMenuByType(kMenuType_Book);
	if (bookMenu)	// open, so get rid of it first
		bookMenu->Destructor(1);

	// create new book menu
	Tile* tile = intfc->menuRoot->ReadXML("data\\menus\\book_menu.xml");
	if (tile)
	{
		Tile* bookRoot = tile->GetRoot();
		if (bookRoot)
		{
			TileMenu* bookMenuTile = tile_cast <TileMenu>(bookRoot);
			if (bookMenuTile)
			{
				BookMenu* bookMenu = (BookMenu*)(bookMenuTile->menu);
				if (bookMenu)
				{
					bookMenu->book = m_dummyBook;
					bookMenu->bookRef = NULL;

					ToggleMenuShortcutKeys(false, bookMenu);

					bookMenu->RegisterTile(bookMenuTile);
					
					// update depth
					Tile* backgroundTile = NULL;
					if (m_dummyBook->IsScroll())
						backgroundTile = bookMenu->tile->GetChildByName("book_background_scroll");
					else
						backgroundTile = bookMenu->tile->GetChildByName("book_background");

					Tile::Value* depthVal = backgroundTile->GetValueByType(kTileValue_depth);
					if (depthVal)
					{
						backgroundTile->UpdateFloat(kTileValue_depth, depthVal->num + intfc->GetDepth());
						depthVal->num += intfc->GetDepth();
					}

					if (!m_dummyBook->IsScroll())
						bookMenu->tile->UpdateFloat(BookMenu::kBookValue_IsBook, 2.0);	
					
					bookMenu->EnableMenu(false);
					bookMenu->UpdateText(m_inputText.c_str());
				}
			}
		}
	}
}

//returns new insertion point on current line
UInt32 TextInputJournal::SetLineStartingTag(UInt32 fromPos, UInt32 tagType)
{
	UInt32 newPos = fromPos;

	UInt32 startPos = FindLineStart(fromPos);
	bool bUpdateMinPos = (startPos == m_minPos);		//changing first tag in text requires updating min pos too

	UInt32 lBracPos = FindHTMLMatchedBracket(startPos - 1, true);

	//replace previous tag with new one
	m_inputText.erase(lBracPos, startPos - lBracPos);
	newPos -= startPos - lBracPos;
	if (bUpdateMinPos)
		m_minPos -= startPos - lBracPos;

	m_inputText.insert(lBracPos, kTagStrings[tagType]);
	newPos += kTagStrings[tagType].length();
	if (bUpdateMinPos)
		m_minPos += kTagStrings[tagType].length();

	return newPos;
}

//returns -1 if code not handled, otherwise returns new insertion pos
UInt32 TextInputJournal::ResolveControlCode(UInt8 controlCode, UInt32 insertPos)
{
	if (IsFull())
		return -1;

	UInt32 newPos = insertPos;

	switch (controlCode)
	{
	case DIK_RETURN: //line break
		newPos = InsertText(insertPos, kTagStrings[kHTMLTag_BR].c_str());
		break;
	case DIK_UP:	//move to end of previous line
		newPos = FindLineStart(insertPos);
		if (newPos > GetMinPos())
			newPos = SeekPosition(newPos, true, true);

		break;
	case DIK_DOWN:	//move to start of next line
		newPos = FindLineEnd(insertPos);
		if (newPos <= GetMaxPos())
			newPos = SeekPosition(newPos, false, true);

		break;
	case DIK_C:		//align center
		newPos = SetLineStartingTag(insertPos, kHTMLTag_DIV_Center);
		break;
	case DIK_L:		//align left
		newPos = SetLineStartingTag(insertPos, kHTMLTag_DIV_Left);
		break;
	case DIK_R:		//align right
		newPos = SetLineStartingTag(insertPos, kHTMLTag_DIV_Right);
		break;
	case DIK_1:
	case DIK_2:
	case DIK_3:
	case DIK_4:
	case DIK_5:		//Change font
		{
			char fontNum = controlCode - DIK_1 + '1';
			m_inputText[12] = fontNum;
			break;
		}
	default:
		newPos = -1;
	}

	return newPos;
}

bool TextInputJournal::IsOpen() const
{
	BookMenu* bookMenu = (BookMenu*)GetMenuByType(kMenuType_Book);
	if (bookMenu && bookMenu->book == m_dummyBook)
		return true;
	else
		return false;
}

void TextInputJournal::Update()
{
	BookMenu* bookMenu = (BookMenu*)GetMenuByType(kMenuType_Book);
	if (bookMenu && bookMenu->book == m_dummyBook)
		bookMenu->UpdateText((m_promptText + m_inputText).c_str());
}

//////////////////////////////////
//  TextInputHandler
/////////////////////////////////

// init static members
TextInputHandler	* TextInputHandler::s_singleton = NULL;
const char  * TextInputHandler::m_tabChars = "    ";
const DWORD	TextInputHandler::m_cursorDelay = 500;

TextInputHandler::TextInputHandler(TextInputMenu* menu)
	: m_menu(menu), m_cursorTimer(GetTickCount()), m_lastControlCharPressed(-1), m_cursorChar('|'), m_bControlKeysDisabled(false),
	m_controlHandler(NULL)
{
	memcpy(m_prevKeyboardState, g_inputGlobal->CurrentKeyState, 256);  //TODO or Previous state? Why not using directly the previous state?
	m_menu->Init();
	m_cursorPos = m_menu->GetInputLength();		// we want cursor at end of starting text
}

bool TextInputHandler::BlinkCursor()
{
	DWORD elapsedTime = GetTickCount();
	if (elapsedTime - m_cursorTimer > m_cursorDelay)
	{
		m_cursorTimer = elapsedTime;
		m_cursorChar = (m_cursorChar == '|') ? 127 : '|';
		return true;
	}
	else
		return false;
}

bool TextInputHandler::Create(TextInputMenu* menu)
{
	if (s_singleton)		//already initialized
		return false;
	else
	{
		s_singleton = new TextInputHandler(menu);
		return true;
	}
}

void TextInputHandler::Close()
{
	delete s_singleton;
	s_singleton = NULL;
}

void TextInputHandler::Update()
{
	if (!m_menu->IsOpen() || IsConsoleOpen())
		return;
	//find first key pressed since last time we checked
	UInt8 keyCode = 0;
	for (UInt32 idx = 0; idx <= 0x100; idx++)
	{
		if (g_inputGlobal->CurrentKeyState[idx] && !m_prevKeyboardState[idx])
		{
			keyCode = idx; 
			break;
		}
	}

	bool bControlPressed = g_inputGlobal->CurrentKeyState[DIK_LCONTROL] || g_inputGlobal->CurrentKeyState[DIK_RCONTROL];
	bool bShifted = g_inputGlobal->CurrentKeyState[DIK_LSHIFT] || g_inputGlobal->CurrentKeyState[DIK_RSHIFT];
	bool bUpdateText = false;

	if (keyCode)
	{
		switch (keyCode)
		{
		case DIK_UP:
		case DIK_DOWN:
		case DIK_RETURN:
			{
				UInt32 cursorAdjust = m_menu->ResolveControlCode(keyCode, m_cursorPos);
				if (cursorAdjust != -1)
				{
					m_cursorPos = cursorAdjust;
					bUpdateText = true;
				}

				break;
			}
		case DIK_TAB:
			m_cursorPos = m_menu->InsertText(m_cursorPos, m_tabChars);
			bUpdateText = true;
			break;
		case DIK_BACK:
			m_cursorPos = m_menu->DeleteText(m_cursorPos, true, bControlPressed);
			bUpdateText = true;
			break;
		case DIK_DELETE:
			m_cursorPos = m_menu->DeleteText(m_cursorPos, false, bControlPressed);
			bUpdateText = true;
			break;
		case DIK_LEFT:
			m_cursorPos = m_menu->SeekPosition(m_cursorPos, true, bControlPressed);
			bUpdateText = true;
			break;
		case DIK_RIGHT:
			m_cursorPos = m_menu->SeekPosition(m_cursorPos, false, bControlPressed);
			bUpdateText = true;
			break;
		case DIK_HOME:
			m_cursorPos = m_menu->GetMinPos();
			bUpdateText = true;
			break;
		case DIK_END:
			if (m_menu->GetInputLength() > 0)
			{
				m_cursorPos = m_menu->GetInputLength();
				bUpdateText = true;
			}
			break;
		case DIK_GRAVE:		//Console key - don't include in input text
		case DIK_ESCAPE:
			break;
		default:
			if (bControlPressed)
			{
				UInt32 cursorAdjust = -1;
				if (!m_bControlKeysDisabled) {
					// give menu a chance to handle the control key
					cursorAdjust = m_menu->ResolveControlCode(keyCode, m_cursorPos);
				}

				if (cursorAdjust == -1) {
					// menu didn't handle it. Do we have a function script to try instead?
					if (m_controlHandler) {
						// invoke the function script, passing in the control key code
						InternalFunctionCaller caller(m_controlHandler);
						caller.SetArgs(1, (void*)((UInt32)keyCode));
						ScriptToken* result = UserFunctionManager::Call(caller);
						delete result;		// result unused
					}
					else {
						// store it for later retrieval via GetTextInputControlKeyPressed
						m_lastControlCharPressed = keyCode;
					}
				}
				else
				{
					// menu handled, update cursor position accordingly
					m_cursorPos = cursorAdjust;
					bUpdateText = true;
				}
			}
			else
			{
				char charToInsert = ScancodeToChar(keyCode, bShifted);
				if (charToInsert)
				{
					m_cursorPos = m_menu->InsertChar(m_cursorPos, charToInsert);
					bUpdateText = true;
				}
			}

			break;
		}
	}

	//store current state of keyboard for next update
	memcpy(m_prevKeyboardState, g_inputGlobal->CurrentKeyState, 256); //Againg use input previous state directly?

	if (BlinkCursor())
		bUpdateText = true;

	if (bUpdateText)
		UpdateDisplay();
}

void TextInputHandler::UpdateDisplay()
{
	if (m_menu->IsOpen())
	{
		m_menu->InsertChar(m_cursorPos, m_cursorChar);
		m_menu->Update();
		m_menu->Erase(m_cursorPos);				//cursor should never be left in input string
	}
}

UInt8 TextInputHandler::GetControlKeyPressed()
{
	UInt8 ctrlChar = m_lastControlCharPressed;
	m_lastControlCharPressed = -1;
	return ctrlChar;
}

void TextInputHandler::GetInputString(char* outString) const
{
	std::string inputString = m_menu->GetInputText();
	strcpy_s(outString, inputString.length() + 1, inputString.c_str());
}

void TextInputHandler::InsertInText(const char* toInsert)
{
	//if text contains brackets, verify they are correctly matched
	if (m_menu->IsHTML())
	{
		bool bFoundOpenBracket = false;
		UInt32 insertLen = strlen(toInsert);
		for (UInt32 i = 0; i < insertLen; i++)
		{
			char curChar = toInsert[i];
			if (curChar == '<')
			{
				if (!bFoundOpenBracket)
					bFoundOpenBracket = true;
				else
					return;				//two consecutive opening brackets = bad
			}
			else if (curChar == '>')
			{
				if (bFoundOpenBracket)
					bFoundOpenBracket = false;		//closing bracket matches opening bracket
				else
					return;							//closing bracket without opening bracket = bad
			}
		}
	}

	//brackets all match up, so go ahead and insert the text
	m_cursorPos = m_menu->InsertText(m_cursorPos, toInsert);

	//don't update display until next Update() call
	// actually, do update now.
	UpdateDisplay();
}

void TextInputHandler::DeleteInText(UInt32 numToDelete, bool bBackwards, bool bWholeWords)
{
	while (numToDelete--)
		m_cursorPos = m_menu->DeleteText(m_cursorPos, bBackwards, bWholeWords);
}

void TextInputHandler::MoveCursor(UInt32 numToMove, bool bBackwards)
{

	while (numToMove--)
	{
		UInt32 cursorStartPos = m_cursorPos;
		m_cursorPos = m_menu->SeekPosition(m_cursorPos, bBackwards, false);
		if (m_cursorPos == cursorStartPos)		// can't move any further
			break;
	}
}

UInt32 TextInputHandler::GetCursorPos()
{
	return m_cursorPos;
}

void TextInputHandler::SetControlKeysDisabled(bool bDisable)
{
	m_bControlKeysDisabled = bDisable;
}

bool TextInputHandler::SetInputText(const std::string& text, UInt32 newCursorPos)
{
	// script is responsible for making sure text is in an appropriate format
	// we make sure cursor doesn't end up outside text or inside an html tag
	if (newCursorPos >= text.length()) {
		return false;
	}
	else if (m_menu->IsHTML()) {
		size_t bracPos = text.find_first_of("<>", newCursorPos);
		if (bracPos != std::string::npos && text[bracPos] == '>') {
			return false;
		}
		else {
			bracPos = text.find_last_of("<>", newCursorPos);
			if (bracPos != std::string::npos && text[bracPos] == '<') {
				return false;
			}
		}
	}

	m_menu->SetText(text);
	m_cursorPos = newCursorPos;
	UpdateDisplay();
	return true;
}

/////////////////////////////////////////
//
//  Commands
//
////////////////////////////////////////

static bool Cmd_OpenTextInput_Execute(COMMAND_ARGS)
{
	*result = 0;
	char buffer[kMaxMessageLength];
	UInt32 maxLength = kMaxMessageLength;
	UInt32 menuType = kInputMenuType_Message;

	if (TextInputHandler::GetTextBox())
		return true;					//already in use

	if (!ExtractFormatStringArgs(0, buffer, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, 
								 kCommandInfo_OpenTextInput.numParams, &menuType, &maxLength))
		return true;

	if (maxLength > kMaxMessageLength)
		maxLength = kMaxMessageLength;

	TextInputMenu * menu = NULL;
	switch (menuType)
	{
	case kInputMenuType_Message:
		menu = new TextInputMessageBox(buffer, maxLength, scriptObj, thisObj);
		break;
	case kInputMenuType_Book:
		menu = new TextInputJournal(buffer, maxLength, false);
		break;
	case kInputMenuType_Scroll:
		menu = new TextInputJournal(buffer, maxLength, true);
		break;
	default:			//invalid type
		return true;
	}

	if (TextInputHandler::Create(menu))
		*result = 1;
	else				// wtf?
		delete menu;

	return true;
}

static bool Cmd_GetInputText_Execute(COMMAND_ARGS)
{
	UInt32 bRemoveHTML = 0;
	char inputText[kMaxMessageLength] = { 0 };
	std::string inputStr;
	const char* toAssign = "";

	if (ExtractArgs(PASS_EXTRACT_ARGS, &bRemoveHTML)) {
		TextInputHandler* textBox = TextInputHandler::GetTextBox();

		if (textBox) {
			textBox->GetInputString(inputText);
			if (bRemoveHTML && textBox->IsHTML()) {
				// strip all HTML tags
				inputStr = inputText;
				UInt32 pos = 0;
				while ((pos = inputStr.find('<'), pos) != -1) {
					UInt32 endPos = inputStr.find('>');
					if (endPos == -1) {
						// wtf? oh well, just erase the single '<' character if this ever happens.
						endPos = pos;
					}
					else {
						// check for <br>, replace with new line
						if (endPos - pos == 3 && inputStr[pos+1] == 'B' && inputStr[pos+2] == 'R') {
							inputStr.erase(pos, 4);
							inputStr.insert(pos, 1, '\n');
						}
						else {
							inputStr.erase(pos, endPos-pos+1);
						}
					}
				}
				toAssign = inputStr.c_str();
			}
			else {
				toAssign = inputText;
			}
		}
	}

	AssignToStringVar(PASS_COMMAND_ARGS, toAssign);
	return true;
}

static bool Cmd_CloseTextInput_Execute(COMMAND_ARGS)
{
	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox)
		textBox->Close();

	return true;
}

static bool Cmd_UpdateTextInput_Execute(COMMAND_ARGS)
{
	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox)
		textBox->Update();

	return true;
}

static bool Cmd_IsTextInputInUse_Execute(COMMAND_ARGS)
{
	*result = TextInputHandler::GetTextBox() ? 1 : 0;
	return true;
}

static bool Cmd_GetTextInputControlPressed_Execute(COMMAND_ARGS)
{
	*result = -1;

	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox)
	{
		UInt8 ctrl = textBox->GetControlKeyPressed();
		if (ctrl != 0xFF)
			*result = ctrl;
	}

	return true;
}

static bool Cmd_InsertInInputText_Execute(COMMAND_ARGS)
{
	*result = 0;

	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox)
	{
		char buffer[kMaxMessageLength];
		if (!ExtractFormatStringArgs(0, buffer, paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, 21))
			return true;

		textBox->InsertInText(buffer);
	}

	return true;
}

static bool Cmd_DeleteFromInputText_Execute(COMMAND_ARGS)
{
	// syntax: DeleteFromInputText numToDelete backwards? wholeWords?
	UInt32 numToDelete = 0;
	UInt32 bBackwards = 0;
	UInt32 bWholeWords = 0;
	*result = 0;

	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox)
	{
		if (ExtractArgs(PASS_EXTRACT_ARGS, &numToDelete, &bBackwards, &bWholeWords))
			textBox->DeleteInText(numToDelete, bBackwards ? true : false, bWholeWords ? true : false);
	}

	return true;
}

static bool Cmd_MoveTextInputCursor_Execute(COMMAND_ARGS)
{
	UInt32 numToMove = 0;
	UInt32 bBackwards = 0;
	*result = 0;

	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox)
	{
		if (ExtractArgs(PASS_EXTRACT_ARGS, &numToMove, &bBackwards))
			textBox->MoveCursor(numToMove, bBackwards ? true : false);
	}

	return true;
}

static bool Cmd_GetTextInputCursorPos_Execute(COMMAND_ARGS)
{
	*result = -1;

	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox)
		*result = textBox->GetCursorPos();

	return true;
}

static bool Cmd_SetTextInputDefaultControlsDisabled_Execute(COMMAND_ARGS)
{
	UInt32 bDisable = 0;
	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox && ExtractArgs(PASS_EXTRACT_ARGS, &bDisable)) {
		textBox->SetControlKeysDisabled(bDisable ? true : false);
	}

	return true;
}

static bool Cmd_SetTextInputControlHandler_Execute(COMMAND_ARGS)
{
	TESForm* scriptForm = NULL;
	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox && ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &scriptForm)) {
		Script* script = OBLIVION_CAST(scriptForm, TESForm, Script);
		if (script) {
			textBox->SetControlHandler(script);
		}
	}

	return true;
}

static bool Cmd_SetInputText_Execute(COMMAND_ARGS)
{
	*result = 0.0;

	TextInputHandler* textBox = TextInputHandler::GetTextBox();
	if (textBox) {
		ExpressionEvaluator eval(PASS_COMMAND_ARGS);
		if (eval.ExtractArgs() && eval.NumArgs() == 2) {
			const char* newText = eval.Arg(0)->GetString();
			if (newText) {
				UInt32 newPos = eval.Arg(1)->GetNumber();
				if (textBox->SetInputText(newText, newPos)) {
					*result = 1.0;
				}
			}
		}
	}

	return true;
}

#endif

static ParamInfo kParams_OpenTextInput[23] =
{
	FORMAT_STRING_PARAMS,
	{	"type",		kParamType_Integer,		1	},
	{	"maxlength",kParamType_Integer,		1	},
};

DEFINE_COMMAND(OpenTextInput,
			   opens an interface for user text input,
			   0,
			   23,
			   kParams_OpenTextInput);

DEFINE_COMMAND(GetInputText,
			   returns user text input,
			   0,
			   1,
			   kParams_OneOptionalInt);

DEFINE_COMMAND(CloseTextInput,
			   closes a text input interface,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(UpdateTextInput,
			   updates the text input interface,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(IsTextInputInUse,
			   returns 1 if a script is currently using the text input interface,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(GetTextInputControlPressed,
			   returns the ASCII code of the last control character pressed,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(InsertInInputText,
			   inserts a string in text input at the current cursor position,
			   0,
			   21,
			   kParams_FormatString);

static ParamInfo kParams_DeleteFromInputText[3] = 
{
	{	"numToDelete",	kParamType_Integer,	0	},
	{	"bBackwards",	kParamType_Integer,	1	},
	{	"bWholeWords",	kParamType_Integer,	1	},
};

DEFINE_COMMAND(DeleteFromInputText,
			   deletes input text starting from the current cursor position,
			   0,
			   3,
			   kParams_DeleteFromInputText);

static ParamInfo kParams_MoveTextInputCursor[2] =
{
	{	"numToMove",	kParamType_Integer,	0	},
	{	"bBackwards",	kParamType_Integer,	1	},
};

DEFINE_COMMAND(MoveTextInputCursor,
			   moves the cursor forward or backward by the specified number of characters,
			   0,
			   2,
			   kParams_MoveTextInputCursor);

DEFINE_COMMAND(GetTextInputCursorPos,
			   returns the current cursor position,
			   0,
			   0,
			   NULL);

DEFINE_COMMAND(SetTextInputDefaultControlsDisabled, disables or enables handling of default controls, 0, 1, kParams_OneInt);
DEFINE_COMMAND(SetTextInputControlHandler, sets a function script to handle text box controls, 0, 1, kParams_OneInventoryObject);

static ParamInfo kOBSEParams_SetInputText[2] =
{
	{ "text", kOBSEParamType_String, 0 },
	{ "pos", kOBSEParamType_Number, 0 },
};

CommandInfo kCommandInfo_SetInputText =
{
	"SetInputText", "", 0, "sets the text and cursor position of text input menu",
	0, 2, kOBSEParams_SetInputText,
	HANDLER(Cmd_SetInputText_Execute),
	Cmd_Expression_Parse, NULL, 0
};
