
#define TRACK_SIZE_X	65
#define TRACK_SIZE_Y	15
#define MAP_DRAW_COORDS(i,j) ((i*16)+j)
#define TRACKA_STR_ARRAY {"============/=/=============================\\", \
"==========/  /                               \\", 							\
"========/   /   /=====\\=============/=====\\   \\", 						\
"           |  /         \\         /         \\  |", 						\
"           |/             \\  |  /             \\|", 						\
"           |                \\|/                |", 						\
"           |                 |                 |", 						\
"           |                /|\\                |", 						\
"           |\\             /  |  \\             /|", 						\
"           |  \\         /         \\         /  |", 						\
"            \\   \\=====/=============\\=====/   /", 						\
"========\\    \\                               /", 						\
"=========\\    \\=====\\==================/====/==================", 		\
"===========\\          \\              /", 								\
"=============\\==========\\==========/===========================" }

typedef struct sensor_map_chars_t {
	char original;
	char activated;
	int x;
	int y;
}sensor_map_chars_t;


void track_a_sensor_char_init(sensor_map_chars_t* sensor_chars);
void track_b_sensor_char_init(sensor_map_chars_t* sensor_chars);
