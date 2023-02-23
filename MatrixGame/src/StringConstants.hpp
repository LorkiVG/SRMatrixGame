// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef STRING_CONSTANTS
#define STRING_CONSTANTS

#define FILE_CONFIGURATION                      L"robots.dat"
#define FILE_CONFIGURATION_LOCATION             L"cfg\\"

// textures

#define TEXTURE_PATH_POINTLIGHT                 L"Matrix\\Textures\\pointlight"

#define TEXTURE_PATH_PB                         L"Matrix\\Textures\\pb"
#define TEXTURE_PATH_SELBACK                    L"Matrix\\Textures\\selback"


#define TEXTURE_PATH_SPOT                       L"Matrix\\Textures\\LandSpots\\spot"
#define TEXTURE_PATH_HIT                        L"Matrix\\Textures\\LandSpots\\spot_hit"
#define TEXTURE_PATH_SELECTION                  L"Matrix\\Textures\\LandSpots\\spot_move_to"
#define TEXTURE_PATH_VORONKA                    L"Matrix\\Textures\\LandSpots\\varonka"
#define TEXTURE_PATH_TURRET_RADIUS              L"Mods\\PlanetaryBattles\\PBEngine\\Matrix\\Textures\\LandSpots\\TurretSpot"


//#define TEXTURE_PATH_INSHOREWAVE                L"Matrix\\Textures\\Water\\inshorewave"
//#define TEXTURE_PATH_WATER                      L"Matrix\\Textures\\Water\\1"
//#define TEXTURE_PATH_WATERREFLECTION            L"Matrix\\Textures\\Water\\Mirror"

#define TEXTURE_PATH_REFLECTION                 L"Matrix\\Textures\\reflection"

#define TEXTURE_PATH_DUST                       L"Matrix\\Textures\\Billboard\\Dust"

#define TEXTURE_PATH_BIGBOOM                    L"Matrix\\Textures\\Effects\\bigboom"


#define TEXTURE_PATH_GUN_FIRE                   L"Matrix\\Textures\\Effects\\gun_fire"
#define TEXTURE_PATH_GUN_BULLETS1               L"Matrix\\Textures\\Effects\\bullet1"
#define TEXTURE_PATH_GUN_BULLETS2               L"Matrix\\Textures\\Effects\\bullet2"
#define TEXTURE_PATH_SPLASH                     L"Matrix\\Textures\\Effects\\splash"

#define TEXTURE_PATH_KEELWATER                  L"Matrix\\Textures\\Billboard\\keelwater"
#define TEXTURE_PATH_WAVE                       L"Matrix\\Textures\\Billboard\\kryg00"
#define TEXTURE_PATH_ZAHVAT                     L"Matrix\\Textures\\Billboard\\zahspot"
#define TEXTURE_PATH_OBJSEL                     L"Matrix\\Textures\\Billboard\\selection"

#define TEXTURE_PATH_MOVETO                     L"Mods\\PlanetaryBattles\\PBEngine\\Matrix\\Textures\\Sprites\\way_point"
#define TEXTURE_PATH_GATHER_POINT               L"Mods\\PlanetaryBattles\\PBEngine\\Matrix\\Textures\\Sprites\\gathering_point"

//#define TEXTURE_FLYER_BODY                     L"Matrix\\Helicopter\\main.png"
//#define TEXTURE_FLYER_ENGINE                   L"Matrix\\Helicopter\\main.png"
//#define TEXTURE_FLYER_VINT                     L"Matrix\\Helicopter\\main.png"
//#define TEXTURE_FLYER_VINT_1                   L"Matrix\\Helicopter\\screw1.png"
//#define TEXTURE_FLYER_WEAPON_1                 L"Matrix\\Helicopter\\helicopter_w.png"

//#define TEXTURE_BUILDING_PLATFORM1             L"Matrix\\Building\\b1_platform.png"
//#define TEXTURE_BUILDING_PLATFORM2             L"Matrix\\Building\\b2_platform.png"
//#define TEXTURE_BUILDING_PLATFORM3             L"Matrix\\Building\\b3_platform.png"
//#define TEXTURE_BUILDING_PLATFORM4             L"Matrix\\Building\\b4_platform.png"
// objects

