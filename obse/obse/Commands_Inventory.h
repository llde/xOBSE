#pragma once

#include "CommandTable.h"

// container functions
extern CommandInfo kCommandInfo_GetNumItems;
extern CommandInfo kCommandInfo_GetInventoryItemType;
extern CommandInfo kCommandInfo_GetInventoryObject;
extern CommandInfo kCommandInfo_GetContainerRespawns;
extern CommandInfo kCommandInfo_SetContainerRespawns;

// actor equipped inventory functions
extern CommandInfo kCommandInfo_GetEquipmentSlotType;
extern CommandInfo kCommandInfo_GetEquippedObject;
extern CommandInfo kCommandInfo_GetEquipmentSlotMask;
extern CommandInfo kCommandInfo_GetEquippedCurrentValue;
extern CommandInfo kCommandInfo_GetEquippedObjectValue;
extern CommandInfo kCommandInfo_SetEquippedObjectValue;
extern CommandInfo kCommandInfo_SetEquippedCurrentValue;

extern CommandInfo kCommandInfo_GetObjectValue;
extern CommandInfo kCommandInfo_GetCurrentValue;
extern CommandInfo kCommandInfo_GetBaseObject;
extern CommandInfo kCommandInfo_GetType;
//extern CommandInfo kCommandInfo_SetObjectValue;
//extern CommandInfo kCommandInfo_ModObjectValue;
//extern CommandInfo kCommandInfo_TestSetObjectValue;
//extern CommandInfo kCommandInfo_TestModObjectValue;
extern CommandInfo kCommandInfo_SetCurrentSoulLevel;

extern CommandInfo kCommandInfo_GetWeight;
extern CommandInfo kCommandInfo_SetWeight;
extern CommandInfo kCommandInfo_ModWeight;
extern CommandInfo kCommandInfo_GetGoldValue;
extern CommandInfo kCommandInfo_SetGoldValue;
extern CommandInfo kCommandInfo_ModGoldValue;
extern CommandInfo kCommandInfo_GetObjectHealth;
extern CommandInfo kCommandInfo_SetObjectHealth;
extern CommandInfo kCommandInfo_ModObjectHealth;
extern CommandInfo kCommandInfo_GetEquipmentSlot;
extern CommandInfo kCommandInfo_SetEquipmentSlot;
extern CommandInfo kCommandInfo_GetObjectCharge;
extern CommandInfo kCommandInfo_SetObjectCharge;
extern CommandInfo kCommandInfo_ModObjectCharge;
extern CommandInfo kCommandInfo_IsQuestItem;
extern CommandInfo kCommandInfo_SetQuestItem;
extern CommandInfo kCommandInfo_GetEnchantment;
extern CommandInfo kCommandInfo_SetEnchantment;
extern CommandInfo kCommandInfo_RemoveEnchantment;
extern CommandInfo kCommandInfo_GetAttackDamage;
extern CommandInfo kCommandInfo_SetAttackDamage;
extern CommandInfo kCommandInfo_ModAttackDamage;
extern CommandInfo kCommandInfo_GetWeaponReach;
extern CommandInfo kCommandInfo_SetWeaponReach;
extern CommandInfo kCommandInfo_ModWeaponReach;
extern CommandInfo kCommandInfo_GetWeaponSpeed;
extern CommandInfo kCommandInfo_SetWeaponSpeed;
extern CommandInfo kCommandInfo_ModWeaponSpeed;
extern CommandInfo kCommandInfo_GetWeaponType;
extern CommandInfo kCommandInfo_SetWeaponType;
extern CommandInfo kCommandInfo_GetIgnoresResistance;
extern CommandInfo kCommandInfo_SetIgnoresResistance;
extern CommandInfo kCommandInfo_GetArmorAR;
extern CommandInfo kCommandInfo_SetArmorAR;
extern CommandInfo kCommandInfo_ModArmorAR;
extern CommandInfo kCommandInfo_GetArmorType;
extern CommandInfo kCommandInfo_SetArmorType;
extern CommandInfo kCommandInfo_SoulLevel;
extern CommandInfo kCommandInfo_SetSoulLevel;
extern CommandInfo kCommandInfo_GetSoulGemCapacity;
extern CommandInfo kCommandInfo_SetSoulGemCapacity;
extern CommandInfo kCommandInfo_IsFood;
extern CommandInfo kCommandInfo_SetIsFood;
extern CommandInfo kCommandInfo_IsPoison;
extern CommandInfo kCommandInfo_SetIsPoison;
extern CommandInfo kCommandInfo_SetName;
extern CommandInfo kCommandInfo_CompareName;
extern CommandInfo kCommandInfo_CopyName;
extern CommandInfo kCommandInfo_ModName;
extern CommandInfo kCommandInfo_AppendToName;
extern CommandInfo kCommandInfo_SetModelPath;
extern CommandInfo kCommandInfo_SetIconPath;
extern CommandInfo kCommandInfo_SetMaleBipedPath;
extern CommandInfo kCommandInfo_SetFemaleBipedPath;
extern CommandInfo kCommandInfo_SetMaleGroundPath;
extern CommandInfo kCommandInfo_SetFemaleGroundPath;
extern CommandInfo kCommandInfo_SetMaleIconPath;
extern CommandInfo kCommandInfo_SetFemaleIconPath;
extern CommandInfo kCommandInfo_GetBookCantBeTaken;
extern CommandInfo kCommandInfo_SetBookCantBeTaken;
extern CommandInfo kCommandInfo_GetBookIsScroll;
extern CommandInfo kCommandInfo_SetBookIsScroll;
extern CommandInfo kCommandInfo_GetBookSkillTaught;
extern CommandInfo kCommandInfo_SetBookSkillTaught;
extern CommandInfo kCommandInfo_SetBookSkillTaughtC;
extern CommandInfo kCommandInfo_GetApparatusType;
extern CommandInfo kCommandInfo_SetApparatusType;
extern CommandInfo kCommandInfo_GetQuality;
extern CommandInfo kCommandInfo_SetQuality;
extern CommandInfo kCommandInfo_ModQuality;

