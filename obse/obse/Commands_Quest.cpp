#include "Commands_Quest.h"
#include "ParamInfos.h"
#include "Script.h"

#if OBLIVION

#include "ScriptUtils.h"
#include "GameAPI.h"
#include "ArrayVar.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameData.h"
#include "Hooks_Gameplay.h"

static bool Cmd_QuestExists_Execute(COMMAND_ARGS)
{
	char questName[0x200] = { 0 };
	*result = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &questName) && questName)
		*result = (*g_dataHandler)->GetQuestByEditorName(questName, strlen(questName)) ? 1 : 0;

	return true;
}

static bool Cmd_GetActiveQuest_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (*g_thePlayer && (*g_thePlayer)->activeQuest)
		*refResult = (*g_thePlayer)->activeQuest->refID;

	return true;
}

static bool Cmd_SetActiveQuest_Execute(COMMAND_ARGS)
{
	TESQuest* quest = NULL;
	if (*g_thePlayer && ExtractArgs(PASS_EXTRACT_ARGS, &quest) && quest)
		(*g_thePlayer)->activeQuest = quest;

	return true;
}

static bool Cmd_ClearActiveQuest_Execute(COMMAND_ARGS)
{
	if (*g_thePlayer) {
		(*g_thePlayer)->activeQuest = NULL;
	}
	return true;
}

class QuestPredicate
{
public:
	virtual bool Accept(TESQuest* quest) const = 0;
};

class QuestStatePredicate : public QuestPredicate
{
public:
	QuestStatePredicate(UInt32 state) : m_state (state) { }
	virtual bool Accept(TESQuest* quest) const { return ((quest->questFlags & m_state) == m_state); }
private:
	UInt32	m_state;
};

static ArrayID GetQuestList_Execute(const QuestPredicate& pred, Script* scriptObj)
{
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());

	double idx = 0.0;
	for (tList<TESQuest>::Iterator Itr = (*g_dataHandler)->quests.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		TESQuest* quest = Itr.Get();
		if (pred.Accept(quest)) {
			g_ArrayMap.SetElementFormID(arr, idx, quest->refID);
			idx += 1.0;
		}
	}

	return arr;
}

static bool Cmd_GetCurrentQuests_Execute(COMMAND_ARGS)
{
	*result = GetQuestList_Execute(QuestStatePredicate(TESQuest::kQuestFlag_Active), scriptObj);
	return true;
}

static bool Cmd_GetCompletedQuests_Execute(COMMAND_ARGS)
{
	*result = GetQuestList_Execute(QuestStatePredicate(TESQuest::kQuestFlag_Completed), scriptObj);
	return true;
}

static bool Cmd_UncompleteQuest_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESQuest* quest = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &quest) && quest) {
		quest->SetCompleted(false);
		*result = 1;
	}

	return true;
}

static bool Cmd_IsQuestCompleted_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESQuest* quest = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &quest) && quest) {
		*result = quest->IsCompleted() ? 1.0 : 0.0;
	}

	if(IsConsoleMode())
		Console_Print("IsQuestCompleted >> %.0f", *result);

	return true;
}

static bool Cmd_GetStageIDs_Execute(COMMAND_ARGS)
{
	class Counter {
	public:
		Counter(ArrayID arrID) : m_arrID(arrID), m_curIndex(0) { }

		bool Accept(const TESQuest::StageEntry* entry) {
			if (entry == NULL)
				return false;

			g_ArrayMap.SetElementNumber(m_arrID, m_curIndex++, entry->index);
			return true;
		}

#if _DEBUG
		~Counter() {
			g_ArrayMap.DumpArray(m_arrID);
		}
#endif
	private:
		ArrayID m_arrID;
		UInt32	m_curIndex;
	};

	TESQuest* quest = NULL;
	ArrayID stageArray = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = stageArray;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &quest) && quest) {
		quest->stageList.Visit(Counter(stageArray));
	}

	return true;
}

static bool Cmd_GetStageEntries_Execute(COMMAND_ARGS)
{
	class EntryVisitor {
	public:
		EntryVisitor(UInt32 modIndex, ArrayID arrID) : m_curIdx(0), m_arrID(arrID), m_modIndex(modIndex) { }

		bool Accept(const QuestStageItem* item) {
			if (item == NULL)
				return false;

			ArrayID id = g_ArrayMap.Create(kDataType_String, false, m_modIndex);
			UInt16 day = 0, year = 0;
			UInt32 month = 0;
			if (item->logDate)
				item->logDate->Get(day, month, year);

			g_ArrayMap.SetElementNumber(id, "day", day);
			g_ArrayMap.SetElementNumber(id, "month", month);
			g_ArrayMap.SetElementNumber(id, "year", year);

			const char* text = item->GetLogText();
			if (!text)
				text = "";

			g_ArrayMap.SetElementString(id, "text", text);

			g_ArrayMap.SetElementArray(m_arrID, m_curIdx++, id);

			DEBUG_PRINT("%d/%d/%d %s",	day, month, year, text);

			return true;
		}
	private:
		UInt32	m_curIdx;
		ArrayID	m_arrID;
		UInt32	m_modIndex;
	};

	ArrayID arrID = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arrID;

	TESQuest* quest = NULL;
	UInt32 stage;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &quest, &stage)) {
		if (quest) {
			TESQuest::StageEntry* stageEntry = quest->GetStageEntry(stage);
			if (stageEntry) {
				stageEntry->itemList.Visit(EntryVisitor(scriptObj->GetModIndex(), arrID));
			}
		}
	}

	return true;
}