//#define OBJECT_PATH_FLYER_BODY                 L"Matrix\\Helicopter\\main.vo"
//#define OBJECT_PATH_FLYER_ENGINE               L"Matrix\\Helicopter\\engine.vo"
//#define OBJECT_PATH_FLYER_VINT                 L"Matrix\\Helicopter\\screw.vo"
//#define OBJECT_PATH_FLYER_WEAPON_1             L"Matrix\\Helicopter\\weapon1.VO"
//#define OBJECT_PATH_FLYER_WEAPON_2             L"Matrix\\Helicopter\\weapon2.VO"
//#define OBJECT_PATH_FLYER_WEAPON_3             L"Matrix\\Helicopter\\weapon3.VO"

#define OBJECT_PATH_BUILDINGS_RUINS              L"Matrix\\Building\\ruins\\"

#define OBJECT_PATH_BASE_00                      L"Matrix\\Building\\b0.cvo"
#define OBJECT_PATH_BASE_01                      L"Matrix\\Building\\b1.cvo"
#define OBJECT_PATH_BASE_02                      L"Matrix\\Building\\b2.cvo"
#define OBJECT_PATH_BASE_03                      L"Matrix\\Building\\b3.cvo"
#define OBJECT_PATH_BASE_04                      L"Matrix\\Building\\b4.cvo"
#define OBJECT_PATH_BASE_05                      L"Matrix\\Building\\b5.cvo"

#define CONFIG_PATH_WEAPONS_DATA				 L"WeaponsConfig"

#define MODULES_CONFIG_PATH_HULLS			 	 L"RobotsConfig.Modules.Hulls"
#define MODULES_CONFIG_PATH_CHASSIS				 L"RobotsConfig.Modules.Chassis"
#define MODULES_CONFIG_PATH_HEADS			 	 L"RobotsConfig.Modules.Heads"
#define MODULES_CONFIG_PATH_WEAPONS				 L"RobotsConfig.Modules.Weapons"

#define CONFIG_PATH_TURRETS						 L"TurretsConfig"
#define CONFIG_PATH_WEAPON_EFFECTS				 L"WeaponEffectsConfig"

#define OBJECT_PATH_LOGO_RED                     L"Matrix\\Logos\\RobotLogosR.VO"
#define OBJECT_PATH_LOGO_GREEN                   L"Matrix\\Logos\\RobotLogosG.VO"
#define OBJECT_PATH_LOGO_BLUE                    L"Matrix\\Logos\\RobotLogosB.VO"

#define OBJECT_PATH_MOVETO                       L"Matrix\\MoveTo\\moveto.VO"

// matrices
#define MATRIX_BASE_PLACE                        L"Place"

// animations
#define ANIMATION_NAME_BEGIN_MOVE_BACK           L"BeginMoveBack"
#define ANIMATION_NAME_END_MOVE_BACK             L"EndMoveBack"
#define ANIMATION_NAME_MOVE_BACK                 L"MoveBack"

#define ANIMATION_NAME_BEGIN_MOVE                L"BeginMove"
#define ANIMATION_NAME_END_MOVE                  L"EndMove"
#define ANIMATION_NAME_MOVE                      L"Move"
#define ANIMATION_NAME_STAY                      L"Stay"
#define ANIMATION_NAME_ROTATE                    L"Rotate"

#define ANIMATION_NAME_IDLE                      L"Idle"
#define ANIMATION_NAME_FIRE                      L"Fire"
#define ANIMATION_NAME_FIRELOOP                  L"FireLoop"

#define TEMPLATE_PAUSE                           L"Pause"
#define TEMPLATE_DIALOG_BEGIN                    L"Begin"
#define TEMPLATE_DIALOG_EXIT                     L"Exit"
#define TEMPLATE_DIALOG_LOSE                     L"Loose"
#define TEMPLATE_DIALOG_WIN                      L"Win"
#define TEMPLATE_DIALOG_MENU                     L"Menu"
#define TEMPLATE_DIALOG_RESET                    L"Reset"
#define TEMPLATE_DIALOG_SURRENDER                L"Surrender"
#define TEMPLATE_DIALOG_STATISTICS               L"Stat"
#define TEMPLATE_DIALOG_STATISTICS_D             L"StatD"