extern CommandInfo kCommandInfo_SetModelPath;
extern CommandInfo kCommandInfo_SetIconPath;
extern CommandInfo kCommandInfo_SetMaleBipedPath;
extern CommandInfo kCommandInfo_SetFemaleBipedPath;
extern CommandInfo kCommandInfo_SetMaleGroundPath;
extern CommandInfo kCommandInfo_SetFemaleGroundPath;
extern CommandInfo kCommandInfo_SetMaleIconPath;
extern CommandInfo kCommandInfo_SetFemaleIconPath;

extern CommandInfo kCommandInfo_ModModelPath;
extern CommandInfo kCommandInfo_ModIconPath;
extern CommandInfo kCommandInfo_ModMaleBipedPath;
extern CommandInfo kCommandInfo_ModFemaleBipedPath;
extern CommandInfo kCommandInfo_ModMaleGroundPath;
extern CommandInfo kCommandInfo_ModFemaleGroundPath;
extern CommandInfo kCommandInfo_ModMaleIconPath;
extern CommandInfo kCommandInfo_ModFemaleIconPath;

extern CommandInfo kCommandInfo_CompareModelPath;
extern CommandInfo kCommandInfo_CompareIconPath;
extern CommandInfo kCommandInfo_CompareMaleBipedPath;
extern CommandInfo kCommandInfo_CompareFemaleBipedPath;
extern CommandInfo kCommandInfo_CompareMaleGroundPath;
extern CommandInfo kCommandInfo_CompareFemaleGroundPath;
extern CommandInfo kCommandInfo_CompareMaleIconPath;
extern CommandInfo kCommandInfo_CompareFemaleIconPath;

extern CommandInfo kCommandInfo_CopyModelPath;
extern CommandInfo kCommandInfo_CopyIconPath;
extern CommandInfo kCommandInfo_CopyMaleBipedPath;
extern CommandInfo kCommandInfo_CopyFemaleBipedPath;
extern CommandInfo kCommandInfo_CopyMaleGroundPath;
extern CommandInfo kCommandInfo_CopyFemaleGroundPath;
extern CommandInfo kCommandInfo_CopyMaleIconPath;
extern CommandInfo kCommandInfo_CopyFemaleIconPath;

extern CommandInfo kCommandInfo_SetCurrentHealth;

extern CommandInfo kCommandInfo_GetEquippedCurrentHealth;
extern CommandInfo kCommandInfo_SetEquippedCurrentHealth;
extern CommandInfo kCommandInfo_ModEquippedCurrentHealth;
extern CommandInfo kCommandInfo_GetEquippedCurrentCharge;
extern CommandInfo kCommandInfo_SetEquippedCurrentCharge;
extern CommandInfo kCommandInfo_ModEquippedCurrentCharge;
extern CommandInfo kCommandInfo_GetEquippedWeaponPoison;
extern CommandInfo kCommandInfo_SetEquippedWeaponPoison;
extern CommandInfo kCommandInfo_RemoveEquippedWeaponPoison;

