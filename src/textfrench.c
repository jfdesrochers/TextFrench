#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define DEBUG 0

#define MY_UUID { 0x42, 0x35, 0x46, 0xE7, 0x54, 0x18, 0x4F, 0x47, 0x96, 0x63, 0xF0, 0xDB, 0x98, 0x7C, 0x04, 0x40 }
PBL_APP_INFO(MY_UUID,
             "TextFrench", "lastfuture, pitoo, JF Desrochers",
             2, 2, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
#if DEBUG
             APP_INFO_WATCH_FACE // APP_INFO_STANDARD_APP
#else
			 APP_INFO_WATCH_FACE
#endif
);

void itoa(int num, char* buffer) {
    const char digits[11] = "0123456789";
    if(num > 9) {
        buffer[0] = digits[num / 10];
        buffer[1] = digits[num % 10];
    } else {
        buffer[0] = digits[num % 10];
        buffer[1] = '\0';
    }
}

Window window;
TextLayer row_1, row_1b, row_2, row_2b, row_3, row_3b, row_4, row_4b, row_5, row_5b;
PropertyAnimation anim_1, anim_1b, anim_2, anim_2b, anim_3, anim_3b, anim_4, anim_4b, anim_5, anim_5b;
static char row_1_buffer[20], row_2_buffer[20], row_3_buffer[20], row_4_buffer[20], row_5_buffer[20], row_1_oldbuf[20], row_2_oldbuf[20], row_3_oldbuf[20], row_4_oldbuf[20], row_5_oldbuf[20];
static int row_1_x, row_2_x, row_3_x, row_4_x, row_5_x, row_1_y, row_2_y, row_3_y, row_4_y, row_5_y, row_1_oldx, row_2_oldx, row_3_oldx, row_4_oldx, row_5_oldx, row_1_oldy, row_2_oldy, row_3_oldy, row_4_oldy, row_5_oldy;
static bool row_3_asc, row_4_asc, has_row_2, has_row_3, has_row_4, firstblood, tenplusone;
GFont fontHour, fontUhr, fontMinutes, fontDate;

PblTm t;


/////////////////////////////////////////////////////////////////////////

#include "string.h"
static const char* MONTHS[] = {
    "Janvier",
    "Février",
    "Mars",
    "Avril",
    "Mai",
    "Juin",
    "Juillet",
    "Août",
    "Septembre",
    "Octobre",
    "Novembre",
    "Décembre",
};
static const char* WEEKDAYS[] = {
    "Di",
    "Lu",
    "Ma",
    "Me",
    "Je",
    "Ve",
    "Sa",
};
static const char* TEENS[] = {
    "minuit",
    "une",
    "deux",
    "trois",
    "quatre",
    "cinq",
    "six",
    "sept",
    "huit",
    "neuf",
    "dix",
    "onze",
    "douze",
    "treize",
    "quatorze",
    "quinze",
    "seize",
    "dix-sept",
    "dix-huit",
    "dix-neuf",
};
static const char* TEENS_DOTLESS[] = { // ı
    "minuit",
    "une",
    "deux",
    "troıs",
    "quatre",
    "cınq",
    "sıx",
    "sept",
    "huit",
    "neuf",
    "dix",
    "onze",
    "douze",
    "treıze",
    "quatorze",
    "quınze",
    "seıze",
    "dix-sept",
    "dix-huit",
    "dix-neuf",
};
static const char* TENS[] = {
    "vingt",
    "trente",
    "quarante",
    "cinquante",
    "soixante",
};
static const char* TENS_DOTLESS[] = { // ı
    "vıngt",
    "trente",
    "quarante",
    "cınquante",
    "soıxante",
};
static const bool TEENS_ASC[] = {
    false,
    false,
    true,
    false,
    false,
    false,
    false,
    false,
    true,
    true,
    true,
    false,
    true,
    false,
    false,
    false,
    false,
    true,
    true,
    true,
};
static const bool TENS_ASC[] = {
    false,
    false,
    false,
    false,
    false,
};

static const char* STR_MIDI = "midi";
static const char* STR_UND = "et ";
static const char* STR_SPACE = " ";
static const char* STR_DOT = ".";
static const char* STR_S = "s";

static const char* STR_HOUR = "heure";