// params
//#define BURN_PARAM_VO                          L"BurnableVO"
//#define BURN_PARAM_TEX                         L"BurnableTex"
//#define BREAK_PARAM_VO                         L"BreakableVO"
#define BREAK_PARAM_HITPOINT                     L"Hitpoint"

#define CFG_TOP_SIZE                             L"TextureTopMinSize"
#define CFG_TOP_SCALE                            L"TextureTopDownScaleFactor"
#define CFG_BOT_SIZE                             L"TextureBottomMinSize"
#define CFG_BOT_SCALE                            L"TextureBottomDownScaleFactor"
#define CFG_LAND_TEXTURES_16                     L"LandTextures16Bit"
#define CFG_SOFTWARE_CURSOR                      L"SoftwareCursor"
#define CFG_GLOSS_LAND                           L"LandTexturesUseGloss"
#define CFG_GLOSS_OBJECT                         L"ObjectTexturesUseGloss"
#define CFG_OBJECT_TEX_16                        L"ObjectTextures16Bit"
#define CFG_DEBUG_INFO                           L"DebugInfo"
#define CFG_VERTEX_LIGHT                         L"VertexLight"
#define CFG_ASSIGN_KEYS                          L"AssignKeys"
#define CFG_IZVRAT_MS                            L"IzvratMultiSelection"
#define CFG_ROBOT_SHADOW                         L"RobotShadow"
#define CFG_SKY_BOX                              L"SkyBox"
#define CFG_OBJECTTOMINIMAP                      L"ObjectsToMinimap"
#define CFG_GAMMA_RAMP                           L"GammaRamp"
#define CFG_MAX_FPS                              L"MaxFps"
#define CFG_MAX_VIEW_DISTANCE			 		 L"MaxViewDistance"
#define CFG_OBJECTS_PER_SCREEN			 		 L"MaxObjectsPerScreen"
#define CFG_EFFECTS_COUNT				 		 L"MaxEffectsCount"
#define CFG_SHADOWS_DRAW_DISTANCE		 		 L"ShadowsDrawDistance"
#define CFG_THIN_FOG_DRAW_DISTANCE		 		 L"ThinFogDrawDistance"
#define CFG_DENSE_FOG_DRAW_DISTANCE		 		 L"DenseFogDrawDistance"
#define CFG_DESIGNS_TO_SAVE						 L"MaxDesignsToSave"
#define CFG_PLAYER_ROBOTS_AUTO_BOOM				 L"PlayerRobotsAutoBoom"
#define CFG_ENABLE_FLYERS						 L"EnableFlyers"
#define CFG_LOADING_SCREEN_ANIM					 L"LoadingScreenAnim"

#define BLOCK_PATH_MAIN_CONFIG                   L"MainConfig"
#define BLOCK_PATH_FLYERS_CONFIG			     L"HelicoptersConfig"
#define PAR_TEMPLATES                            L"Templates"
#define PAR_REPLACE                              L"Replaces"
#define PAR_REPLACE_DIFFICULTY                   L"_difficulty"
#define PAR_REPLACE_BEGIN_ICON_RACE              L"_race"
#define PAR_REPLACE_BEGIN_TEXT                   L"_begin"
#define PAR_REPLACE_END_TEXT_LOOSE               L"_loose"
#define PAR_REPLACE_END_TEXT_WIN                 L"_win"
#define PAR_REPLACE_END_TEXT_PLANET              L"_planet"

//#define PAR_SOURCE_TEXTURES                    L"Ter"
//#define PAR_SOURCE_BORDERS                     L"TerBorder"
#define PAR_SOURCE_IMAGES                        L"Images"
#define PAR_SOURCE_DIFFICULTY                    L"GameDifficulty"
#define PAR_SOURCE_CURSORS                       L"Cursors"
#define PAR_SOURCE_DAMAGES                       L"Damages"
#define PAR_SOURCE_DAMAGES_OBJECT                L"Object"
#define PAR_SOURCE_DAMAGES_CANNON                L"Cannon"
#define PAR_SOURCE_DAMAGES_BUILDING              L"Building"
#define PAR_SOURCE_DAMAGES_ROBOT                 L"Robot"
#define PAR_SOURCE_DAMAGES_FLYER                 L"Flyer"