extern CommandInfo kCommandInfo_GetCurrentHealth;
extern CommandInfo kCommandInfo_GetCurrentCharge;
extern CommandInfo kCommandInfo_GetCurrentSoulLevel;

extern CommandInfo kCommandInfo_IsPlayable;
extern CommandInfo kCommandInfo_SetPlayable;

extern CommandInfo kCommandInfo_IsWeapon;
extern CommandInfo kCommandInfo_IsAmmo;
extern CommandInfo kCommandInfo_IsArmor;
extern CommandInfo kCommandInfo_IsClothing;
extern CommandInfo kCommandInfo_IsBook;
extern CommandInfo kCommandInfo_IsIngredient;
extern CommandInfo kCommandInfo_IsContainer;
extern CommandInfo kCommandInfo_IsKey;
extern CommandInfo kCommandInfo_IsAlchemyItem;
extern CommandInfo kCommandInfo_IsApparatus;
extern CommandInfo kCommandInfo_IsSoulGem;
extern CommandInfo kCommandInfo_IsSigilStone;
extern CommandInfo kCommandInfo_IsDoor;
extern CommandInfo kCommandInfo_IsActivator;
extern CommandInfo kCommandInfo_IsLight;
extern CommandInfo kCommandInfo_IsFurniture;
extern CommandInfo kCommandInfo_IsMiscItem;

extern CommandInfo kCommandInfo_IsClonedForm;
extern CommandInfo kCommandInfo_CloneForm;

extern CommandInfo kCommandInfo_CompareNames;

extern CommandInfo kCommandInfo_IsLightCarriable;
extern CommandInfo kCommandInfo_GetLightRadius;
extern CommandInfo kCommandInfo_SetLightRadius;

extern CommandInfo kCommandInfo_HasName;

extern CommandInfo kCommandInfo_AddItemNS;
extern CommandInfo kCommandInfo_RemoveItemNS;
extern CommandInfo kCommandInfo_EquipItemNS;
extern CommandInfo kCommandInfo_UnequipItemNS;
extern CommandInfo kCommandInfo_IsPlayable2;

extern CommandInfo kCommandInfo_GetFullGoldValue;
extern CommandInfo kCommandInfo_GetHotKeyItem;
extern CommandInfo kCommandInfo_ClearHotKey;
extern CommandInfo kCommandInfo_SetHotKeyItem;

extern CommandInfo kCommandInfo_IsModelPathValid;
extern CommandInfo kCommandInfo_IsIconPathValid;
extern CommandInfo kCommandInfo_IsBipedModelPathValid;
extern CommandInfo kCommandInfo_IsBipedIconPathValid;
extern CommandInfo kCommandInfo_FileExists;

extern CommandInfo kCommandInfo_SetNameEx;

extern CommandInfo kCommandInfo_GetHidesRings;
extern CommandInfo kCommandInfo_GetHidesAmulet;
extern CommandInfo kCommandInfo_SetHidesRings;
extern CommandInfo kCommandInfo_SetHidesAmulet;
extern CommandInfo kCommandInfo_IsFlora;

extern CommandInfo kCommandInfo_GetBipedSlotMask;
extern CommandInfo kCommandInfo_SetBipedSlotMask;

extern CommandInfo kCommandInfo_GetItems;
extern CommandInfo kCommandInfo_GetBaseItems;

extern CommandInfo kCommandInfo_SetCurrentCharge;
extern CommandInfo kCommandInfo_ModCurrentCharge;

extern CommandInfo kCommandInfo_EquipItem2;
extern CommandInfo kCommandInfo_EquipItem2NS;
extern CommandInfo kCommandInfo_EquipMe;
extern CommandInfo kCommandInfo_UnequipMe;

extern CommandInfo kCommandInfo_IsEquipped;

extern CommandInfo kCommandInfo_EquipItemSilent;
extern CommandInfo kCommandInfo_UnequipItemSilent;

extern CommandInfo kCommandInfo_GetEquippedTorchTimeLeft;
extern CommandInfo kCommandInfo_SetGoldValue_T;