/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#ifndef DBCFORMAT_H
#define DBCFORMAT_H

#include "Defines.h"

typedef struct DBCFileListStruct
{
    const char* filename;
    const char* format;
    std::string col_name[300];
    std::string col_desc[300];
} DBCFLS;

void FillDBCColumnNames(); // 246 (MAX_DBCS) @ 3.3.3a.11723

static DBCFLS DBCFileList[] =
{
    {"Achievement.dbc", "iiiissssssssssssssssissssssssssssssssiiiiiissssssssssssssssiii"},
    {"Achievement_Category.dbc", "iissssssssssssssssii"},
    {"Achievement_Criteria.dbc", "iiiiiiiiissssssssssssssssiiiiii"},
    {"AnimationData.dbc", "isiiiiii"},
    {"AreaGroup.dbc", "iiiiiiii"},
    {"AreaPOI.dbc", "iiiiiiiiiiiifffiiissssssssssssssssissssssssssssssssiii"},
    {"AreaTable.dbc", "iiiiiiiiiiissssssssssssssssiiiiiiffi"},
    {"AreaTrigger.dbc", "iiffffffff"},
    {"AttackAnimKits.dbc", "iiiii"},
    {"AttackAnimTypes.dbc", "is"},
    {"AuctionHouse.dbc", "iiiissssssssssssssssi"},
    {"BankBagSlotPrices.dbc", "ii"},
    {"BannedAddOns.dbc", "iiiiiiiiiii"},
    {"BarberShopStyle.dbc", "iissssssssssssssssissssssssssssssssifiii"},
    {"BattlemasterList.dbc", "iiiiiiiiiiissssssssssssssssiiiii"},
    {"CameraShakes.dbc", "iiifffff"},
    {"Cfg_Categories.dbc", "iiiiiiiiiiiiiiiiiiiii"},
    {"Cfg_Configs.dbc", "iiii"},
    {"CharacterFacialHairStyles.dbc", "iiiiiiii"},
    {"CharBaseInfo.dbc", "bb"},
    {"CharHairGeosets.dbc", "iiiiii"},
    {"CharHairTextures.dbc", "iiiiiiii"},
    {"CharSections.dbc", "iiiisssiii"},
    {"CharStartOutfit.dbc", "ibbbbiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"CharTitles.dbc", "iissssssssssssssssissssssssssssssssii"},
    {"CharVariations.dbc", ""},
    {"ChatChannels.dbc", "iisssssssssssssssssissssssssssssssssi"},
    {"ChatProfanity.dbc", "isi"},
    {"ChrClasses.dbc", "iiiissssssssssssssssissssssssssssssssissssssssssssssssiiiiii"},
    {"ChrRaces.dbc", "iiiiiisfiiiisissssssssssssssssissssssssssssssssissssssssssssssssisssi"},
    {"CinematicCamera.dbc", "isiffff"},
    {"CinematicSequences.dbc", "iiiiiiiiii"},
    {"CreatureDisplayInfo.dbc", "iiiifisssiiiiiii"},
    {"CreatureDisplayInfoExtra.dbc", "iiiiiiiiiiiiiiiiiiiis"},
    {"CreatureFamily.dbc", "ififiiiiiissssssssssssssssis"},
    {"CreatureModelData.dbc", "iisiisiffffiiiifffffffffffff"},
    {"CreatureMovementInfo.dbc", "ii"},
    {"CreatureSoundData.dbc", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"CreatureSpellData.dbc", "iiiiiiiii"},
    {"CreatureType.dbc", "issssssssssssssssii"},
    {"CurrencyCategory.dbc", "iissssssssssssssssi"},
    {"CurrencyTypes.dbc", "iiii"},
    {"DanceMoves.dbc", "iiisisssssssssssssssssii"},
    {"DeathThudLookups.dbc", "iiiii"},
    {"DeclinedWord.dbc", "is"},
    {"DeclinedWordCases.dbc", "iiis"},
    {"DestructibleModelData.dbc", "iiiiiiiiiiiiiiiiiii"},
    {"DungeonEncounter.dbc", "iiiiissssssssssssssssii"},
    {"DungeonMap.dbc", "iiiffffi"},
    {"DungeonMapChunk.dbc", "iiiif"},
    {"DurabilityCosts.dbc", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"DurabilityQuality.dbc", "if"},
    {"Emotes.dbc", "isiiiii"},
    {"EmotesText.dbc", "isiiiiiiiiiiiiiiiii"},
    {"EmotesTextData.dbc", "issssssssssssssssi"},
    {"EmotesTextSound.dbc", "iiiii"},
    {"EnvironmentalDamage.dbc", "iii"},
    {"Exhaustion.dbc", "iifffssssssssssssssssif"},
    {"Faction.dbc", "iiiiiiiiiiiiiiiiiiiiiiissssssssssssssssissssssssssssssssi"},
    {"FactionGroup.dbc", "iisssssssssssssssssi"},
    {"FactionTemplate.dbc", "iiiiiiiiiiiiii"},
    {"FileData.dbc", "isi"},
    {"FootprintTextures.dbc", "is"},
    {"FootstepTerrainLookup.dbc", "iiiii"},
    {"GameObjectArtKit.dbc", "iiiissss"},
    {"GameObjectDisplayInfo.dbc", "isiiiiiiiiiifffffff"},
    {"GameTables.dbc", "sii"},
    {"GameTips.dbc", "issssssssssssssssi"},
    {"GemProperties.dbc", "iiiii"},
    {"GlyphProperties.dbc", "iiii"},
    {"GlyphSlot.dbc", "iii"},
    {"GMSurveyAnswers.dbc", "iiissssssssssssssssi"},
    {"GMSurveyCurrentSurvey.dbc", "ii"},
    {"GMSurveyQuestions.dbc", "issssssssssssssssi"},
    {"GMSurveySurveys.dbc", "iiiiiiiiiii"},
    {"GMTicketCategory.dbc", "issssssssssssssssi"},
    {"GroundEffectDoodad.dbc", "iis"},
    {"GroundEffectTexture.dbc", "iiiiiiiiiii"},
    {"gtBarberShopCostBase.dbc", "f"},
    {"gtChanceToMeleeCrit.dbc", "f"},
    {"gtChanceToMeleeCritBase.dbc", "f"},
    {"gtChanceToSpellCrit.dbc", "f"},
    {"gtChanceToSpellCritBase.dbc", "f"},
    {"gtCombatRatings.dbc", "f"},
    {"gtNPCManaCostScaler.dbc", "f"},
    {"gtOCTClassCombatRatingScalar.dbc", "if"},
    {"gtOCTRegenHP.dbc", "f"},
    {"gtOCTRegenMP.dbc", "f"},
    {"gtRegenHPPerSpt.dbc", "f"},
    {"gtRegenMPPerSpt.dbc", "f"},
    {"HelmetGeosetVisData.dbc", "iiiiiiii"},
    {"HolidayDescriptions.dbc", "issssssssssssssssi"},
    {"HolidayNames.dbc", "issssssssssssssssi"},
    {"Holidays.dbc", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiisiii"},
    {"Item.dbc", "iiiiiiii"},
    {"ItemBagFamily.dbc", "issssssssssssssssi"},
    {"ItemClass.dbc", "iiissssssssssssssssi"},
    {"ItemCondExtCosts.dbc", "iiii"},
    {"ItemDisplayInfo.dbc", "issssssiiiiiiiissssssssii"},
    {"ItemExtendedCost.dbc", "iiiiiiiiiiiiiiii"},
    {"ItemGroupSounds.dbc", "iiiii"},
    {"ItemLimitCategory.dbc", "issssssssssssssssiii"},
    {"ItemPetFood.dbc", "issssssssssssssssi"},
    {"ItemPurchaseGroup.dbc", "iiiissssssssssssssssiiiiii"},
    {"ItemRandomProperties.dbc", "isiiiiissssssssssssssssi"},
    {"ItemRandomSuffix.dbc", "issssssssssssssssisiiiiiiiiii"},
    {"ItemSet.dbc", "issssssssssssssssiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"ItemSubClass.dbc", "iiiiiiiiiissssssssssssssssissssssssssssssssi"},
    {"ItemSubClassMask.dbc", "iissssssssssssssssi"},
    {"ItemVisualEffects.dbc", "is"},
    {"ItemVisuals.dbc", "iiiiii"},
    {"Languages.dbc", "issssssssssssssssi"},
    {"LanguageWords.dbc", "iis"},
    {"LFGDungeonExpansion.dbc", "iiiiiiii"},
    {"LFGDungeonGroup.dbc", "issssssssssssssssiiii"},
    {"LFGDungeons.dbc", "issssssssssssssssiiiiiiiiiiiiiiissssssssssssssssi"},
    {"Light.dbc", "iifffffiiiiiiii"},
    {"LightFloatBand.dbc", "iiiiffffffffffffffffffffffffffffff"},
    {"LightIntBand.dbc", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"LightParams.dbc", "iiiifffff"},
    {"LightSkybox.dbc", "isi"},
    {"LiquidMaterial.dbc", "iii"},
    {"LiquidType.dbc", "isiiiiiffffifiiiiiiiiiiiffffiiiifffffffffffii"},
    {"LoadingScreens.dbc", "isss"},
    {"LoadingScreenTaxiSplines.dbc", "iifffffffifffffffii"},
    {"Lock.dbc", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"LockType.dbc", "issssssssssssssssissssssssssssssssissssssssssssssssis"},
    {"MailTemplate.dbc", "issssssssssssssssissssssssssssssssi"},
    {"Map.dbc", "isiisssssssssssssssssiissssssssssssssssissssssssssssssssiififfiiii"},
    {"MapDifficulty.dbc", "iiissssssssssssssssiiii"},
    {"Material.dbc", "iiiii"},
    {"Movie.dbc", "isi"},
    {"MovieFileData.dbc", "ii"},
    {"MovieVariation.dbc", "iii"},
    {"NameGen.dbc", "isii"},
    {"NamesProfanity.dbc", "isi"},
    {"NamesReserved.dbc", "isi"},
    {"NPCSounds.dbc", "iiiii"},
    {"ObjectEffect.dbc", "isiiiiiififi"},
    {"ObjectEffectGroup.dbc", "is"},
    {"ObjectEffectModifier.dbc", "iiiiffff"},
    {"ObjectEffectPackage.dbc", "is"},
    {"ObjectEffectPackageElem.dbc", "iiii"},
    {"OverrideSpellData.dbc", "iiiiiiiiiiii"},
    {"Package.dbc", "iiissssssssssssssssi"},
    {"PageTextMaterial.dbc", "is"},
    {"PaperDollItemFrame.dbc", "ssi"},
    {"ParticleColor.dbc", "ibbbbbbbbb"},
    {"PetitionType.dbc", "isi"},
    {"PetPersonality.dbc", "issssssssssssssssiiiffff"},
    {"PowerDisplay.dbc", "iiiiii"},
    {"PvpDifficulty.dbc", "iiiiii"},
    {"QuestFactionReward.dbc", "iiiiiiiiiii"},
    {"QuestInfo.dbc", "issssssssssssssssi"},
    {"QuestSort.dbc", "issssssssssssssssi"},
    {"QuestXP.dbc", "iiiiiiiiiii"},
    {"RandPropPoints.dbc", "iiiiiiiiiiiiiiii"},
    {"Resistances.dbc", "iiissssssssssssssssi"},
    {"ScalingStatDistribution.dbc", "iiiiiiiiiiiiiiiiiiiiii"},
    {"ScalingStatValues.dbc", "iiiiiiiiiiiiiiiiiiiiiiii"},
    {"ScreenEffect.dbc", "isiiiiiiii"},
    {"ServerMessages.dbc", "issssssssssssssssi"},
    {"SheatheSoundLookups.dbc", "iiiiiii"},
    {"SkillCostsData.dbc", "iiiii"},
    {"SkillLine.dbc", "iiissssssssssssssssissssssssssssssssiissssssssssssssssii"},
    {"SkillLineAbility.dbc", "iiiiiiiiiiiiii"},
    {"SkillLineCategory.dbc", "issssssssssssssssii"},
    {"SkillRaceClassInfo.dbc", "iiiiiiii"},
    {"SkillTiers.dbc", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"SoundAmbience.dbc", "iii"},
    {"SoundEmitters.dbc", "iffffffiis"},
    {"SoundEntries.dbc", "iisssssssssssiiiiiiiiiisfiffii"},
    {"SoundEntriesAdvanced.dbc", "iifiiiiiiiiifffiiiiffffs"},
    {"SoundFilter.dbc", "is"},
    {"SoundFilterElem.dbc", "iiiifffffffff"},
    {"SoundProviderPreferences.dbc", "isiifffiifififffifffffff"},
    {"SoundSamplePreferences.dbc", "iiiiiifffffffffff"},
    {"SoundWaterType.dbc", "iiii"},
    {"SpamMessages.dbc", "is"},
    {"Spell.dbc", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifiiiiiiiiiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffiiiiiiiiiiiiiissssssssssssssssissssssssssssssssiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiii"},
    {"SpellCastTimes.dbc", "iiii"},
    {"SpellCategory.dbc", "ii"},
    {"SpellChainEffects.dbc", "iiiiiiisiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"SpellDescriptionVariables.dbc", "is"},
    {"SpellDifficulty.dbc", "iiiii"},
    {"SpellDispelType.dbc", "issssssssssssssssiiis"},
    {"SpellDuration.dbc", "iiii"},
    {"SpellEffectCameraShakes.dbc", "iiii"},
    {"SpellFocusObject.dbc", "issssssssssssssssi"},
    {"SpellIcon.dbc", "is"},
    {"SpellItemEnchantment.dbc", "iiiiiiiiiiiiiissssssssssssssssiiiiiiii"},
    {"SpellItemEnchantmentCondition.dbc", "ibbbbbiiiiibbbbbbbbbbiiiiibbbbb"},
    {"SpellMechanic.dbc", "issssssssssssssssi"},
    {"SpellMissile.dbc", "iiffffffffffffi"},
    {"SpellMissileMotion.dbc", "issii"},
    {"SpellRadius.dbc", "ifif"},
    {"SpellRange.dbc", "iffffissssssssssssssssissssssssssssssssi"},
    {"SpellRuneCost.dbc", "iiiii"},
    {"SpellShapeshiftForm.dbc", "iissssssssssssssssiiiiiiiiiiiiiiiii"},
    {"SpellVisual.dbc", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
    {"SpellVisualEffectName.dbc", "issffff"},
    {"SpellVisualKit.dbc", "iiiiiiiiiiiiiiiiiiiiiiffffffffffffffii"},
    {"SpellVisualKitAreaModel.dbc", "iii"},
    {"SpellVisualKitModelAttach.dbc", "iiiiiiiiii"},
    {"SpellVisualPrecastTransitions.dbc", "iss"},
    {"StableSlotPrices.dbc", "ii"},
    {"Startup_Strings.dbc", "isssssssssssssssssi"},
    {"Stationery.dbc", "iisi"},
    {"StringLookups.dbc", "is"},
    {"SummonProperties.dbc", "iiiiii"},
    {"Talent.dbc", "iiiiiiiiiiiiiiiiiiiiiii"},
    {"TalentTab.dbc", "issssssssssssssssiiiiiis"},
    {"TaxiNodes.dbc", "iifffssssssssssssssssiii"},
    {"TaxiPath.dbc", "iiii"},
    {"TaxiPathNode.dbc", "iiiifffiiii"},
    {"TeamContributionPoints.dbc", "if"},
    {"TerrainType.dbc", "isiiii"},
    {"TerrainTypeSounds.dbc", "i"},
    {"TotemCategory.dbc", "issssssssssssssssiii"},
    {"TransportAnimation.dbc", "iiifffi"},
    {"TransportPhysics.dbc", "iffffffffff"},
    {"TransportRotation.dbc", "iiiffff"},
    {"UISoundLookups.dbc", "iis"},
    {"UnitBlood.dbc", "iiiiissssi"},
    {"UnitBloodLevels.dbc", "iiii"},
    {"Vehicle.dbc", "iiffffiiiiiiiifffffffffffffffiiiififiiii"},
    {"VehicleSeat.dbc", "iiiffffffffffiiiiiiiffffffiiifffiiiiiiiffiiiiiiiiiiiiiiiii"},
    {"VehicleUIIndicator.dbc", "ii"},
    {"VehicleUIIndSeat.dbc", "iiiff"},
    {"VideoHardware.dbc", "iiiiiiiiiiiiiiiiiissiii"},
    {"VocalUISounds.dbc", "iiiiiii"},
    {"WeaponImpactSounds.dbc", "iiiiiiiiiiiiiiiiiiiiiii"},
    {"WeaponSwingSounds2.dbc", "iiii"},
    {"Weather.dbc", "iiiffffs"},
    {"WMOAreaTable.dbc", "iiiiiiiiiiissssssssssssssssi"},
    {"WorldChunkSounds.dbc", "iiiiiiiii"},
    {"WorldMapArea.dbc", "iiisffffiii"},
    {"WorldMapContinent.dbc", "iiiiiifffffffi"},
    {"WorldMapOverlay.dbc", "iiiiiiiisiiiiiiii"},
    {"WorldMapTransforms.dbc", "iiffffiffi"},
    {"WorldSafeLocs.dbc", "iifffssssssssssssssssi"},
    {"WorldStateUI.dbc", "iiiisssssssssssssssssissssssssssssssssiiiissssssssssssssssiiiii"},
    {"WorldStateZoneSounds.dbc", "iiiiiiii"},
    {"WowError_Strings.dbc", "isssssssssssssssssi"},
    {"ZoneIntroMusicTable.dbc", "isiii"},
    {"ZoneMusic.dbc", "isiiiiii"}
};

#endif