static const int UHR_ASC = 28; // 28 // 33
static const int MINUTES_ASC = 25; // 25 // 26
static const int MINUTES2_ASC = 28; // 28 // 28
static const int DATE_ASC = 36; // 36 // 40

static const int MINUTES_X = 5; // 5 // 0
static const int MINUTES2_X = 5; // 5 // 0

void setup_text_layer(TextLayer* row, PropertyAnimation *this_anim, int x, int y, int oldx, int oldy, GFont font, int magic, bool delayed, bool black){
    int rectheight = 50;
    text_layer_set_text_color(row, GColorWhite);
    if (black) {
        text_layer_set_background_color(row, GColorBlack);
        rectheight = 37;
    } else {
        text_layer_set_background_color(row, GColorClear);
    }
    layer_add_child(&window.layer, &row->layer);
    text_layer_set_font(row,font);

    int speed = 1000;
    int distance = oldy - y;

    if (distance < 0) { distance *= -1; }

    if (firstblood) {
        speed = 600;
    } else if (x == -144) {
        speed = 1400;
    } else if (oldx == 144) {
        speed = 1000;
    } else {
        speed = 500;
    }

    GRect start_rect = GRect(oldx,oldy,144-oldx-1,rectheight);
    GRect target_rect = GRect(x,y,144-x-1,rectheight);

    if (magic == 1) { // disappear
        start_rect = GRect(oldx,oldy,144-oldx-1,rectheight);
        target_rect = GRect(-150,oldy,144-oldx-1,rectheight); // loin a gauche pour sortir de l'ecran
    } else if (magic == 2) { // reappear
        start_rect = GRect(144,y,144-x-1,rectheight);
        target_rect = GRect(x,y,144-x-1,rectheight);
    } else if (magic == 3) { // and stay down
        start_rect = GRect(0,0,0,0);
        target_rect = GRect(0,0,0,0);
        speed = 1;
    } else {
    }

    if (magic != 3) {
        layer_set_frame(&row->layer, start_rect);
        property_animation_init_layer_frame(this_anim, &row->layer, NULL, &target_rect);

        animation_set_duration(&this_anim->animation, speed);
        animation_set_curve(&this_anim->animation, AnimationCurveEaseInOut);
        if (delayed) {
            animation_set_delay(&this_anim->animation, 100);
        }
        animation_schedule(&this_anim->animation);
    }
}

void GetTime(int hours, int minutes, int day, int month, int weekday){

    has_row_2 = false;
    has_row_3 = false;
    has_row_4 = false;

    int hours12h = hours % 12;
    if (hours == 12) { strcat(row_1_buffer, STR_MIDI); }
    else strcat(row_1_buffer, TEENS[hours12h]);

    if (hours12h > 0) {
        strcat(row_2_buffer, STR_HOUR);
        if (hours12h > 1) strcat(row_2_buffer, STR_S);
        has_row_2 = true;
    }

    tenplusone = false;

    if (minutes == 0) {
    } else if (minutes < 20) {
        row_3_asc = TEENS_ASC[minutes];
        if (row_3_asc == false) strcat(row_3_buffer, TEENS_DOTLESS[minutes]);
        else strcat(row_3_buffer, TEENS[minutes]);
        has_row_3 = true;
    } else {
        int dizaine = minutes/10;
        int unite = minutes % 10;
        row_3_asc = TENS_ASC[dizaine-2];
        if (row_3_asc == false) strcat(row_3_buffer, TENS_DOTLESS[dizaine-2]);
        else strcat(row_3_buffer, TENS[dizaine-2]);
        has_row_3 = true;

        if (unite > 0) {
            if (unite == 1) { // descente verticale des unites
                tenplusone = true;
                strcat(row_4_buffer, STR_UND);
            }
            row_4_asc = TEENS_ASC[unite];
            if (row_4_asc == false) strcat(row_4_buffer, TEENS_DOTLESS[unite]);
            else strcat(row_4_buffer, TEENS[unite]);
            has_row_4 = true;
        }
    }

    char daynum[] = "xx";
    itoa(day,daynum);
    strcat(row_5_buffer, WEEKDAYS[weekday]);
    strcat(row_5_buffer, STR_DOT);
    strcat(row_5_buffer, STR_SPACE);
    strcat(row_5_buffer, daynum);
    strcat(row_5_buffer, STR_SPACE);
    strcat(row_5_buffer, MONTHS[month]);
}

