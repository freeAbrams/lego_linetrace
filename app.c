#include "ev3api.h"
#include "app.h"
#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"
#endif

#define L_MOTOR EV3_PORT_C /* 左モーターポートの設定 */
#define R_MOTOR EV3_PORT_B /* 右モーターポートの設定 */
#define L_SENSOR EV3_PORT_3 /* 左センサーポートの設定 */
#define R_SENSOR EV3_PORT_2 /* 右センサーポートの設定 */

const int target = 90; /* 黒の線上を走行するための目標反射値 */
const int speed = 15; /* モーターのスピード */
const int rotate_speed = 10; /* ロボットの旋回スピード */
const int motor_angle_threshold = 20; /* モーターの角度閾値 */
const int color_threshold = 60; /* 色の閾値 */

/* ログファイルと時間の変数 */
FILE *log_file = NULL; /* ログファイルのポインタ */
SYSTIM start_time; /* 開始時間 */

//int found_green = 0;
int color_flag = 0; /* 特定の色を検出したかどうかのフラグ */

int l_reflect = 0; /* 左センサーの反射値 */
int r_reflect = 0; /* 右センサーの反射値 */
int l_motor_angle = 0;
int r_motor_angle = 0;
int l_speed = 0;
int r_speed = 0;

void log_task(intptr_t exinf)
{
    while (1)
    {
        SYSTIM now_time;
        get_tim(&now_time);
        fprintf(log_file, "TIME = %d, L_SENSOR = %d, R_SENSOR = %d, L_MOTOR = %d, R_MOTOR = %d\n", now_time - start_time, l_reflect, r_reflect, l_speed, r_speed);
        tslp_tsk(500);
    }
}

void cyclic_task(intptr_t exinf) 
{
    while (color_flag == 0)
    {
        /* 左右のセンサーの反射値を取得 */
        l_reflect = ev3_color_sensor_get_reflect(L_SENSOR);
        r_reflect = ev3_color_sensor_get_reflect(R_SENSOR);
        int reflect_diff = l_reflect - r_reflect;
        

        /* モーターの角度差を計算 */
        l_motor_angle = ev3_motor_get_counts(L_MOTOR);
        r_motor_angle = ev3_motor_get_counts(R_MOTOR);
        int diff = l_motor_angle - r_motor_angle;

        /* 左右の反射値に応じてロボットを制御 */
        l_speed = speed + (l_reflect - r_reflect)/4;
        r_speed = speed + (r_reflect - l_reflect)/4;
        
        ev3_motor_set_power(L_MOTOR, l_speed);
        ev3_motor_set_power(R_MOTOR, r_speed);
        //ev3_motor_steer(L_MOTOR, R_MOTOR, speed, reflect_diff);

        /* 色センサーが青、赤、黄を検出するまでループ */
        if(ev3_color_sensor_get_color(L_SENSOR) == COLOR_BLUE || ev3_color_sensor_get_color(L_SENSOR) == COLOR_RED || ev3_color_sensor_get_color(L_SENSOR) == COLOR_YELLOW || ev3_color_sensor_get_color(R_SENSOR) == COLOR_BLUE || ev3_color_sensor_get_color(R_SENSOR) == COLOR_RED || ev3_color_sensor_get_color(R_SENSOR) == COLOR_YELLOW)
        {
            ev3_motor_reset_counts(L_MOTOR);
            ev3_motor_reset_counts(R_MOTOR);
            color_flag = 1;
            break;
        }
    }
    while (color_flag ==1)
    {
        ev3_motor_steer(L_MOTOR, R_MOTOR, 10, 20);
        tslp_tsk(500);
        ev3_motor_steer(L_MOTOR, R_MOTOR, 10, -20);
        tslp_tsk(500);
        if (ev3_color_sensor_get_color(L_SENSOR) == COLOR_GREEN ||ev3_color_sensor_get_color(R_SENSOR) == COLOR_GREEN)
        {
            color_flag = 0;
            break;
        }
        if (ev3_color_sensor_get_color(L_SENSOR) == COLOR_YELLOW ||ev3_color_sensor_get_color(R_SENSOR) == COLOR_YELLOW)
        {
            ev3_motor_steer(L_MOTOR, R_MOTOR, 10, 60);
            tslp_tsk(500);
            break;
        }
        if (ev3_color_sensor_get_color(L_SENSOR) == COLOR_RED ||ev3_color_sensor_get_color(R_SENSOR) == COLOR_RED)
        {
            ev3_motor_steer(L_MOTOR, R_MOTOR, 10, 60);
            tslp_tsk(500);
            break;
        }
    }    
}

void main_task(intptr_t unused){
    //モーターの設定
    ev3_motor_config(L_MOTOR, LARGE_MOTOR);
    ev3_motor_config(R_MOTOR, LARGE_MOTOR);
    //センサの設定
    ev3_sensor_config(R_SENSOR, COLOR_SENSOR);
    ev3_sensor_config(L_SENSOR, COLOR_SENSOR);
    //タスクを開始する
    ev3_sta_cyc(CYCHDR1);
    act_tsk((intptr_t)log_task);
    ev3_sta_cyc(CYCHDR2);
    act_tsk((intptr_t)cyclic_task);
}
