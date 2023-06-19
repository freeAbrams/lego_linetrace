/* ヘッダーファイルの読み込み */
#include "ev3api.h"
#include "app.h"

#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"
#endif

/* ポートの設定 */
#define L_MOTOR EV3_PORT_B /* 左モーターポートの設定 */
#define R_MOTOR EV3_PORT_C /* 右モーターポートの設定 */
#define L_SENSOR EV3_PORT_1 /* 左センサーポートの設定 */
#define R_SENSOR EV3_PORT_2 /* 右センサーポートの設定 */
#define LOG_FILENAME "log.txt" /* ログファイルの名前 */

/* 定数の設定 */
const int target = 30; /* 黒の線上を走行するための目標反射値 */
const int speed = 50; /* モーターのスピード */
const int rotate_speed = 10; /* ロボットの旋回スピード */
const int motor_angle_threshold = 20; /* モーターの角度閾値 */
const int color_threshold = 60; /* 色の閾値 */

/* ログファイルと時間の変数 */
FILE *log_file = NULL; /* ログファイルのポインタ */
SYSTIM start_time; /* 開始時間 */

bool color_flag = false; /* 特定の色を検出したかどうかのフラグ */

void main_task(intptr_t unused) 
{ 
    /* モーターとセンサーの設定 */
    ev3_sensor_config(L_SENSOR, COLOR_SENSOR);
    ev3_sensor_config(R_SENSOR, COLOR_SENSOR);
    ev3_motor_config(L_MOTOR, LARGE_MOTOR);
    ev3_motor_config(R_MOTOR, LARGE_MOTOR);
    
    /* ログファイルを開く */
    log_file = fopen(LOG_FILENAME, "w");
    get_tim(&start_time);

    /* タスクを開始する */ 
    ev3_sta_cyc(LOG_CYC);
    act_tsk(TRACE_TASK);
}

void log_task(intptr_t unused)
{
    if(log_file != NULL)
    {
        SYSTIM now_time;
        get_tim(&now_time);
        fprintf(log_file, "%d, %d, %d, %d, %d, %d, %d\n", now_time - start_time,
                ev3_motor_get_counts(L_MOTOR), ev3_motor_get_counts(R_MOTOR),
                ev3_motor_get_power(L_MOTOR), ev3_motor_get_power(R_MOTOR),
                ev3_color_sensor_get_reflect(L_SENSOR), ev3_color_sensor_get_reflect(R_SENSOR));
    }
}

void trace_task(intptr_t unused)
{
    int l_motor_angle = 0;
    int r_motor_angle = 0;
    int diff = 0;
    colorid_t color;
    bool found_green = false;
    while (1)
    {
        while(!color_flag)
        {
        /* 左右のセンサーの反射値を取得 */
        int l_reflect = ev3_color_sensor_get_reflect(L_SENSOR);
        int r_reflect = ev3_color_sensor_get_reflect(R_SENSOR);

        /* モーターの角度差を計算 */
        l_motor_angle = ev3_motor_get_counts(L_MOTOR);
        r_motor_angle = ev3_motor_get_counts(R_MOTOR);
        diff = l_motor_angle - r_motor_angle;

        /* 左右の反射値に応じてロボットを制御 */
        int l_speed = speed - (l_reflect - target);
        int r_speed = speed - (r_reflect - target);
        ev3_motor_set_power(L_MOTOR, l_speed);
        ev3_motor_set_power(R_MOTOR, r_speed);

        /* 色センサーが青、赤、黄を検出するまでループ */
        if(ev3_color_sensor_get_color(L_SENSOR) == COLOR_BLUE || ev3_color_sensor_get_color(L_SENSOR) == COLOR_RED || ev3_color_sensor_get_color(L_SENSOR) == COLOR_YELLOW || ev3_color_sensor_get_color(R_SENSOR) == COLOR_BLUE || ev3_color_sensor_get_color(R_SENSOR) == COLOR_RED || ev3_color_sensor_get_color(R_SENSOR) == COLOR_YELLOW)
        {
            ev3_motor_reset_counts(L_MOTOR);
            ev3_motor_reset_counts(R_MOTOR);
            color_flag = true;
            break;
        }
        }
    }
}

void junction_task(intptr_t unused)
{
    int l_motor_angle = 0;
    int r_motor_angle = 0;
    int diff = 0;
    int diff_yellow = 0;
    int diff_red = 0;
    int diff_green = 0;
    colorid_t color;
    bool found_green = false;
    while (1)
    {
        while(color_flag)
        {
        /* モーターの角度を計算 */
        l_motor_angle = ev3_motor_get_counts(L_MOTOR);
        r_motor_angle = ev3_motor_get_counts(R_MOTOR);
        diff = l_motor_angle - r_motor_angle;

        /* カラーセンサーで色を検出 */
        color = ev3_color_sensor_get_color(L_SENSOR);

        /* カラーに応じて動作を変更 */
        switch (color) 
        {
            case COLOR_YELLOW:
                color_flag = true;
                diff_yellow = diff;
                ev3_motor_set_power(L_MOTOR, rotate_speed);
                ev3_motor_set_power(R_MOTOR, -rotate_speed);
                break;
            case COLOR_RED:
                color_flag = true;
                diff_red = diff;
                ev3_motor_set_power(L_MOTOR, rotate_speed);
                ev3_motor_set_power(R_MOTOR, -rotate_speed);
                break;
            case COLOR_GREEN:
                diff_green = diff;
                found_green = true;
                color_flag = false;
                break;
            default:
                ev3_motor_set_power(L_MOTOR, rotate_speed);
                ev3_motor_set_power(R_MOTOR, -rotate_speed);
                break;
        }

        /* 少し待つ */
        tslp_tsk(10);
        }
    }
    

    /* 緑に向かって前進 */
    int target_diff = diff_green;
    while(abs(ev3_motor_get_counts(L_MOTOR) - ev3_motor_get_counts(R_MOTOR)) < abs(target_diff))
    {
        ev3_motor_set_power(L_MOTOR, speed);
        ev3_motor_set_power(R_MOTOR, speed);
        tslp_tsk(10);
    }
}