void update_time(PblTm* t){

    memset(row_1_oldbuf,0,20);
    memset(row_2_oldbuf,0,20);
    memset(row_3_oldbuf,0,20);
    memset(row_4_oldbuf,0,20);
    memset(row_5_oldbuf,0,20);

    strcat(row_1_oldbuf,row_1_buffer);
    strcat(row_2_oldbuf,row_2_buffer);
    strcat(row_3_oldbuf,row_3_buffer);
    strcat(row_4_oldbuf,row_4_buffer);
    strcat(row_5_oldbuf,row_5_buffer);

    memset(row_1_buffer,0,20);
    memset(row_2_buffer,0,20);
    memset(row_3_buffer,0,20);
    memset(row_4_buffer,0,20);
    memset(row_5_buffer,0,20);

    bool has_row_3_old = has_row_3;
    bool has_row_4_old = has_row_4;

//    strcat(row_1_buffer, STR_SPACE); // workaround for weird "z" bug

#if DEBUG
    static int dm = 52;
    static int dh = 1;
    //GetTime(7,57,2,8,0); // longest strings?
    GetTime(dh,dm,t->tm_mday,t->tm_mon,t->tm_wday);
    dm++;
    if (dm == 60) { dm = 0; dh++; }
    if (dh == 60) { dh = 0; }
#else
    GetTime(t->tm_hour,t->tm_min,t->tm_mday,t->tm_mon,t->tm_wday);
#endif

    int spacing = 0;

    row_1_oldx = row_1_x;
    row_2_oldx = row_2_x;
    row_3_oldx = row_3_x;
    row_4_oldx = row_4_x;
    row_5_oldx = row_5_x;

    row_1_oldy = row_1_y;
    row_2_oldy = row_2_y;
    row_3_oldy = row_3_y;
    row_4_oldy = row_4_y;
    row_5_oldy = row_5_y;

    row_1_x = row_2_x = row_3_x = row_4_x = row_5_x = 20;

    row_1_y = spacing;
    if (has_row_2) {
        spacing += UHR_ASC;
    }
    row_2_y = spacing;
    if (has_row_3) {
        if (has_row_2) {
            if (row_3_asc) { spacing += MINUTES_ASC; } else { spacing += MINUTES_ASC-MINUTES2_X; }
        } else {
            if (row_3_asc) { spacing += MINUTES2_ASC; } else { spacing += MINUTES2_ASC-MINUTES2_X; }
        }
    }
    row_3_y = spacing;
    if (has_row_4) {
        if (row_4_asc) { spacing += MINUTES2_ASC; } else { spacing += MINUTES2_ASC-MINUTES2_X; }
    }
    row_4_y = spacing;
    spacing += DATE_ASC;
    row_5_y = spacing;

    row_1_x -= row_1_y/6;
    row_2_x -= row_2_y/6;
    row_3_x -= row_3_y/6;
    row_4_x -= row_4_y/6;
    row_5_x -= row_5_y/6;
//    row_1_x -= 7;
    row_5_x += 4;

    spacing += 22;

    int y_offset = (168-spacing)/2;

    row_1_y += y_offset;
    row_2_y += y_offset;
    row_3_y += y_offset;
    row_4_y += y_offset;
    row_5_y += y_offset;

    if (firstblood) {
        has_row_3_old = has_row_3;
        has_row_4_old = has_row_4;
        memset(row_3_oldbuf,0,20);
        memset(row_4_oldbuf,0,20);
        strcat(row_3_oldbuf,row_3_buffer);
        strcat(row_4_oldbuf,row_4_buffer);

        int sidestoggle = -144;

        row_1_oldx = sidestoggle;
        sidestoggle *= -1;
        if (has_row_2) { sidestoggle *= -1; };
        row_2_oldx = sidestoggle;
        if (has_row_3) { sidestoggle *= -1; };
        row_3_oldx = sidestoggle;
        if (has_row_4) { sidestoggle *= -1; };
        row_4_oldx = sidestoggle;
        sidestoggle *= -1;
        row_5_oldx = sidestoggle;

        row_1_oldy = row_1_y;
        row_2_oldy = row_2_y;
        row_3_oldy = row_3_y;
        row_4_oldy = row_4_y;
        row_5_oldy = row_5_y;
    }


    int magic = 0;
    bool haschanged = false;

    if (strcmp(row_1_oldbuf,row_1_buffer)) { haschanged = true; } else { haschanged = false; }

    if (haschanged && firstblood != true) {
        setup_text_layer(&row_1,&anim_1,-144,row_1_oldy,row_1_oldx,row_1_oldy,fontHour,0,false,true);
        text_layer_set_text(&row_1,row_1_oldbuf);
        setup_text_layer(&row_1b,&anim_1b,row_1_x,row_1_y,144,row_1_y,fontHour,magic,true,true);
        text_layer_set_text(&row_1b,row_1_buffer);
    } else {
        setup_text_layer(&row_1,&anim_1,row_1_x,row_1_y,row_1_oldx,row_1_oldy,fontHour,0,true,true);
        text_layer_set_text(&row_1,row_1_buffer);
        text_layer_set_text(&row_1b,STR_SPACE);
    }

    if (strcmp(row_2_oldbuf,row_2_buffer)) { haschanged = true; } else { haschanged = false; }

    if (haschanged && firstblood != true) {
        setup_text_layer(&row_2,&anim_2,-144,row_2_oldy,row_2_oldx,row_2_oldy,fontUhr,0,false,false);
        text_layer_set_text(&row_2,row_2_oldbuf);
        setup_text_layer(&row_2b,&anim_2b,row_2_x,row_2_y,144,row_2_y,fontUhr,magic,true,false);
        text_layer_set_text(&row_2b,row_2_buffer);
    } else {
        setup_text_layer(&row_2,&anim_2,row_2_x,row_2_y,row_2_oldx,row_2_oldy,fontUhr,0,true,false);
        text_layer_set_text(&row_2,row_2_buffer);
        text_layer_set_text(&row_2b,STR_SPACE);
    }

    if (strcmp(row_3_oldbuf,row_3_buffer)) { haschanged = true; } else { haschanged = false; }

    if (has_row_3 == has_row_3_old && has_row_3 == true) {
        magic = 0;
    } else if (has_row_3 == has_row_3_old && has_row_3 == false) {
        magic = 3; // stay down
    } else if (has_row_3 != has_row_3_old && has_row_3 == false) {
        magic = 1; // disappear
    } else if (has_row_3 != has_row_3_old && has_row_3 == true) {
        magic = 2; // reappear
    }
    if (magic == 0) {
        if (haschanged) {
            if (tenplusone) {
                setup_text_layer(&row_3,&anim_3,row_3_x,row_3_y,144,row_3_y,fontMinutes,magic,true,false);
                text_layer_set_text(&row_3,row_3_buffer);
                text_layer_set_text(&row_3b,STR_SPACE);
            } else {
                setup_text_layer(&row_3,&anim_3,-144,row_3_oldy,row_3_oldx,row_3_oldy,fontMinutes,magic,false,false);
                text_layer_set_text(&row_3,row_3_oldbuf);
                setup_text_layer(&row_3b,&anim_3b,row_3_x,row_3_y,144,row_3_y,fontMinutes,magic,true,false);
                text_layer_set_text(&row_3b,row_3_buffer);
            }
        } else {
            setup_text_layer(&row_3,&anim_3,row_3_x,row_3_y,row_3_oldx,row_3_oldy,fontMinutes,magic,false,false);
            text_layer_set_text(&row_3,row_3_buffer);
            text_layer_set_text(&row_3b,STR_SPACE);
        }
    } else {
        setup_text_layer(&row_3,&anim_3,row_3_x,row_3_y,row_3_oldx,row_3_oldy,fontMinutes,magic,false,false);
        if (magic == 1) {
            text_layer_set_text(&row_3,row_3_oldbuf);
        } else {
            text_layer_set_text(&row_3,row_3_buffer);
        }
        text_layer_set_text(&row_3b,STR_SPACE);
    }

    if (strcmp(row_4_oldbuf,row_4_buffer)) { haschanged = true; } else { haschanged = false; }

    if (has_row_4 == has_row_4_old && has_row_4 == true) {
        magic = 0;
    } else if (has_row_4 == has_row_4_old && has_row_4 == false) {
        magic = 3; // stay down
    } else if (has_row_4 != has_row_4_old && has_row_4 == false) {
        magic = 1; // disappear
    } else if (has_row_4 != has_row_4_old && has_row_4 == true) {
        magic = 2; // reappear
    }
    if (haschanged && tenplusone) {
        setup_text_layer(&row_4,&anim_4,row_4_x,row_4_y,row_3_oldx,row_3_oldy,fontMinutes,0,false,false);
        text_layer_set_text(&row_4,row_4_buffer);
        text_layer_set_text(&row_4b,STR_SPACE);
    } else if (magic == 0) {
        if (haschanged) {
            setup_text_layer(&row_4,&anim_4,-144,row_4_oldy,row_4_oldx,row_4_oldy,fontMinutes,magic,false,false);
            text_layer_set_text(&row_4,row_4_oldbuf);
            setup_text_layer(&row_4b,&anim_4b,row_4_x,row_4_y,144,row_4_y,fontMinutes,magic,true,false);
            text_layer_set_text(&row_4b,row_4_buffer);
        } else {
            setup_text_layer(&row_4,&anim_4,row_4_x,row_4_y,row_4_oldx,row_4_oldy,fontMinutes,magic,false,false);
            text_layer_set_text(&row_4,row_4_buffer);
            text_layer_set_text(&row_4b,STR_SPACE);
        }
    } else {
        setup_text_layer(&row_4,&anim_4,row_4_x,row_4_y,row_4_oldx,row_4_oldy,fontMinutes,magic,false,false);
        if (magic == 1) {
            text_layer_set_text(&row_4,row_4_oldbuf);
        } else {
            text_layer_set_text(&row_4,row_4_buffer);
        }
        text_layer_set_text(&row_4b,STR_SPACE);
    }

    if (strcmp(row_5_oldbuf,row_5_buffer)) { haschanged = true; } else { haschanged = false; }

    if (haschanged && firstblood != true) {
        setup_text_layer(&row_5,&anim_5,-144,row_5_oldy,row_5_oldx,row_5_oldy,fontDate,0,false,false);
        text_layer_set_text(&row_5,row_5_oldbuf);
        setup_text_layer(&row_5b,&anim_5b,row_5_x,row_5_y,144,row_5_y,fontDate,magic,true,false);
        text_layer_set_text(&row_5b,row_5_buffer);
    } else {
        setup_text_layer(&row_5,&anim_5,row_5_x,row_5_y,row_5_oldx,row_5_oldy,fontDate,0,true,false);
        text_layer_set_text(&row_5,row_5_buffer);
        text_layer_set_text(&row_5b,STR_SPACE);
    }

    if (firstblood) {
        firstblood = false;
    }
}


