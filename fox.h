#include "LIST.H"
#include "HASHTABLE.H"
#include "SXMLC.H"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <allegro.h>

//---------------------------
#define BITMAP_FOX_UP 				0
#define BITMAP_FOX_DOWN 			1
#define BITMAP_FOX_GO 				2

#define BITMAP_MAX_FOX_EXPLOSION 	28
#define BITMAP_MAX_BOSS_EXPLOSION 	50
#define BITMAP_MAX_BOSS_FIRE 		10

//---------------------------
#define	BALKAN						0
#define LASER						1

#define GUIDED_MISSILE				2
#define MISSILE						3

//---------------------------
#define BITMAP_BALKAN				0
#define BITMAP_BALKAN2				1
#define BITMAP_BALKAN3				2

#define BITMAP_LASER				3
#define BITMAP_LASER2				4
#define BITMAP_LASER3				5

#define BITMAP_GUIDED_MISSILE		6
#define BITMAP_GUIDED_MISSILE2		7
#define BITMAP_GUIDED_MISSILE3		8

#define BITMAP_MISSILE				9
#define BITMAP_MISSILE2				10
#define BITMAP_MISSILE3				11

//---------------------------
#define BITMAP_ITEM_BALKAN			0
#define BITMAP_ITEM_BALKAN2			1
#define BITMAP_ITEM_BALKAN3			2

#define BITMAP_ITEM_LASER			3
#define BITMAP_ITEM_LASER2			4
#define BITMAP_ITEM_LASER3			5

#define BITMAP_ITEM_GUIDED_MISSILE	6
#define BITMAP_ITEM_MISSILE			7
//---------------------------
	
#define MOVE_LEFT					0
#define MOVE_RIGHT					1
#define MOVE_UP						2
#define MOVE_DOWN					3
#define MOVE_ZOOMIN_LEFT			4
#define MOVE_ZOOMIN_RIGHT			5
#define MOVE_WAVE					6
#define MOVE_RIGHT_CENTER			7 	// 보스용
#define MOVE_FIXED					8 	// 보스용
#define MOVE_ORBIT					9
#define MOVE_GUIDED					10
#define MOVE_GUIDED_BACK			11

#define SOUND_ITEM			0
#define SOUND_ENEMY_EXPLOD	1
#define SOUND_FOX_EXPLOD		2
#define SOUND_POWERUP			3
#define SOUND_SIREN			4
#define SOUND_BOSS_LASER		5
#define SOUND_BOSS_SCREAM		6
#define SOUND_BOSS_EXPLOD		7
#define SOUND_MAX_POWER		8
#define SOUND_PAUSE			9

#define BGM_STAGE1			0
#define BGM_STAGE_CLEAR		1
#define BGM_BOSS			2
	
//---------------------------
SAMPLE *sound_effect[20];

MIDI *bgm_midi[10];

BITMAP *fox_bitmap;
BITMAP *fox_bitmaps[4];
BITMAP *fox_light_bitmaps[4];
BITMAP *fox_life_heart_bitmap;
BITMAP *fox_life_small_bitmap;
BITMAP *fox_enemy_life_heart_bitmap;
BITMAP *fox_explosion_bitmaps[10*10];
BITMAP *fire_bitmaps[16*1];
BITMAP *fox_item_bitmaps[20];
BITMAP *fox_damage_bitmap;

static HashTable *fox_weapon_table;

int fox_life_count;
int fox_energy_count;

int fox_bitmap_index;
int fox_explosion;
int fox_explosion_time;

int boss_explosion;
int boss_explosion_time;

char fox_weapon_name[30];
char fox_weapon_sub_name[30];

int fox_weapon_level = 0;	// 동일한 아이템을 계속 먹을 경우 파워가 증가한다.
int fox_weapon_max_level = FALSE;
int fox_weapon_sub_level = 0;

char fox_twinkling;		// 깜빡임 토글 변수
long fox_twinkling_time;	// 마지막 깜빡인 시간

long fox_ghost_time;		// 무적 시작 시간
long fox_attacked_time;		// 적 무기와 충돌난 시간

long fox_collided_time;		// 적 무기와 충돌난 시간
int fox_collided_index;		// 적 무기와 충돌난 시간

long fox_weapon_time = 0;						// 무기 발사 시간
long fox_item_weapon_time = 0;					// 아이템 무기 발사 시간
long fox_urgency_weapon_time = 0;		// 응급 무기 발사 시간

long stage_elapsed_time = 0;				// 스테이지 진행된 시간

long fox_popup_message_time = 0;
long fox_popup_message_show_time = 0;
int fox_popup_show_emblem = FALSE;

BITMAP *explosion_bitmaps[10*10];
BITMAP *boss_explosion_bitmaps[8*8];

BITMAP *stage_bg_bitmap;
BITMAP *stage_buf_bitmap;
BITMAP *stage_popup_bitmap;
BITMAP *stage_bitmap;

int fox_explosion_sprite_count;
int enemy_explosion_sprite_count;
int boss_explosion_sprite_count;
int fire_sprite_count;

int fox_x = 0;			// 기체 현재 x위치
int fox_y = 0;			// 기체 현재 y위치
int fox_restore = 0;	// 기체 원상 복귀
float fox_speed = 0.0;		

