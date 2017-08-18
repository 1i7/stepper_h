#include "stepper.h"

extern "C"{
    #include "timer_setup.h"
}

// Stepper motors
static stepper sm_x, sm_y, sm_z;

/////////////////////////////////////////////////////
// настройки таймера

// для периода 1 микросекунда (1млн вызовов в секунду == 1МГц):
//   timer handler takes longer than timer period: cycle time=3us, timer period=1us
//int _timer_id = TIMER3;
//int _timer_prescaler = TIMER_PRESCALER_1_8;
//int _timer_period = 10;
//int _timer_period_us = 1;

// для периода 5 микросекунд (200тыс вызовов в секунду == 200КГц):
//   timer handler takes longer than timer period: cycle time=5us, timer period=5us
//int _timer_id = TIMER3;
//int _timer_prescaler = TIMER_PRESCALER_1_8;
//int _timer_period = 50;
//int _timer_period_us = 5;


// для периода 10 микросекунд (100тыс вызовов в секунду == 100КГц):
// На ChipKIT Uno32 наименьший вариант ок для движения по линии 3х моторов
// На ChipKIT Uno32
// 2 мотора (ок):
//   Finished cycle, max time=9
// 3 мотора (не ок):
//   timer handler takes longer than timer period: cycle time=10us, timer period=10us
//int _timer_id = TIMER3;
//int _timer_prescaler = TIMER_PRESCALER_1_8;
//int _timer_period = 100;
//int _timer_period_us = 10;

// для периода 20 микросекунд (50тыс вызовов в секунду == 50КГц):
// На ChipKIT Uno32 наименьший вариант ок для движения по линии 3х моторов
// 3 мотора (ок):
//   Finished cycle, max time=11
// 2 мотора (тем более ок):
//   Finished cycle, max time=9
// совсем не ок для движения по дуге (по 90мкс на acos/asin)
int _timer_id = TIMER3;
int _timer_prescaler = TIMER_PRESCALER_1_8;
int _timer_period = 200;
int _timer_period_us = 20;

// для периода 200 микросекунд (5тыс вызовов в секунду == 5КГц):
// ок для движения по дуге (по 90мкс на acos/asin)
//int _timer_id = TIMER3;
//int _timer_prescaler = TIMER_PRESCALER_1_8;
//int _timer_period = 2000;
//int _timer_period_us = 200;

////////////////////////////////////////////////
// минимальная задержка между шагами мотора

// пищит и не крутится
//int _step_delay_us = 1;

// гудит и не крутится
//int _step_delay_us = 10;

// крутится ок, если притормозить, то может не продолжить
// аппаратно: минимальный рабочий вариант,
// программно: не вписывается ни в одну в рабочую частоту
//int _step_delay_us = 20;

// крутится ок, если притормозить, то продолжает полюбому
// аппаратно: оптимальный вариант,
// программно: впишется в период 10 микросекунд (одновременно 2 мотора)
//int _step_delay_us = 30;

// крутится ок (как и всё, что больше 30 микросекунд)
// аппаратно: медленнее в 2 раза оптимального варианта
// программно: впишется в период 20 микросекунд (одновременно 3 мотора)
int _step_delay_us = 60;

//int _step_delay_us = 100;
//int _step_delay_us = 200;
//int _step_delay_us = 600;
//int _step_delay_us = 1000;


// Вариант с индивидуальными задержками для разных моторов

// X
// 1/32, 104 mm/s
int x_step_delay_us = 60;
int x_dist_per_step = 6250;

// Y
// 1/32, 104 mm/s
//int y_step_delay_us = 60;
//int y_dist_per_step = 6250;

// 1/1, 132 mm/s
int y_step_delay_us = 1500;
int y_dist_per_step = 200000;


static void prepare_line3() {
    // prepare_steps(stepper *smotor,
    //     long step_count, unsigned long step_delay,
    //     calibrate_mode_t calibrate_mode=NONE);
    
    // шагаем с максимальной скоростью
    prepare_steps(&sm_x, 200000, x_step_delay_us);
    // вызвать CYCLE_ERROR_MOTOR_ERROR
    //prepare_steps(&sm_x, 200000, x_step_delay_us-1);
    prepare_steps(&sm_y, 200000, y_step_delay_us);
    prepare_steps(&sm_z, 200000, _step_delay_us);
}

static void prepare_whirl2() {
    // prepare_steps(stepper *smotor,
    //     long step_count, unsigned long step_delay,
    //     calibrate_mode_t calibrate_mode=NONE);
    
    // шагаем с максимальной скоростью
    prepare_whirl(&sm_x, 1, x_step_delay_us);
    prepare_whirl(&sm_y, 1, y_step_delay_us);
}

static void prepare_buffered2() {
    // void prepare_buffered_steps(stepper *smotor,
    //    int buf_size, unsigned long* delay_buffer, long* step_buffer)
 
    static unsigned long delay_buffer[3];
    static long step_buffer[3];

    delay_buffer[0] = y_step_delay_us;
    delay_buffer[1] = y_step_delay_us*10;
    delay_buffer[2] = y_step_delay_us*2;

    step_buffer[0] = 200*10;
    step_buffer[1] = -200*5;
    step_buffer[2] = 200*2;
    
    prepare_buffered_steps(&sm_y, 3, delay_buffer, step_buffer);

    // для икса просто шаги
    prepare_steps(&sm_x, 200000, x_step_delay_us);
}

