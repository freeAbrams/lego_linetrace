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
const int speed = 15; /* モーターのスピード */

int l_reflect = 0; /* 左センサーの反射値 */
int r_reflect = 0; /* 右センサーの反射値 */
int l_motor_angle = 0;
int r_motor_angle = 0;
int l_speed = 0;
int r_speed = 0;

void log_task(intptr_t exinf)
{
        FILE *log_file = NULL;
        log_file = fopen("/log.txt", "a");
        while (1)
        {
            fprintf(log_file, "L_SENSOR = %d, R_SENSOR = %d, L_MOTOR = %d, R_MOTOR = %d\n", l_reflect, r_reflect, l_speed, r_speed);
            tslp_tsk(500);
        }
        fclose(log_file);
}

void run_task(intptr_t unused) 
{
    while (1)
    {
        /* 左右のセンサーの反射値を取得 */
        l_reflect = ev3_color_sensor_get_reflect(L_SENSOR);
        r_reflect = ev3_color_sensor_get_reflect(R_SENSOR);
        /* 左右の反射値に応じてロボットを制御 */
        l_speed = speed + (l_reflect - r_reflect)/4;
        r_speed = speed + (r_reflect - l_reflect)/4;
        ev3_motor_set_power(L_MOTOR, l_speed);//左モーターのスピードを設定
        ev3_motor_set_power(R_MOTOR, r_speed);//右モーターのスピードを設定
        if (l_reflect <= 25 && r_reflect <= 25)
        {
            ev3_motor_stop(L_MOTOR, true);
            ev3_motor_stop(R_MOTOR, true);
            ev3_motor_rotate(R_MOTOR, 300, speed, true);
            tslp_tsk(3000);
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
    act_tsk(RUN_TASK);
}
