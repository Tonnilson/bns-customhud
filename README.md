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
Loads or reloads booster.txt located in `Documents\BnS\customhud.txt`
It's not recommended to use this

## customhud.txt
`customhud.txt is optional and not required or recommended but it is an option`

This is a way for users to customize which UI elements are hidden from the screen.
###### List of known Panel Names / Layer indexes
I can't stress this enough but don't use layer indexes (integers) this are extremely unreliable due to how I determine them.
```
PlayerExpBarPanel
SprintPanel
PlayerStatusPanel
PlayerCombatStatusPanel
PlayerStancePanel
ChattingPanel
QuestQuickSlotPanel
ItemBarPanel
SkillBarPanel
ContextPanel
SystemMenuPanel
NotificationMenuPanel
Feedback_CombatSignal_Panel
EnergyUIPanel
EnergyPanel
# This method is not recommended as it's based off indexing which is sorted by layerId both of which change dynamically
# Layers start from 0-82 ( DO NOT USE low indexes, start from 20+)
# These are just a relative position on the table list, again note that these change each time you load into a new level and may shift up or down by a few
15: Name
17: XP Bar
18: System Menu Icons
19: Notification Icons
22: Chat
24: Top Left Icons
26: Quest Bar
27: Map
28: Z|X|C|V
29: 1|2|3|4
30: LMB
31: RMB
32: Hotkey items
33: Talisman
34: Buff Gauge
35: Tab
36: SS
37: E
39: Auto Combat Icon
40: Ultimate Gauge
41: Ping
42: FPS
49: HP Bar
56 | 57: Depth
```

# FAQ
### Does it provide an FPS bonus like fpsbooster for UE3?
Yes but actually no, some people may see a decent increase while others will see little to none. UE4 uses DirectX 11 while UE3 used DirectX 9 which had significant problems, the issues with DX9 are not as concerning with DX11 so the performance gain is very little.

### Does this work on all regions?
Confirmed working on NA/EU. Other regions like TW are untested but should in theory work

### My game crashes a lot when using Alt + X too many times
Known problem but unlike fpsbooster this issue comes down to the games code. I am using the games own custom functions for hiding and showing parts of the UI and the cause of the crash is unknown, somewhere in their code. The UI Panels were not meant to be hidden and have their own functions / threads monitoring them so this is probably why.
