/***************************************************************************
 * Dystopia 2 © 2000, 2001, 2002, 2003, 2004 & 2005 by Brian Graversen     *
 *                                                                         *
 * In order to use any part of this code, you must comply with the license *
 * files found in the license folder in the doc folder of this codebase.   *
 *                                                                         *
 * Dystopia MUD is based of Godwars by Richard Woolcock.                   *
 *                                                                         *
 * Godwars is based of Merc by Michael Chastain, Michael Quan and Mitchell *
 * Tse.                                                                    *
 *                                                                         *
 * Merc is based of DIKU by Sebastian Hammer, Michael Seifert, Hans Henrik *
 * Staerfeldt. Tom Madsen and Katja Nyboe.                                 *
 *                                                                         *
 * Any additional licenses, copyrights, etc that affects this sourcefile   *
 * will be mentioned just below this copyright notice.                     *
 ***************************************************************************/

/* all events should have their own unique number */
#define EVENT_NONE                       0
#define EVENT_GAME_WEATHER               1
#define EVENT_GAME_CRASHSAFE             2
#define EVENT_GAME_RAGNAROK              3
#define EVENT_GAME_RAGNAROK_RESET        4
#define EVENT_GAME_ARENA                 5
#define EVENT_GAME_ARENA_CLOSE           6
#define EVENT_GAME_PULSE30               7
#define EVENT_GAME_AREASAVE              8
#define EVENT_GAME_KINGDOMQUEST          9
#define EVENT_GAME_ARTICHECK            10
#define EVENT_GAME_ARENA_WARNING        11
#define EVENT_PLAYER_SAVE              101
#define EVENT_PLAYER_HEAL              102
#define EVENT_PLAYER_ARENA_WAIT        103
#define EVENT_PLAYER_STUDY             104
#define EVENT_PLAYER_DEATHFRENZY       105
#define EVENT_PLAYER_FAE_MATTER        106
#define EVENT_PLAYER_FAE_PLASMA        107
#define EVENT_PLAYER_FAE_ENERGY        108
#define EVENT_PLAYER_FAE_WILL          109
#define EVENT_PLAYER_SPIDERS           110
#define EVENT_PLAYER_GEYSER_WAIT       111
#define EVENT_PLAYER_PHANTOM           112
#define EVENT_PLAYER_FAE_SIGIL         113
#define EVENT_PLAYER_FAE_TIMEWARP      114
#define EVENT_PLAYER_SHADOWPORTAL_WAIT 115
#define EVENT_PLAYER_SHADOWPORTAL      116
#define EVENT_PLAYER_CHILLBOLT         117
#define EVENT_PLAYER_PVIPER_RED        118
#define EVENT_PLAYER_PVIPER_GREEN      119
#define EVENT_PLAYER_PVIPER_BLUE       120
#define EVENT_PLAYER_PVIPER_YELLOW     121
#define EVENT_PLAYER_PVIPER_PURPLE     122
#define EVENT_PLAYER_CROWS             123
#define EVENT_PLAYER_LEECHES           124
#define EVENT_PLAYER_TARUK             125
#define EVENT_PLAYER_DISPLACE          126
#define EVENT_PLAYER_SCRYBIRDS         127
#define EVENT_PLAYER_CATALYST          128
#define EVENT_PLAYER_ARCHANGEL         129
#define EVENT_PLAYER_MUMMYWAIT         130
#define EVENT_PLAYER_KALARM            131
#define EVENT_PLAYER_WATERSTREAM       132
#define EVENT_PLAYER_HEATMETAL         133
#define EVENT_PLAYER_GUSTWIND_WAIT     134
#define EVENT_PLAYER_FAEPIPES          135
#define EVENT_PLAYER_MINDBLANK         136
#define EVENT_PLAYER_MINDBOOST         137
#define EVENT_PLAYER_TIMETRIP          138
#define EVENT_PLAYER_TIMETRIP_WAIT     139
#define EVENT_PLAYER_GUARDDOG          140
#define EVENT_PLAYER_LEGEND            141
#define EVENT_PLAYER_DRAGONORB         142
#define EVENT_PLAYER_SLOWSPELL         143
#define EVENT_PLAYER_HASTESPELL        144
#define EVENT_PLAYER_CHOPATTACK        145
#define EVENT_PLAYER_CHOPATTACK_WAIT   146
#define EVENT_PLAYER_DOOMBOLT          147
#define EVENT_PLAYER_SACRIFICE_FAE     148
#define EVENT_PLAYER_SACRIFICE_WAIT    149
#define EVENT_PLAYER_ARENA_MADNESS     150
#define EVENT_PLAYER_ESCAPE            151
#define EVENT_PLAYER_CONFUSE_WAIT      152
#define EVENT_PLAYER_EARTHRING         153
#define EVENT_PLAYER_HUNTINGSTARS      154
#define EVENT_PLAYER_BLOODTHEFT        155
#define EVENT_PLAYER_RAZORHANDS        156
#define EVENT_PLAYER_FAE_HAUNTING      157
#define EVENT_PLAYER_CONTEST           158
#define EVENT_PLAYER_WHIRLWIND         159
#define EVENT_PLAYER_MESSAGE           160
#define EVENT_PLAYER_PERMANENCY        161
#define EVENT_PLAYER_EARTHFLUX         162
#define EVENT_PLAYER_WATERFLUX         163
#define EVENT_PLAYER_FAECHAIN          164
#define EVENT_PLAYER_BLOODIMMUNE       165
#define EVENT_PLAYER_MIRRORIMAGE       166
#define EVENT_PLAYER_WITNESS           167
#define EVENT_PLAYER_WITNESSGRAB       168
#define EVENT_PLAYER_AFK               169
#define EVENT_PLAYER_ALIGNMENT         170
#define EVENT_PLAYER_PHANTOM_WAIT      171
#define EVENT_PLAYER_BLOODTASTE        172
#define EVENT_MOBILE_HEAL              501
#define EVENT_MOBILE_MOVE              502
#define EVENT_MOBILE_SPEC              503
#define EVENT_MOBILE_SCAVENGE          504
#define EVENT_CHAR_UPDATE              505
#define EVENT_MOBILE_MUMMYROT          506
#define EVENT_MOBILE_CASTING           507
#define EVENT_MOBILE_PIPEMOVE          508
#define EVENT_MOBILE_FLEE              509
#define EVENT_MOBILE_CONFUSED          510
#define EVENT_MOBILE_SHADOWGRABBED     511
#define EVENT_MOBILE_SUNSET            512
#define EVENT_MOBILE_FIGHTING          513
#define EVENT_MOBILE_CANTRIP_HEAL      514
#define EVENT_MOBILE_EXTRACT           515
#define EVENT_MOBILE_ACIDTENDRILS      516
#define EVENT_MOBILE_BLURTENDRILS      517
#define EVENT_MOBILE_PSPRAY            518
#define EVENT_MOBILE_CHAINBLAST        519
#define EVENT_MOBILE_ACIDBLOOD         520
#define EVENT_MOBILE_SNOTLINGSLEEP     521
#define EVENT_MOBILE_ILLTHID           522
#define EVENT_MOBILE_KGUARD            523
#define EVENT_MOBILE_CANTRIP_SPIKES    524
#define EVENT_MOBILE_RUPTURE           525
#define EVENT_MOBILE_STANCING          526
#define EVENT_SOCKET_IDLE              701
#define EVENT_SOCKET_NEGOTIATE         702
#define EVENT_OBJECT_DECAY             801
#define EVENT_OBJECT_AFFECTS           802
#define EVENT_OBJECT_IMPLODE           803
#define EVENT_OBJECT_PLAGUE            804
#define EVENT_OBJECT_CAULDRON          805
#define EVENT_OBJECT_DOOMCHARGE        806
#define EVENT_ARTIFACT_UPDATE          901
#define EVENT_ARTIFACT_GENERIC         902
#define EVENT_ARTIFACT_DRAGONORB       903
#define EVENT_ARTIFACT_SINGINGSWORD    904
#define EVENT_ARTIFACT_EARTHRING       905
#define EVENT_ARTIFACT_DRAGONROD       906
#define EVENT_ROOM_BLADEBARRIER       1001
#define EVENT_ROOM_AGGROCHECK         1002
#define EVENT_ROOM_PENTAGRAM          1003
#define EVENT_ROOM_GEYSER             1004
#define EVENT_ROOM_VINES              1005
#define EVENT_ROOM_SHADOWVEIL         1006
#define EVENT_ROOM_CALLWILD           1007
#define EVENT_ROOM_KINGDOMQUEST       1008
#define EVENT_ROOM_ACT                1009
#define EVENT_ROOM_MISDIRECT          1010
#define EVENT_ROOM_TENDRILS           1011
#define EVENT_ROOM_WARDING            1012
#define EVENT_ROOM_BLASTWARD          1013
#define EVENT_ROOM_DOOMBOLT           1014
#define EVENT_ROOM_PWALL              1015
#define EVENT_ROOM_WATERDOME          1016
#define EVENT_ROOM_SHADOWGUARD        1017
#define EVENT_ROOM_DISPEL_MAGIC       1018
#define EVENT_ROOM_EXTRA_ACTION       1019
#define EVENT_ROOM_CANTRIP            1020
#define EVENT_ROOM_TRACKS             1021
#define EVENT_AREA_PLAGUE             2001
#define EVENT_AREA_EARTHMOTHER        2002
#define EVENT_AREA_RESET              2003
#define EVENT_AREA_MILKANDHONEY       2004


#define EVENT_UNOWNED       0
#define EVENT_OWNER_OBJ     1
#define EVENT_OWNER_CHAR    2
#define EVENT_OWNER_DESC    3
#define EVENT_OWNER_ROOM    4
#define EVENT_OWNER_AREA    5
#define EVENT_OWNER_GAME    6