#define PAR_SOURCE_RADAR_ROBOT_RADIUS            L"RobotRadius"
#define PAR_SOURCE_RADAR_FLYER_RADIUS            L"FlyerRadius"

#define BLOCK_PATH_REINFORCEMENTS				 L"RobotsConfig.ReinforcementsCall"

#define PAR_SOURCE_TIMINGS                       L"Timings"
#define PAR_SOURCE_TIMINGS_RESOURCES             L"Resources"

#define PAR_SOURCE_TIMINGS_TITAN                 L"RESOURCE_TITAN"
#define PAR_SOURCE_TIMINGS_ELECTRONICS           L"RESOURCE_ELECTRONICS"
#define PAR_SOURCE_TIMINGS_ENERGY                L"RESOURCE_ENERGY"
#define PAR_SOURCE_TIMINGS_PLASMA                L"RESOURCE_PLASMA"

#define PAR_SOURCE_TIMINGS_BASE                  L"RESOURCE_BASE"
#define PAR_SOURCE_TIMINGS_UNITS                 L"Units"
#define PAR_SOURCE_TIMINGS_ROBOT                 L"UNIT_ROBOT"
#define PAR_SOURCE_TIMINGS_FLYER                 L"UNIT_FLYER"
#define PAR_SOURCE_TIMINGS_TURRET                L"UNIT_TURRET"

#define PAR_SOURCE_TIMINGS_CAPTURE               L"Capture"
#define PAR_SOURCE_TIMINGS_ERASE                 L"ERASE"
#define PAR_SOURCE_TIMINGS_PAINT                 L"PAINT"
#define PAR_SOURCE_TIMINGS_ROLLBACK              L"ROLLBACK"

#define PAR_SOURCE_WATER                                 L"Water"
#define PAR_SOURCE_WATER_WATER                           L"water"
#define PAR_SOURCE_WATER_MIRROR                          L"mirror"
#define PAR_SOURCE_WATER_INSHORE                         L"inshore"

#define PAR_SOURCE_WEAPONS                               L"Weapons"

#define PAR_SOURCE_MINIMAP                               L"Minimap"
#define PAR_SOURCE_MINIMAP_POINT                         L"point"
#define PAR_SOURCE_MINIMAP_ARROW                         L"arrow" 
#define PAR_SOURCE_MINIMAP_FLYER                         L"flyer"
#define PAR_SOURCE_MINIMAP_ROBOT                         L"robot"
#define PAR_SOURCE_MINIMAP_FACTORY                       L"factory"
#define PAR_SOURCE_MINIMAP_BASE                          L"base"
#define PAR_SOURCE_MINIMAP_TURRET                        L"turret"
#define PAR_SOURCE_MINIMAP_FLYER_R                       L"flyer_r"
#define PAR_SOURCE_MINIMAP_ROBOT_R                       L"robot_r"
#define PAR_SOURCE_MINIMAP_FACTORY_R                     L"factory_r"
#define PAR_SOURCE_MINIMAP_BASE_R                        L"base_r"
#define PAR_SOURCE_MINIMAP_TURRET_R                      L"turret_r"


#define PAR_SOURCE_HINTS                                 L"Hints"
#define PAR_SOURCE_HINTS_BITMAPS                         L"Bitmaps"
#define PAR_SOURCE_HINTS_SOURCE                          L"Source"
#define PAR_SOURCE_HINTS_LT                              L"LeftTop"
#define PAR_SOURCE_HINTS_T                               L"Top"
#define PAR_SOURCE_HINTS_RT                              L"RightTop"
#define PAR_SOURCE_HINTS_L                               L"Left"
#define PAR_SOURCE_HINTS_C                               L"Center"
#define PAR_SOURCE_HINTS_R                               L"Right"
#define PAR_SOURCE_HINTS_LB                              L"LeftBottom"
#define PAR_SOURCE_HINTS_B                               L"Bottom"
#define PAR_SOURCE_HINTS_RB                              L"RightBottom"

#define PAR_SOURCE_CAMERA                       L"Camera"
#define PAR_SOURCE_CAMERA_STRATEGY              L"Strategy"
#define PAR_SOURCE_CAMERA_ARCADE                L"Arcade"

