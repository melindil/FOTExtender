/* MIT License

Copyright (c) 2018 melindil

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "HookExecutor.h"
#include "Entity.h"
#include "Logger.h"
#include "FOTPerkTable.h"
#include "AttributesTable.h"
#include "LuaHelper.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>



//const int ENTITY_OFFSET_ATTRIBUTES = 0x2a2;
//const int ENTITY_OFFSET_TEMP_ATTRIBUTES = 0x914;

//const int INDEX_DERIVED_RADRESIST = 3;
//const int INDEX_DERIVED_NORMALRESIST = 14;

//const int ATTRIBUTES_SIZE = 0x339;

const uint32_t DATA_PERK_TABLE = 0x8a4500;
//const DWORD FXN_ENTITY_SHOWMESSAGE = 0x5e6d20;
//const DWORD FXN_ATTRIBUTES_CONSTRUCTOR = 0x608d30;
//const DWORD FXN_ENTITY_APPLYBUFF = 0x56f300;
//const DWORD FXN_ENTITY_REMOVEBUFF = 0x56f510;

// Lua stubs
int l_replaceperk(lua_State* l)
{
	HookExecutor** he = (HookExecutor**)luaL_checkudata(l, 1, "HookExecutor");
	if (lua_istable(l, 2) && lua_isnumber(l, 3))
	{
		(*he)->ReplacePerk(l);
	}
	return 0;

}

HookExecutor::HookExecutor(Logger* logger)
	: logger_(logger)
{
	lua_ = luaL_newstate();
	luaL_openlibs(lua_);
	luaL_dofile(lua_, "fte.lua");

	logger->RegisterLUA(lua_);
	Entity::RegisterLua(lua_, logger);

	// Register HookExecutor functions for Lua
	HookExecutor** heptrptr = (HookExecutor**)lua_newuserdata(lua_, sizeof(HookExecutor*));
	*heptrptr = this;
	luaL_newmetatable(lua_, "HookExecutor");
	lua_pushcfunction(lua_, l_replaceperk);
	lua_setfield(lua_, -2, "ReplacePerk");
	lua_pushvalue(lua_, -1);
	lua_setfield(lua_, -2, "__index");
	lua_setmetatable(lua_, -2);
	lua_setglobal(lua_, "hookexecutor");
	
}

//static const DWORD FXN_FOTHEAPALLOC = 0x6c4dd0;
//static char* (*FOTHeapAlloc)(DWORD) = (char* (*)(DWORD))FXN_FOTHEAPALLOC;

HookExecutor::~HookExecutor()
{
}

/*void HookExecutor::ShowEntityMessage(void* entity, WCHAR const* msg)
{
	DummyClass* c1 = (DummyClass*)entity;
	auto fxn = &DummyClass::Entity_ShowMessage;
	size_t offset = FXN_ENTITY_SHOWMESSAGE;
	memcpy(&fxn, &offset, 4);

	// Things work much better if we let FoT allocate the memory for the
	// message.  Note that there are three DWORDs before the message
	// content: A usage counter (which should start at 0 in this code
	// location), an entity size?, and a string length in chars
	DWORD len = 14 + 2 * wcslen(msg);
	char* alloced = FOTHeapAlloc(len);

	*((DWORD*)alloced) = 0;	// Ref count
	*((DWORD*)alloced + 1) = wcslen(msg);
	*((DWORD*)alloced + 2) = wcslen(msg);
	alloced += 12;
	wcscpy_s((WCHAR*)alloced, (len - 12) / 2, msg);

	(c1->*fxn)(((DWORD)&alloced), 0x8be1c8);
	
}*/

void HookExecutor::ReplacePerk(FOTPerkTableEntry* newperk, int entry)
{
	FOTPerkTableEntry* perkarray = (FOTPerkTableEntry*)DATA_PERK_TABLE;
	perkarray += entry;
	DWORD old_protect;
	VirtualProtect(perkarray, sizeof(FOTPerkTableEntry), PAGE_EXECUTE_READWRITE, &old_protect);
	memcpy(perkarray, newperk, sizeof(FOTPerkTableEntry));
	VirtualProtect(perkarray, sizeof(FOTPerkTableEntry), old_protect, &old_protect);
}