static bool Cmd_SetStageText_Execute(COMMAND_ARGS)
{
	TESQuest* quest = NULL;
	UInt32 stageID, itemID;
	char text[kMaxMessageLength] = { 0 };

	QuestStageItem* stageItem = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &quest, &stageID, &itemID, text) && quest) {
		TESQuest::StageEntry* stageEntry = quest->GetStageEntry(stageID);
		if (stageEntry) {
			stageItem = stageEntry->itemList.GetNthItem(itemID);
			if (stageItem) {
				SetQuestStageItemText(stageItem, text);
				*result = 1.0;
			}
		}
	}

	if (IsConsoleMode()) {
		Console_Print("SetStageText >> %.0f", *result);
	}

	if (stageItem) {
		DEBUG_PRINT("Set text to %s", stageItem->GetLogText());
	}

	return true;
}

static bool Cmd_UnsetStageText_Execute(COMMAND_ARGS)
{
	TESQuest* quest = NULL;
	UInt32 stageID, itemID;

	QuestStageItem* stageItem = NULL;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &quest, &stageID, &itemID) && quest) {
		TESQuest::StageEntry* stageEntry = quest->GetStageEntry(stageID);
		if (stageEntry) {
			stageItem = stageEntry->itemList.GetNthItem(itemID);
			if (stageItem) {
				UnsetQuestStageItemText(stageItem);
				*result = 1.0;
			}
		}
	}

	if (IsConsoleMode()) {
		Console_Print("UnsetStageText >> %.0f", *result);
	}

	return true;
}

static bool Cmd_SetStageDate_Execute(COMMAND_ARGS)
{
	TESQuest* quest = NULL;
	UInt32 stageID, itemID, d, m, y;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &quest, &stageID, &itemID, &d, &m, &y) && quest) {
		if (d > 0 && d <= 32 && m > 0 && m <= 12 && y <= 0xFFFF) {
			TESQuest::StageEntry* stageEntry = quest->GetStageEntry(stageID);
			if (stageEntry) {
				QuestStageItem* stageItem = stageEntry->itemList.GetNthItem(itemID);
				if (stageItem && stageItem->SetLogDate(QuestStageItem::LogDate::Create(d, m, y))) {
					*result = 1.0;
				}
			}
		}
	}

	if (IsConsoleMode()) {
		Console_Print("SetStageDate >> %.0f", *result);
	}

	return true;
}

#endif

DEFINE_COMMAND(QuestExists, returns 1 if a quest exists with the specified editorID, 0, 1, kParams_OneString);
DEFINE_COMMAND(GetActiveQuest, "returns the player's active quest, if any", 0, 0, NULL);
DEFINE_COMMAND(SetActiveQuest, sets the players active quest, 0, 1, kParams_OneQuest);
DEFINE_COMMAND(GetCurrentQuests, returns a list of all currently active (uncompleted) quests. Note that this list includes quests which do not appear in the journal,
			   0, 0, NULL);
DEFINE_COMMAND(GetCompletedQuests, returns a list of all completed quests, 0, 0, NULL);
DEFINE_COMMAND(IsQuestCompleted, returns 1 if the specified quest has been completed, 0, 1, kParams_OneQuest);
DEFINE_COMMAND(UncompleteQuest, marks a quest as not having been completed, 0, 1, kParams_OneQuest);
DEFINE_COMMAND(ClearActiveQuest, clears the players active quest, 0, 0, NULL);

DEFINE_COMMAND(GetStageIDs, returns an array of stage IDs for a quest, 0, 1, kParams_OneQuest);

static ParamInfo kParams_OneQuest_OneInt[2] =
{
	{ "quest", kParamType_Quest, 0 },
	{ "int", kParamType_Integer, 0 }
};

DEFINE_COMMAND(GetStageEntries, returns an array of structures representing the entries for a quest stage,
			   0, 2, kParams_OneQuest_OneInt);

static ParamInfo kParams_SetStageText[4] =
{
	{ "quest", kParamType_Quest, 0 },
	{ "stage", kParamType_Integer, 0 },
	{ "index", kParamType_Integer, 0 },
	{ "text", kParamType_String, 0 }
};

DEFINE_COMMAND(SetStageText, sets the text associated with a log entry in a quest stage, 0, 4, kParams_SetStageText);

static ParamInfo kParams_QuestStageItem[3] =
{
	{ "quest", kParamType_Quest, 0 },
	{ "stage", kParamType_Integer, 0 },
	{ "index", kParamType_Integer, 0 }
};

DEFINE_COMMAND(UnsetStageText, reverts any changes to log entry text, 0, 3, kParams_QuestStageItem);

static ParamInfo kParams_SetStageDate[6] =
{
	{ "quest", kParamType_Quest, 0 },
	{ "stage", kParamType_Integer, 0 },
	{ "index", kParamType_Integer, 0 },
	{ "day", kParamType_Integer, 0 },
	{ "month", kParamType_Integer, 0 },
	{ "year", kParamType_Integer, 0 }
};

DEFINE_COMMAND(SetStageDate, sets the log date for a quest log entry, 0, 6, kParams_SetStageDate);