#define PAR_SOURCE_CAMERA_WHEELSTEP             L"CamMouseWheelStep"
#define PAR_SOURCE_CAMERA_ROTSPEEDX             L"CamRotSpeedX"
#define PAR_SOURCE_CAMERA_ROTSPEEDZ             L"CamRotSpeedZ"
#define PAR_SOURCE_CAMERA_ROTANGLEMIN           L"CamRotAngleMin"
#define PAR_SOURCE_CAMERA_ROTANGLEMAX           L"CamRotAngleMax"
#define PAR_SOURCE_CAMERA_DISTMIN               L"CamDistMin"
#define PAR_SOURCE_CAMERA_DISTMAX               L"CamDistMax"
#define PAR_SOURCE_CAMERA_DISTPARAM             L"CamDistParam"
#define PAR_SOURCE_CAMERA_ANGLEPARAM            L"CamAngleParam"
#define PAR_SOURCE_CAMERA_HEIGHT                L"CamHeight"

#define PAR_SOURCE_CAMERA_BASEANGLEZ            L"CamBaseAngleZ"
#define PAR_SOURCE_CAMERA_MOVESPEED             L"CamMoveSpeed"
#define PAR_SOURCE_CAMERA_FOV					L"CamFieldOfView"
#define PAR_SOURCE_CAMERA_ARCADE_FWD0           L"CamInRobotForward0"
#define PAR_SOURCE_CAMERA_ARCADE_FWD1           L"CamInRobotForward1"

#define CURSOR_ARROW                            L"arrow"
#define CURSOR_CROSS_BLUE                       L"cross_blue"
#define CURSOR_CROSS_RED                        L"cross_red"
#define CURSOR_CROSS_YELLOW                     L"cross_yellow"
#define CURSOR_ARROW_MINI_MAP                   L"arrow_mini_map"
#define CURSOR_STAR                             L"star"

#define FOLDER_NAME_CACHE                       "Cache"
#define FOLDER_NAME_SCREENSHOTS                 "Screenshots"
#define FILE_NAME_SCREENSHOT                    "Shot"

// interface

//system
#define IF_LABELS_BLOCKPAR                      L"AllLabels"
#define IF_LABELS                               L"Labels"
#define IF_LABELS_TEXT                          L"LabelsText"
#define IF_STATE_STATIC_LABEL                   L"StateStaticLabel"
#define IF_STATE_DYNAMIC_LABEL                  L"StateDynamicLabel"
#define IF_STATIC_LABEL                         L"StaticLabel"
#define IF_LABEL_PARAMS                         L"Params"

#define IF_STATE_NORMAL                         L"sNormal"
#define IF_STATE_FOCUSED                        L"sFocused"
#define IF_STATE_PRESSED                        L"sPressed"
#define IF_STATE_DISABLED                       L"sDisabled"
#define IF_STATE_PRESSED_UNFOCUSED              L"sPressedUnfocused"
#define IF_PATH                                 L"cfg\\robots\\iface.txt"

#define IF_ANIMATION                            L"Animation"
#define IF_ANIMATION_TEX                        L"Texture"
#define IF_ANIMATION_TEX_WIDTH                  L"TexWidth"
#define IF_ANIMATION_TEX_HEIGHT                 L"TexHeight"
#define IF_ANIMATION_FRAMES                     L"Frames"

//Popupmenu

#define IF_POPUP_MENU                           L"PopupMenu"
//Main
#define IF_MAIN                                 L"Main"
#define IF_CALL_FROM_HELL                       L"callhell"
#define IF_LIVES_LABEL                          L"lives"
#define IF_NAME_LABEL                           L"name"
#define IF_RHEAD_LABEL                          L"rhead"
#define IF_RHULL_LABEL                          L"rhull"
#define IF_RCHAS_LABEL                          L"rchas"
#define IF_RWGHT_LABEL                          L"rwght"
#define IF_RSPEED_LABEL                         L"rspeed"

#define IF_DOTLINE                              L"dotln"
#define IF_OVER_HEAT                            L"ovhe"