// ----------------------------------------------------------------
typedef struct {
	List *bitmaps;
#if 0
	List *weapon_bitmaps;
#endif
	int weak_x, weak_y;
	int weak_w, weak_h;
} ENEMY_RES;

static HashTable *stage_res_table;

// ----------------------------------------------------------------
// 궤적
typedef struct {
	float vx, vy;
	unsigned int time;
} ORBIT;

static HashTable *orbit_table;

typedef struct {
	BITMAP *bitmap;
	int power;
	float x, y;
	float vx, vy;
	int direction;
	float speed;
	float delay;
	float angle;
} WEAPON;

typedef struct {
	BITMAP *bitmap;
	float x, y;
	float speed;
	int power;
	float accel;
	List *x_list;
	int smoke_begin;
	int smoke_index;
	long smoke_time;
} WEAPON_MISSILE;

typedef struct {
	BITMAP *bitmap;
	int power;
	float x, y;
	float vx, vy;
	float angle;
	int target_enemy;
	float speed;
	float accel;
	int smoke_begin;
	int smoke_index;
	long smoke_time;
	long launch_time;
} WEAPON_GUIDED_MISSILE;

typedef struct {
	char name[30];				// 아이템 종류
	float x, y;
	float dx, dy;
	float speed;
	float accel;
	long creation_time;		// 생성 시간

	int bitmaps[10];			// 스프라이트 비트맵 인덱스 배열
	int bitmap_index;			// 현재 스프라이트 번호
	int bitmap_show_time;		// 스프라이트 교체 시간
	int bitmap_count;			// 전체 스프라이트 갯수
} ITEM;

typedef struct {
	BITMAP *bitmap;
	int power;
	float x, y;
	float vx, vy;
	int direction;
	int speed;
	float delay;
} ENEMY_WEAPON;
						
typedef struct {
	int x, y;
	long time;
	int index;
} ENEMY_FIRE;

typedef struct {
	long creation_time;

	int number;
	int explosion;
	long explosion_time;
	int max_life;
	int life;
	char name[30];
	char item[30];					// 아이템 이름
	
	char side[30];					// 아이템 이름

	float x, y;
	int w, h;
	float vx, vy;

	int trace;
	float trace_angle;

	// 약점 영역
	int weak_x, weak_y;
	int weak_w, weak_h;

	float base_y;				// WAVE 이동시 사용
	float base_speed;			// 이동 속도
	float speed;				// 현재 이동 속도
	float accel;
	long launch_time;			// 첫번째 무기 발사 시간
	long launch_sub_time;			// 두번째 무기 발사 시간 (보스에 사용됨)
	long sub_weapon_time;		// 두번째 무기 에니메이션 시간 (보스에 사용됨)
	int sub_weapon_index;		// 두번째 무기 에니메이션 시간 (보스에 사용됨)
	int reload_time;			// 무기 발사 지연 시간
	char attack_begin;			// 무기 발사 시작
	char attack_pause;			// 무기 발사 잠시 멈춤
	int attack_method;			// 무기 발사 알고리즘 변경
	int attack_odd;				// 원형 탄환 발사시 탄환 틀어주기
	long attack_odd_time;		// 원형 탄환 발사시 탄환을 틀어준 시간
	long attack_method_time;	// 무기 발사 알고리즘 변경 시간
	int motion;					// 이동 알고리즘
	long attacked_time;			// 공격 받은 시간
	float angle;

	List *angry_point_list;
	long angry_time;			// 마지막 성낸 시간
	int angry_max_count;		// 최대 성낼 횟수
	int angry_count;			// 성난 횟수

	List *fire_point_list;
	int fire_count;
	int fire_max_count;			// 최대 불 횟수
	List *fire_list;			// 불의 위치등을 기억하는 리스트

	List *bitmaps;
	int bitmap_index;			// 현재 비트맵 번호
	int bitmap_show_time;		// 마지막 비트맵 출력 시간
	float bitmap_size;
	int bitmap_size_time;		// 확대된 시간
	
	char destroyed;			// 파괴됨
	int revenge_nway;				// 폭팔시 n-way 탄 발사

	int zoom_done;
	
	char orbit_name[30];
	int orbit_index;
	long orbit_time;

} ENEMY;

static List *enemy_list;
static List *enemy_item_list;
static List *enemy_weapon_list;

static List *fox_item_history;

static List *fox_weapon_list;
static List *fox_guided_missile_list;
static List *fox_missile_list;

// ----------------------------------------------------------------
typedef struct {
	BITMAP *bitmap;
	int power;
	int reload_time;
	int speed;
	int launch_count;
} FOX_WEAPON;

typedef struct {
	int time;
	int kind;
	int life;
	float speed;
	int reload_time;	// 무기 발사 지연 시간
	char item[30];	
	char side[30];		// 최초 출현 면 (right, left, top, bottom)
	float first_location;				// 최초 출현 좌표(%)
	int motion;			// 이동 알고리즘
	char orbit_name[30];
	char name[30];
	int revenge_nway;
	float accel;
	List *angry_point_list;
	List *fire_point_list;
} ENEMY_ADVENT;

static List *enemy_advent_list;
// ----------------------------------------------------------------

int process_fox_weapon_attack(
		float weapon_x, float weapon_y, int weapon_w, int weapon_h, 
		int weapon_power, ENEMY *enemy);
void game_pause(void);
