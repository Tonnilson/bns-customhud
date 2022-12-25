# Blade & Soul CustomHUD
A sub-system / successor of FPSBooster for Blade & Soul [Unreal Engine 4]
CustomHUD allows users to remove various things from their HUD / UI as well as hiding name plates above player(s) and NPC(s) heads.

It also (most of the time) removes the black faded border around the game that I am just calling the Depth of field filter. Most people won't notice this but if you have the trained eye then you know what I am talking about and once you noticee it you can't unsee it.

I am just pushing this to keep track of my changes and stuff and if I ever abandon it before actually releasing it someone else can look over it and do stuff to it.

## In-game Keybinds
###### Alt + X
An alternative to Ctrl + X but hides only select UI elements, unlike UE3 there isn't a significant FPS boost but there is some. There is a known issue with the game crashing when using it, unlike FPS Booster the cause is unknown and it's an issue with the games code.
###### Alt + B
Hides or shows name plates above players and NPC's heads, no known issues with this.
###### Alt + Insert
Loads or reloads custom.txt located in `Documents\BnS\customhud.txt`
It's not recommended to use this

## customhud.txt
`customhud.txt is optional and not required or recommended but it is an option`

This is a way for users to customize which UI elements are hidden from the screen.

###### Default list of panel names hidden
```
PlayerExpBarPanel
PlayerStatusPanel
ChattingPanel
QuestQuickSlotPanel
SystemMenuPanel
NotificationMenuPanel
ItemBarPanel
SprintPanel
 ```
 
###### List of known Panel Names / Layer indexes
List of panels to hide, some may not hide like Talisman icon but overall the majority should work. Depending on what you use when unhiding unwanted stuff may re-appear.
```
Context_Icon_1
Context_Icon_2
Context_Icon_3
Context_Icon_4
Context_Icon_5
Context_Classic_Icon_1
Context_Classic_Icon_2
Context_Classic_Icon_3
Context_Classic_Icon_4
Context_Classic_Icon_5
PlayerSummonedPanel
SkillBar_Line_1
SkillBar_Line_2
Cmd_Down_Icon
Cmd_Left_Icon
Cmd_Right_Icon
SkillBarStanceHolder
SkillRecommendHolder
PlayerCastingGauseHolder
PlayerBuffEffectHolder
PlayerDebuffEffectHolder
PlayerBuffDisableHolder
BuffGraphHolder
PlayerStatusPanel
PlayerStatusHolder
PlayerUltimateGaugeHolder
Player_GuardGaugePanel
PlayerStatus_PvPHolder
PlayerSummonedBuffEffectHolder
PlayerSummonedDebuffEffectHolder
ItemBarPanel
PlayerExpBarPanel
SystemMenuPanel
NotificationMenuPanel
PartyBasicFrame
InstantNotificationPanel
ChattingPanel
ChatInputPanel
FramePerSecond
Minimap_Holder
QuestQuickSlotPanel
Tooltip_LinkDummy
BnsModeContextHolder
ImeLanguageStatePanel
BossCombatStatusPanel
ImmuneBreakPanel
FactionPopulationPanel
AchievementCompleteNotificationPanel2
AchievementCompleteNotificationPanel
MegaphonePanel
ShowPanel
DuelPlayerSummonedPanel
DuelPlayerSummonedBuffEffectHolder
DuelPlayerSummonedDebuffEffectHolder
SkillScorePanel
ClockPanel
RaidMemberPanel
RaidSquadPanel
GuildBattleFieldHistoryPanel
SkillResistHolder
SkillResistDescriptionHolder
PlayerSkillRecycleHolder
TeamPanel
Target2_StatusPanel
Target2_BuffEffectHolder
Target2_DebuffEffectHolder
Boss2_1_StatusPanel
Boss2_1_BuffEffectHolder
Boss2_1_DebuffEffectHolder
Boss2_2_StatusPanel
Boss2_2_BuffEffectHolder
Boss2_2_DebuffEffectHolder
BossListPanel
DuelTagMatchSkillBar
PlayerSystemEffectHolder
PlayerLongTermHolder
HeadlineFxPanel
LatencyInfoPanel
TendencyFieldPanel
SpectateCameraPanel
BladeMaster_BuffIndicatorHolder
KungfuFighter_TargetIndicatorHolder
ForceMaster_Indicator2Holder
Shooter_IndicatorHolder
Destroyer_IndicatorHolder
Summon_IndicatorHolder
Assassin_TargetIndicatorHolder
SwordMaster_BuffIndicatorHolder
Warlock_BuffIndicatorHolder
SoulFighter_BuffIndicatorHolder
Warrior_BuffIndicatorHolder
Archer_IndicatorHolder
Thunderer_BuffIndicatorHolder
DuelBotCardSkillBar
SkillBar_Line_Extra
novaSkillBar
HeadlineFishingFxPanel
Kungfufighter_ComboTitle
Feedback_CombatSignal_Panel
GimmickOptionPanel
EnergyPanel
PlayerStatus_PssNotificationHolder
PlayerStatus_PssHolder
ForceMasterG3_JobStyleGaugePanel
SwordMasterG3_JobStyleGaugPanel
```

# FAQ
### Does it provide an FPS bonus like fpsbooster for UE3?
Yes but actually no, some people may see a decent increase while others will see little to none. UE4 uses DirectX 11 while UE3 used DirectX 9 which had significant problems, the issues with DX9 are not as concerning with DX11 so the performance gain is very little.

### Does this work on all regions?
Confirmed working on NA/EU. Other regions like TW are untested but should in theory work

### My game crashes a lot when using Alt + X too many times
Known problem but unlike fpsbooster this issue comes down to the games code. I am using the games own custom functions for hiding and showing parts of the UI and the cause of the crash is unknown, somewhere in their code. The UI Panels were not meant to be hidden and have their own functions / threads monitoring them so this is probably why.