#define IF_ORDER_GLOW                           L"osel"
#define IF_GROUP_GLOW                           L"gsel"
#define IF_GROUP_RAMKA                          L"gram"

#define IF_AUTO_SEL_1                           L"kadr1"
#define IF_AUTO_SEL_2                           L"kadr2"
#define IF_AUTO_SEL_3                           L"kadr3"
#define IF_AUTO_SEL_4                           L"kadr4"
#define IF_AUTO_SEL_5                           L"kadr5"
#define IF_AUTO_SEL_6                           L"kadr6"
#define IF_AUTO_SEL_7                           L"kadr7"
#define IF_AUTO_SEL_8                           L"kadr8"

#define IF_MANUAL_BG                            L"manbg"
#define IF_EVIL_LAMP1                           L"ela1"
#define IF_EVIL_LAMP2                           L"ela2"
#define IF_EVIL_LAMP3                           L"ela3"

#define IF_MAIN_PANEL1                          L"mp1"
#define IF_MAIN_PANEL2                          L"mp2"

#define IF_MAIN_HP                              L"hp" //hit points

#define IF_ORDER_STOP                           L"ost"
#define IF_ORDER_MOVE                           L"ogo"
#define IF_ORDER_PATROL                         L"opa"
#define IF_ORDER_FIRE                           L"ofi"
#define IF_ORDER_CAPTURE                        L"oca"
#define IF_ORDER_CANCEL                         L"ocan"
#define IF_ORDER_REPAIR                         L"orep"
#define IF_ORDER_BOMB                           L"obomb"
#define IF_ORDER_BOMB_AUTO                      L"obomba"

#define IF_EXIT_ARCADE_MODE                     L"lero"
#define IF_ENTER_ARCADE_MODE                    L"inro"

//Режимы автоматического поведения для робота игрока
#define IF_AORDER_ATTACK_OFF                    L"oafrf"
#define IF_AORDER_ATTACK_ON                     L"oafrn"
#define IF_AORDER_CAPTURE_OFF                   L"oacapf"
#define IF_AORDER_CAPTURE_ON                    L"oacapn"
#define IF_AORDER_PROTECT_OFF                   L"oafcf"
#define IF_AORDER_PROTECT_ON                    L"oafcn"

#define IF_ZAGLUSHKA1                           L"zagl1"

#define IF_TITAN_PLANT                          L"titpl"
#define IF_PLASMA_PLANT                         L"plaspl"
#define IF_ELECTRO_PLANT                        L"elecpl"
#define IF_ENERGY_PLANT                         L"batpl"
#define IF_REPAIR_PLANT                         L"reppl"
#define IF_BASE_PLANT                           L"basepl"

#define IF_BASE_LINE                            L"baseln"

#define IF_BUILD_ROBOT                          L"buro"
#define IF_BUILD_FLYER                          L"buhe"
#define IF_BUILD_TURRET                         L"buca"
#define IF_BUILD_REPAIR                         L"bure"

#define IF_BUILD_TUR1                           L"tur1"
#define IF_BUILD_TUR2                           L"tur2"
#define IF_BUILD_TUR3                           L"tur3"
#define IF_BUILD_TUR4                           L"tur4"

#define IF_BUILD_FLYER_1                        L"hel1"
#define IF_BUILD_FLYER_2                        L"hel2"
#define IF_BUILD_FLYER_3                        L"hel3"
#define IF_BUILD_FLYER_4                        L"hel4"

#define IF_FLYER_1_IMG                          L"hel1_img"
#define IF_FLYER_2_IMG                          L"hel2_img"
#define IF_FLYER_3_IMG                          L"hel3_img"
#define IF_FLYER_4_IMG                          L"hel4_img"

#define IF_FLYER_1_ICON_BIG                     L"hel1_ico_b"
#define IF_FLYER_2_ICON_BIG                     L"hel2_ico_b"
#define IF_FLYER_3_ICON_BIG                     L"hel3_ico_b"
#define IF_FLYER_4_ICON_BIG                     L"hel4_ico_b"

#define IF_FLYER_1_ICON_SMALL                   L"hel1_ico_s"
#define IF_FLYER_2_ICON_SMALL                   L"hel2_ico_s"
#define IF_FLYER_3_ICON_SMALL                   L"hel3_ico_s"
#define IF_FLYER_4_ICON_SMALL                   L"hel4_ico_s"