void HookExecutor::ReplacePerk(lua_State* l)
{
	*logger_ << "Lua called ReplacePerk" << std::endl;

	int entry = (int)lua_tointeger(l, 3);
	*logger_ << "Entry " << entry << std::endl;
	FOTPerkTableEntry newperk;

	newperk.perkShortName = LuaHelper::GetPermTableString(l, 2, "name");

	newperk.reqLevel = LuaHelper::GetTableInteger(l, 2, "minlevel");
	*logger_ << "Perk min level: " << newperk.reqLevel << std::endl;
	newperk.maxLevels = LuaHelper::GetTableInteger(l, 2, "maxperktaken");
	*logger_ << "Perk num levels: " << newperk.maxLevels << std::endl;

	newperk.perkBonusStat = LuaHelper::GetPermTableString(l, 2, "bonusstat");
	*logger_ << "BonusStat: " << newperk.perkBonusStat << std::endl;
	newperk.perkBonusAmt = LuaHelper::GetTableInteger(l, 2, "bonusamt");
	*logger_ << "BonusAmt: " << newperk.perkBonusAmt << std::endl;

	newperk.perkReqStat1 = LuaHelper::GetPermTableString(l, 2, "requiredstat1");
	*logger_ << "Perk required stat 1: " << newperk.perkReqStat1 << std::endl;
	newperk.perkReqAmt1 = LuaHelper::GetTableInteger(l, 2, "requiredamt1");
	*logger_ << "Perk required amt 1: " << newperk.perkReqAmt1 << std::endl;

	newperk.perkAndOrFlag = LuaHelper::GetTableInteger(l, 2, "and_or_flag");
	*logger_ << "Perk And/Or: " << newperk.perkAndOrFlag << std::endl;

	newperk.perkReqStat2 = LuaHelper::GetPermTableString(l, 2, "requiredstat2");
	*logger_ << "Perk required stat 2: " << newperk.perkReqStat2 << std::endl;
	newperk.perkReqAmt2 = LuaHelper::GetTableInteger(l, 2, "requiredamt2");
	*logger_ << "Perk required amt 2: " << newperk.perkReqAmt2 << std::endl;

	newperk.perkReqST = LuaHelper::GetTableInteger(l, 2, "requiredST");
	newperk.perkReqPE = LuaHelper::GetTableInteger(l, 2, "requiredPE");
	newperk.perkReqEN = LuaHelper::GetTableInteger(l, 2, "requiredEN");
	newperk.perkReqCH = LuaHelper::GetTableInteger(l, 2, "requiredCH");
	newperk.perkReqIN = LuaHelper::GetTableInteger(l, 2, "requiredIN");
	newperk.perkReqAG = LuaHelper::GetTableInteger(l, 2, "requiredAG");
	newperk.perkReqLK = LuaHelper::GetTableInteger(l, 2, "requiredLK");
	*logger_ << "Perk required ST: " << newperk.perkReqST << std::endl;
	*logger_ << "Perk required PE: " << newperk.perkReqPE << std::endl;
	*logger_ << "Perk required EN: " << newperk.perkReqEN << std::endl;
	*logger_ << "Perk required CH: " << newperk.perkReqCH << std::endl;
	*logger_ << "Perk required IN: " << newperk.perkReqIN << std::endl;
	*logger_ << "Perk required AG: " << newperk.perkReqAG << std::endl;
	*logger_ << "Perk required LK: " << newperk.perkReqLK << std::endl;

	newperk.perkRestrictedRace = LuaHelper::GetPermTableString(l, 2, "prohibitedRace");
	*logger_ << "Perk prohibited race: " << newperk.perkRestrictedRace << std::endl;
	newperk.perkRequiredRace = LuaHelper::GetPermTableString(l, 2, "requiredRace");
	*logger_ << "Perk required race: " << newperk.perkRequiredRace << std::endl;

	*logger_ << "Adding perk " << newperk.perkShortName << std::endl;
	ReplacePerk(&newperk, entry);

}

// Testing trigger - fires every 10 seconds when an entity has Team Player bonus.
// Added this trigger since I was familiar with the code in that area.
void HookExecutor::TeamPlayerTrigger(void* entity)
{
	//static const WCHAR* tpmsg = L"<Cg>HULK SMASH!";
	//ShowEntityMessage(entity, tpmsg);
	std::stringstream ss;
	ss << "TeamPlayer trigger, entity address 0x" << std::hex << (size_t)(entity) << std::dec;
	logger_->Log(ss.str());
}

// Trigger for Hulk Smash start - character is irradiated