static void handle_minute_tick(AppContextRef app_ctx, PebbleTickEvent* e) {

#if DEBUG
    static int compte = 1;
    compte++;
    if (compte == 2) { update_time(e->tick_time); compte = 0; }
#else
    update_time(e->tick_time);
#endif

}

void handle_init(AppContextRef ctx) {
    (void)ctx;

    firstblood = true;

    window_init(&window, "Film_Plakat");
    window_stack_push(&window, true /* Animated */);
    window_set_background_color(&window, GColorBlack);
    resource_init_current_app(&APP_RESOURCES);

    fontHour = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_BI_35));
    fontUhr = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_LI_30));
    fontMinutes = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_I_33));
    fontDate = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_I_13));

    memset(row_1_buffer,0,20);
    memset(row_2_buffer,0,20);
    memset(row_3_buffer,0,20);
    memset(row_4_buffer,0,20);
    memset(row_5_buffer,0,20);

    text_layer_init(&row_2, window.layer.frame);
    text_layer_init(&row_3, window.layer.frame);
    text_layer_init(&row_4, window.layer.frame);
    text_layer_init(&row_5, window.layer.frame);
    text_layer_init(&row_2b, window.layer.frame);
    text_layer_init(&row_3b, window.layer.frame);
    text_layer_init(&row_4b, window.layer.frame);
    text_layer_init(&row_5b, window.layer.frame);

    text_layer_init(&row_1, window.layer.frame);
    text_layer_init(&row_1b, window.layer.frame);

    get_time(&t);
}


void main(void *params) {
    PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .tick_info = {
            .tick_handler = &handle_minute_tick,
#if DEBUG
            .tick_units = SECOND_UNIT
#else
            .tick_units = MINUTE_UNIT
#endif
        }
    };
    app_event_loop(params, &handlers);
}