#define IF_TURRETS_MAX                          L"turmax"
#define IF_TURRETS_HAVE                         L"turhav"

#define IF_QUEUE_ICON                           L"sticon"
#define IF_QUEUE_OTHER                          L"stother"

#define IF_TURRET_SMALL1                         L"tsm1"
#define IF_TURRET_SMALL2                         L"tsm2"
#define IF_TURRET_SMALL3                         L"tsm3"
#define IF_TURRET_SMALL4                         L"tsm4"

#define IF_TURRET_MED1                           L"tmd1"
#define IF_TURRET_MED2                           L"tmd2"
#define IF_TURRET_MED3                           L"tmd3"
#define IF_TURRET_MED4                           L"tmd4"

//Radar
#define IF_RADAR                                L"Radar"
#define IF_RADAR_PN                             L"rad"

//Mini-map
#define IF_MINI_MAP                             L"MiniM"
#define IF_MAP_ZOOM_IN                          L"zi"
#define IF_MAP_ZOOM_OUT                         L"zo"
#define IF_MAP_PANEL                            L"mmp"
#define IF_SHOWROBOTS_PANEL                     L"srpan"
#define IF_SHOWROBOTS_BUTT                      L"srb"

//Top
#define IF_TOP                                  L"Top"
#define IF_TOP_PANEL1                           L"tp1"
#define IF_MAIN_MENU_BUTTON                     L"mm"
#define IF_TITAN_LABEL                          L"tit"
#define IF_ELECTRO_LABEL                        L"elect"
#define IF_ENERGY_LABEL                         L"energ"
#define IF_PLASMA_LABEL                         L"plasm"
#define IF_RVALUE_LABEL                         L"rval"

//Base
#define IF_BASE                                 L"Base"

#define IF_BASE_CONSTRUCTION_LEFT               L"conl"
#define IF_BASE_CONSTRUCTION_RIGHT              L"conr"
#define IF_BASE_CONSTRUCTION_FOOT               L"conf"

#define IF_BASE_CONST_CANCEL                    L"cocan"
#define IF_BASE_CONST_BUILD                     L"cobuild"

#define IF_BASE_HEAD_PYLON_EMPTY                L"heade"
#define IF_BASE_WEAPON_PYLON_EMPTY              L"weape"

#define IF_BASE_WEAPON_PYLON_1                  L"pi1"
#define IF_BASE_WEAPON_PYLON_2                  L"pi2"
#define IF_BASE_WEAPON_PYLON_3                  L"pi3"
#define IF_BASE_WEAPON_PYLON_4                  L"pi4"
#define IF_BASE_WEAPON_PYLON_5                  L"pi5"

#define IF_BASE_HEAD_PYLON                      L"pihe"
#define IF_BASE_HULL_PYLON                      L"pihu"
#define IF_BASE_CHASSIS_PYLON                   L"pich"

#define IF_BASE_TITAN_IMAGE                     L"titan"
#define IF_BASE_ELECTRONICS_IMAGE               L"electr"
#define IF_BASE_ENERGY_IMAGE                    L"energy"
#define IF_BASE_PLASMA_IMAGE                    L"plasma"

#define IF_BASE_TITAN_SUMM                      L"titans"
#define IF_BASE_ELECTRONICS_SUMM                L"electrs"
#define IF_BASE_ENERGY_SUMM                     L"energys"
#define IF_BASE_PLASMA_SUMM                     L"plasmas"

#define IF_BASE_SUMM_PANEL                      L"res_summ"
#define IF_BASE_UNIT_PANEL                      L"res_unit"

#define IF_BASE_WARNING1                        L"warn1"

#define IF_BASE_WEIGHT                          L"weight"
#define IF_BASE_SPEED                           L"speed"
#define IF_BASE_STRUCTURE                       L"struct"
#define IF_BASE_DAMAGE                          L"damage"

#define IF_BASE_ITEM_PRICE                      L"itprice"

#define IF_BASE_ITEM_TEXT1                      L"it_text1"
#define IF_BASE_ITEM_TEXT2                      L"it_text2"
#define IF_BASE_ITEM_LABEL1                     L"it_label1"
#define IF_BASE_ITEM_LABEL2                     L"it_label2"

