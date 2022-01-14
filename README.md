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
Some panels cannot be hidden by their name, the default list above is confirmed 100% working.

I can't stress this enough but don't use layer indexes (integers) this are extremely unreliable due to how I determine them and by that I mean I am collecting a list of layers, sorting by their layerId and going off that. LayerID's change when layers are added or removed so loading into a new level (map) may have additional layers created before-hand
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
#HMLVL Name
15
#XP Bar
17
#System Menu Icons
18
#Notification Icons
19
#Chat
22
#Premium buff icons (top left on default ui)
24
#Quest bar
26
#Map
27
#Context menu Z|X|C|V
28
#Context Menu 1|2|3|4
29
#LMB Context Icon
30
#RMB Context Icon
31
#Hotkey icons (5|6|7|8 on default keylist)
32
#Talisman icon
33
#Buff gauge
34
#Tab context icon
35
#Backstep context icon
36
#E Context icon
37
#Auto Combat icon
39
#Ultimate Gauge
40
#Ping
41
#Fps but why? you can turn this off yourself in settings
42
#HP / Chi Bar
49
```

# FAQ
### Does it provide an FPS bonus like fpsbooster for UE3?
Yes but actually no, some people may see a decent increase while others will see little to none. UE4 uses DirectX 11 while UE3 used DirectX 9 which had significant problems, the issues with DX9 are not as concerning with DX11 so the performance gain is very little.

### Does this work on all regions?
Confirmed working on NA/EU. Other regions like TW are untested but should in theory work

### My game crashes a lot when using Alt + X too many times
Known problem but unlike fpsbooster this issue comes down to the games code. I am using the games own custom functions for hiding and showing parts of the UI and the cause of the crash is unknown, somewhere in their code. The UI Panels were not meant to be hidden and have their own functions / threads monitoring them so this is probably why.