void print_cycle_error(stepper_cycle_error_t err) {
    switch(err) {
        case CYCLE_ERROR_NONE:
            Serial.print("CYCLE_ERROR_NONE");
            break;
        case CYCLE_ERROR_TIMER_PERIOD_TOO_LONG:
            Serial.print("CYCLE_ERROR_TIMER_PERIOD_TOO_LONG");
            break;
        case CYCLE_ERROR_TIMER_PERIOD_ALIQUANT_STEP_DELAY:
            Serial.print("CYCLE_ERROR_TIMER_PERIOD_ALIQUANT_STEP_DELAY");
            break;
        case CYCLE_ERROR_MOTOR_ERROR:
            Serial.print("CYCLE_ERROR_MOTOR_ERROR");
            break;
        case CYCLE_ERROR_HANDLER_TIMING_EXCEEDED:
            Serial.print("CYCLE_ERROR_HANDLER_TIMING_EXCEEDED");
            break;
        default:
            break;
    }
}

void print_motor_error(stepper &sm) {
    if(sm.error) {
        if(sm.error & STEPPER_ERROR_SOFT_END_MIN) {
            Serial.print("STEPPER_ERROR_SOFT_END_MIN");
        }
        if(sm.error & STEPPER_ERROR_SOFT_END_MAX) {
            Serial.print("STEPPER_ERROR_SOFT_END_MAX");
        }
        if(sm.error & STEPPER_ERROR_HARD_END_MIN) {
            Serial.print("STEPPER_ERROR_HARD_END_MIN");
        }
        if(sm.error & STEPPER_ERROR_HARD_END_MAX) {
            Serial.print("STEPPER_ERROR_HARD_END_MAX");
        }
        if(sm.error & STEPPER_ERROR_STEP_DELAY_SMALL) {
            Serial.print("STEPPER_ERROR_STEP_DELAY_SMALL");
        }
    } else {
        Serial.print("none");
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting stepper_h test...");
    
    // connected stepper motors
    // init_stepper(stepper* smotor, char name,
    //     int pin_step, int pin_dir, int pin_en,
    //     bool invert_dir, int step_delay,
    //     int distance_per_step)
    // init_stepper_ends(stepper* smotor,
    //     end_strategy min_end_strategy, end_strategy max_end_strategy,
    //     long long min_pos, long long max_pos);
    
    // Pinout for CNC-shield

    // Минимальный период (для максимальной скорости) шага
    // при периоде таймера 20 микросекунд (лучший вариант для
    // движения по линии 3 мотора одновременно)
    // step_delay=60 микросекунд (20*3)
    
    // X
    
    init_stepper(&sm_x, 'x', 2, 5, 8, false, x_step_delay_us, x_dist_per_step);
    //init_stepper(&sm_x, 'x', 2, 5, 8, false, _step_delay_us, 750);
    init_stepper_ends(&sm_x, NO_PIN, NO_PIN, CONST, CONST, 0, 30000000000);
    
    // Y
    init_stepper(&sm_y, 'y', 3, 6, 8, false, y_step_delay_us, y_dist_per_step);
    //init_stepper(&sm_y, 'y', 3, 6, 8, false, _step_delay_us, 750);
    init_stepper_ends(&sm_y, NO_PIN, NO_PIN, CONST, CONST, 0, 21600000000);
    
    // Z
    init_stepper(&sm_z, 'z', 4, 7, 8, false, _step_delay_us, 750);
    init_stepper_ends(&sm_z, NO_PIN, NO_PIN, CONST, CONST, 0, 100000000);

    // настройки таймера
    stepper_configure_timer(_timer_period_us, _timer_id, _timer_prescaler, _timer_period);
    
    // configure motors before starting steps
    //prepare_line3();
    //prepare_whirl2();
    prepare_buffered2();
    
    // start motors, non-blocking
    stepper_start_cycle();
}

void loop() {
    static int prevTime = 0;
    // Debug messages - print current positions of motors once per second
    // while they are rotating, once per 10 seconds when they are stopped
    // (see https://github.com/1i7/stepper_h/blob/master/3pty/arduino/README
    // to fix compile proplem)
    int currTime = millis();
    if( (stepper_cycle_running() && (currTime - prevTime) >= 1000) || (currTime - prevTime) >= 10000 ) {
        prevTime = currTime;
        Serial.print("X.pos=");
        Serial.print(sm_x.current_pos, DEC);
        Serial.print(", Y.pos=");
        Serial.print(sm_y.current_pos, DEC);
        Serial.print(", Z.pos=");
        Serial.print(sm_z.current_pos, DEC);
        Serial.println();
    }
    
    // обработчик таймера не умещается в период
    unsigned long cycle_time = stepper_cycle_max_time();
    if(cycle_time >= _timer_period_us) {
        Serial.print("***ERROR: timer handler takes longer than timer period: ");
        Serial.print("cycle time=");
        Serial.print(cycle_time);
        Serial.print("us, timer period=");
        Serial.print(_timer_period_us);
        Serial.println("us");
    }
    
    // напечатаем максимальное время цикла один раз после завершения
    static bool print_once = true;
    if(print_once && !stepper_cycle_running()) {
        Serial.print("Finished cycle, max time=");
        Serial.println(cycle_time);

        // ошибки цикла
        if(stepper_cycle_error()) {
            Serial.print("Cycle error: ");
            print_cycle_error(stepper_cycle_error());
            Serial.println();
        }
        
        // ошибки моторов
        Serial.println("Motor errors:");
        Serial.print("X: ");
        print_motor_error(sm_x);
        Serial.println();
        Serial.print("Y: ");
        print_motor_error(sm_y);
        Serial.println();
        Serial.print("Z: ");
        print_motor_error(sm_z);
        Serial.println();
        
        print_once = false;
    }
    
    // put any code here, it would run while the motors are rotating
}