void HookExecutor::IsRadiated(void* entity)
{
	/* old code
	if (EntityHasPerk(entity, 18))
	{
		*logger_ << "Entity 0x" << (uint32_t)entity << " radiated with perk" << std::endl;

		if (GetEntityTempPerkValue(entity, 18) == 0)
		{
			*logger_ << "Entity 0x" << (uint32_t)entity << " gaining Hulk Smash" << std::endl;

			// Indicate bonus to player
			static const WCHAR* tpmsg = L"<Cg>HULK SMASH!";
			ShowEntityMessage(entity, tpmsg);

			// Apply the bonus
			
			// Let FOT construct a temp attribute table for us
			std::vector<char> temp_attribute_table(ATTRIBUTES_SIZE, 0);
			DummyClass* c1 = (DummyClass*)temp_attribute_table.data();
			auto fxn = &DummyClass::AttributeTable_Constructor;
			size_t offset = FXN_ATTRIBUTES_CONSTRUCTOR;
			memcpy(&fxn, &offset, 4);
			(c1->*fxn)();

			DWORD* statsptr = (DWORD*)(temp_attribute_table.data()
				+ AttributesTable::GetOffsetByName("strength")); // offset to table
			*statsptr = 2;

			statsptr = (DWORD*)(temp_attribute_table.data()
				+ AttributesTable::GetOffsetByName("radiationResist"));
			*statsptr = 75;

			// Now we need to call the routine that applies the bonus
			c1 = (DummyClass*)entity;
			auto fxn2 = &DummyClass::ApplyBuff;
			size_t offset2 = FXN_ENTITY_APPLYBUFF;
			memcpy(&fxn2, &offset2, 4);
			(c1->*fxn2)(temp_attribute_table.data(), 0, 1.0);
			
		}
		else
		{
			*logger_  << "Entity 0x" << (uint32_t)entity << " refreshing Hulk Smash" << std::endl;
		}
		SetEntityTempPerkValue(entity, 18, 6);
	}*/
	lua_getglobal(lua_, "OnRadiated");
	if (lua_isfunction(lua_, -1))
	{
		Entity e(entity);
		e.MakeLuaObject(lua_);
		lua_pcall(lua_, 1, 0, 0);

	}

}

// Trigger for "long" (10 sec) game tick
void HookExecutor::LongTickTrigger(void* entity)
{
	/* old (working) code
	DWORD hulksmashctr = GetEntityTempPerkValue(entity, 18);
	if (hulksmashctr > 0)
	{
		hulksmashctr--;
		SetEntityTempPerkValue(entity, 18, hulksmashctr);
		if (hulksmashctr == 0)
		{
			// Remove the perk
			static const WCHAR* tpmsg = L"Normal";
			ShowEntityMessage(entity, tpmsg);

			// Remove the bonus

			// Let FOT construct a temp attribute table for us
			std::vector<char> temp_attribute_table(ATTRIBUTES_SIZE, 0);
			DummyClass* c1 = (DummyClass*)temp_attribute_table.data();
			auto fxn = &DummyClass::AttributeTable_Constructor;
			size_t offset = FXN_ATTRIBUTES_CONSTRUCTOR;
			memcpy(&fxn, &offset, 4);
			(c1->*fxn)();

			DWORD* statsptr = (DWORD*)(temp_attribute_table.data()
				+ AttributesTable::GetOffsetByName("strength")); // offset to table
			*statsptr = 2;

			statsptr = (DWORD*)(temp_attribute_table.data()
				+ AttributesTable::GetOffsetByName("radiationResist"));
			*statsptr = 75;

			// Now we need to call the routine that applies the bonus
			c1 = (DummyClass*)entity;
			auto fxn2 = &DummyClass::ApplyBuff;
			size_t offset2 = FXN_ENTITY_REMOVEBUFF;
			memcpy(&fxn2, &offset2, 4);
			(c1->*fxn2)(temp_attribute_table.data(), 0, 1.0);


		}
	}*/
	lua_getglobal(lua_, "OnLongTick");
	if (lua_isfunction(lua_, -1))
	{
		Entity e(entity);
		e.MakeLuaObject(lua_);
		lua_pcall(lua_, 1, 0, 0);

	}

}

/*bool HookExecutor::EntityHasPerk(void* entity, int perknum)
{
	DWORD* perkptr = (DWORD*)
		((char*)entity
			+ ENTITY_OFFSET_ATTRIBUTES	// start of standard attributes
			+ AttributesTable::OFFSET_PERKS);				// start of perk table

	return perkptr[perknum] != 0;
}*/

/*DWORD HookExecutor::GetEntityTempPerkValue(void* entity, int perknum)
{
	DWORD* perkptr = (DWORD*)
		((char*)entity
			+ ENTITY_OFFSET_TEMP_ATTRIBUTES	// start of standard attributes
			+ AttributesTable::OFFSET_PERKS);				// start of perk table
	return perkptr[perknum];
}*/

/*void HookExecutor::SetEntityTempPerkValue(void* entity, int perknum, DWORD value)
{
	DWORD* perkptr = (DWORD*)
		((char*)entity
			+ ENTITY_OFFSET_TEMP_ATTRIBUTES	// start of standard attributes
			+ AttributesTable::OFFSET_PERKS);				// start of perk table

	perkptr[perknum] = value;
}*/


void HookExecutor::OnStart()
{

	lua_getglobal(lua_, "OnStart");
	if (lua_isfunction(lua_, -1))
	{
		lua_pcall(lua_, 0, 0, 0);
	}

	// Initialize AttributesTable after OnStart, to pick up any perks
	// NOTE that this means OnStart shouldn't touch any entities (none are in memory yet anyway)
	AttributesTable::Initialize(logger_);

}