#define IF_BASE_IW_TEXT                         L"iwtext"

#define IF_BASE_IHU_TEXT						L"ihutext"
#define IF_BASE_ICH_TEXT                        L"ichtext"
#define IF_BASE_IHE_TEXT                        L"ihetext"

#define IF_BASE_WARNING                         L"warn"
#define IF_BASE_WARNING1                        L"warn1"
#define IF_BASE_WARNING_LABEL                   L"warnl"
#define IF_BASE_ZERO                            L"zero"
#define IF_BASE_ONE                             L"one"
#define IF_BASE_TWO                             L"two"
#define IF_BASE_THREE                           L"three"
#define IF_BASE_FOUR                            L"four"
#define IF_BASE_FIVE                            L"five"
#define IF_BASE_SIX                             L"six"
#define IF_BASE_HISTORY_RIGHT                   L"hisright"
#define IF_BASE_HISTORY_LEFT                    L"hisleft"
#define IF_BASE_COUNTHZ                         L"counthz"
#define IF_BASE_UP                              L"bup"
#define IF_BASE_DOWN                            L"bdown"
#define IF_BASE_ITEM_CHARS                      L"itch"


//Popup menu
#define IF_POPUP_TOPRIGHT                       L"topright"
#define IF_POPUP_TOPLEFT                        L"topleft"
#define IF_POPUP_BOTTOMRIGHT                    L"bottomright"
#define IF_POPUP_BOTTOMLEFT                     L"bottomleft"
#define IF_POPUP_TOPLINE                        L"topline"
#define IF_POPUP_BOTTOMLINE                     L"bottomline"
#define IF_POPUP_RIGHTLINE                      L"rightline"
#define IF_POPUP_LEFTLINE                       L"leftline"
#define IF_POPUP_SELRIGHT                       L"selright"
#define IF_POPUP_SEL                            L"sel"
#define IF_POPUP_SELMOUSE                       L"selmouse"
#define IF_POPUP_SELRIGHTMOUSE                  L"selrightmouse"
#define IF_POPUP_RAMKA                          L"rmka"
#define IF_POPUP_POINTER                        L"pointer"
#define IF_POPUP_POINTER_NOT_AVAILABLE          L"pointer_not"
#define IF_POPUP_SELECTOR                       L"slct"
#define IF_POPUP_POINTER_OBJECT                 L"cursor"


//Hint fake interface
#define IF_HINTS                                L"Hints"
#define IF_HINTS_OK                             L"hint_ok"
#define IF_HINTS_CANCEL                         L"hint_cancel"
#define IF_HINTS_CANCEL_MENU                    L"hint_cmenu"
#define IF_HINTS_CONTINUE                       L"hint_cont"
#define IF_HINTS_SURRENDER                      L"hint_surr"
#define IF_HINTS_EXIT                           L"hint_exit"
#define IF_HINTS_RESET                          L"hint_reset"
#define IF_HINTS_HELP                           L"hint_help"


#define IF_BT1_ICON                             L"bt1"
#define IF_BT2_ICON                             L"bt2"
#define IF_BT3_ICON                             L"bt3"
#define IF_BT4_ICON                             L"bt4"

#define IF_PODL1                                L"podl1"
#define IF_PODL2                                L"podl2"
#define IF_PODL3                                L"podl3"
#define IF_PODL4                                L"podl4"

#define IF_MB_RES                               L"mbres"
#define IF_TF_RES                               L"tfres"
#define IF_ELF_RES                              L"elfres"
#define IF_ENF_RES                              L"enfres"
#define IF_PF_RES                               L"pfres"

#define IF_BUILDING_OPIS                        L"bopis"
#define IF_BASE_RES_INC                         L"bresg"
#define IF_FACTORY_RES_INC                      L"fresg"

#define IF_MAIN_WEAPONSLOTS                     L"wsl"
#define IF_MAIN_SELF_BOMB                       L"sbo"
#define IF_MAIN_SELF_BOMB_AUTO                   L"sabo"

#define IF_MAIN_PROG                            L"prog"
#define IF_BASE_RCNAME                          L"rcname"

#endif
