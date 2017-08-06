/**
 * stepper.h
 *
 * Библиотека управления шаговыми моторами, подключенными через интерфейс 
 * драйвера "step-dir".
 *
 * LGPLv3, 2014-2016
 *
 * @author Антон Моисеев 1i7.livejournal.com
 */


#ifndef STEPPER_H
#define STEPPER_H

#define NO_PIN -1

#include "stddef.h"

/**
 * Стратегия определения границы движения координаты в одном из направлений:
 * - CONST: значение координаты задается константой в настройках мотора (min/max _pos)
 * - INF: не органичивать движение координаты в этом направлении (значение min/max _pos игнорируется,
 *       концевой датчик, если подключен, обрабатывается в любом случае)
 */
typedef enum {CONST, INF} end_strategy_t;

/**
 * Режим цикла вращения мотора
 */
typedef enum {
    /** Режим калибровки выключен */
    NONE,
    
    /** Калибровка начальной позиции (сбрасывать current_pos в min_pos при каждом шаге) */
    CALIBRATE_START_MIN_POS, 
    
    /** Калибровка границ рабочей области (устанавливать max_pos в current_pos при каждом шаге) */
    CALIBRATE_BOUNDS_MAX_POS
} calibrate_mode_t;

/**
 * Структура - шаговый двигатель.
 */
typedef struct {
    /** 
     * Имя шагового мотора (один символ: X, Y, Z и т.п.) 
     */
    char name;
    
    /*************************************************************/
    /* Подключение мотора к драйверу step-dir */
    /*************************************************************/
    
    /* Информация о подключение через драйвер Step-dir */
    
    /** 
     * Подача периодического импульса HIGH/LOW будет вращать мотор 
     */
    int pin_step;
    
    /** 
     * Направление вращения
     * 1 (HIGH): в одну сторону
     * 0 (LOW): в другую
     *
     * Для движения в сторону увеличения значения виртуальной координаты: 
     * при dir_inv=1: запись 1 (HIGH) в pin_dir
     * при dir_inv=-1: запись 0 (LOW) в pin_dir
     */
    int pin_dir;
    
    /** 
     * Вкл (0)/выкл (1) мотор 
     * -1 (NO_PIN): выход не подключен
     */
    int pin_en;
    
    /*************************************************************/
    /* Концевые датчики */
    /*************************************************************/
    
    /** 
     * Датчик на конце минимального положения текущей координаты;
     * -1 (NO_PIN): датчик не подключен
     */
    int pin_min;
    
    /** 
     * Датчик на конце максимального положения текущей координаты;
     * -1 (NO_PIN): датчик не подключен
     */
    int pin_max;
    
    /*************************************************************/
    /* Настройки подключения - характеристики мотора, драйвера и привода */
    /*************************************************************/
    
    /**
     * Инверсия направления вращения.
     *
     * Для движения в сторону увеличения значения виртуальной координаты: 
     * при dir_inv=1: запись 1 (HIGH) в pin_dir
     * при dir_inv=-1: запись 0 (LOW) в pin_dir
     */
    int dir_inv;
    
    /** 
     * Минимальная задержка между импульсами, микросекунды 
     * (для движения с максимальной скоростью)
     */
    int pulse_delay;
    
    /** 
     * Расстояние, проходимое координатой за один шаг мотора, 
     * базовая единица измерения мотора.
     * 
     * На основе значения distance_per_step счетчик шагов вычисляет
     * текущее положение рабочей координаты.
     * 
     * Единица измерения выбирается в зависимости от задачи и свойств
     * передаточного механизма.
     * 
     * Если брать базовую единицу изменения за нанометры (1/1000 микрометра), 
     * то диапазон значений для рабочей области будет от нуля в одну сторону:
     * 2^31=2147483648-1 нанометров * 7.5/1000/1000/1000=16метров
     * в обе строны: [16м, 16м], т.е. всего 32 метра.
     *
     * Для базовой единицы микрометр (микрон) рабочая область
     * от -16км до 16км, всего 32км.
     */
    long distance_per_step;
    
    /*************************************************************/
    /* Характеристики рабочей области */
    /*************************************************************/
    
    /**
     * Стратегия определения конечного положения для минимальной позиции координаты:
     * CONST/INF
     */
    end_strategy_t min_end_strategy;
    
    /**
     * Стратегия определения конечного положения для максимальной позиции координаты:
     * CONST/INF
     */
    end_strategy_t max_end_strategy;
    
    /** 
     * Минимальное значение положения координаты, базовая единица измерения мотора
     */
    long min_pos;
    
    /** 
     * Максимальное значение положения координаты, базовая единица измерения мотора
     */
    long max_pos;
    
    /*************************************************************/
    /* Информация о движении координаты, подключенной к мотору */
    /*************************************************************/
    
    /** 
     * Текущее положение координаты, базовая единица измерения мотора.
     *
     * Вычисляется и обновляется программно счетчиком шагов 
     * на основе значения distance_per_step.
     * 
     * При dir=1 координата возрастает, при dir=0 координата убывает.
     * 
     * Единица измерения выбирается в зависимости от задачи и свойств
     * передаточного механизма.
     * 
     * Если брать базовую единицу изменения за нанометры (1/1000 микрометра), 
     * то диапазон значений для рабочей области будет от нуля в одну сторону:
     * 2^31=2147483648-1 нанометров * 7.5/1000/1000/1000=16метров
     * в обе строны: [16м, 16м], т.е. всего 32 метра.
     *
     * Для базовой единицы микрометр (микрон) рабочая область
     * от -16км до 16км, всего 32км.
     */
    long current_pos;
} stepper;

/**
 * Статус цикла вращения мотора
 */
typedef enum {
    /** Ожидает запуска */
    STEPPER_STATUS_IDLE, 
    
    /** Вращается */
    STEPPER_STATUS_RUNNING, 
    
    /** Завершил вращение */
    STEPPER_STATUS_FINISHED
} stepper_status_t;

/**
 * Информация о цикле вращения шагового двигателя.
 */
typedef struct {
    stepper_status_t status = STEPPER_STATUS_IDLE;
    
    /** Завершил вращение из-за достижения виртуальной нижней границы */
    bool error_soft_end_min = false;
    /** Завершил вращение из-за достижения виртуальной верхней границы */
    bool error_soft_end_max = false;
    /** Завершил вращение из-за срабатывания концевого датчика нижней границы */
    bool error_hard_end_min = false;
    /** Завершил вращение из-за срабатывания концевого датчика верхней границы */
    bool error_hard_end_max = false;
    /** Слишком маленькая задержка между двумя импульсами для шага */
    bool error_pulse_delay_small = false;
} stepper_info_t;

/**
 * Глобальные ошибки цикла вращения моторов
 */
typedef enum {
    /** Ошибок нет */
    CYCLE_ERROR_NONE = 0,
    
    /** 
     * Хотябы у одного из моторов, добавленных в список вращения, 
     * минимальная задержка между шагами не вмещает 3 периода таймера
     * (следует проверить настройки мотора - значение pulse_delay или 
     * настройки частоты таймера цикла stepper_configure_timer).
     */
    CYCLE_ERROR_TIMER_PERIOD_TOO_LONG,
    
    /**
     * Период таймера некратен минимальной задержке между шагами
     * одного из моторов. Это может привести к тому, что при движении
     * на максимальной скорости минимальная задержка меджу шагами 
     * не будет соблюдаться, поэтому просто запретим такие комбинации:
     * см: https://github.com/1i7/stepper_h/issues/6
     */
    CYCLE_ERROR_TIMER_PERIOD_ALIQUANT_MOTOR_PULSE,
    
    /**
     * Проблема с мотором: выход за границы, некорректная задержка между 
     * шагами или что-то еще. Подробности см в статусе мотора.
     */
    CYCLE_ERROR_MOTOR_ERROR,
    
    /** 
     * Превышено максимальное время выполнения обработчика 
     * события от таймера 
     */
    CYCLE_ERROR_HANDLER_TIMING_EXCEEDED
} stepper_cycle_error_t;

typedef enum {
    /** Не менять текущее значение (при передаче параметра в настройки) */
    DONT_CHANGE,
    
    /** Игнорировать проблему, продолжать выполнение */
    IGNORE, 
    
    /** 
     * Попытаться исправить проблему (например, установить ближайшее корректное значение)
     * и продолжить выполнение 
     */
    FIX, 
    
    /** Остановить мотор, продолжить вращение остальных моторов */
    STOP_MOTOR,
    
    /** Завершить выполнение всего цикла - остановить все моторы */
    CANCEL_CYCLE
} error_handle_strategy_t;

/**
 * Инициализировать шаговый мотор необходимыми значениями.
 * 
 * @param smotor
 * @param name - Имя шагового мотора (один символ: X, Y, Z и т.п.) 
 * @param pin_step Подача периодического импульса HIGH/LOW будет вращать мотор 
 *     (шаг происходит по фронту HIGH > LOW)
 * @param pin_dir - Направление вращения
 *     1 (HIGH): в одну сторону
 *     0 (LOW): в другую
 *
 *     Для движения вправо (в сторону увеличения значения виртуальной координаты): 
 *     при invert_dir==false: запись 1 (HIGH) в pin_dir
 *     при invert_dir==true: запись 0 (LOW) в pin_dir
 * 
 * @param pin_en - вкл (0)/выкл (1) мотор 
 *     -1 (NO_PIN): выход не подключен
 * @param invert_dir - Инверсия направления вращения
 *     true: инвертировать направление вращения
 *     false: не инвертировать
 * @param pulse_delay - Минимальная задержка между импульсами, микросекунды
 *     (для движения с максимальной скоростью) 
 * @param distance_per_step - Расстояние, проходимое координатой за шаг,
 *     базовая единица измерения мотора.
 *     
 *     На основе значения distance_per_step счетчик шагов вычисляет
 *     текущее положение рабочей координаты.
 * 
 *     Единица измерения выбирается в зависимости от задачи и свойств
 *     передаточного механизма.
 * 
 *     Если брать базовую единицу изменения за нанометры (1/1000 микрометра), 
 *     то диапазон значений для рабочей области будет от нуля в одну сторону:
 *     2^31=2147483648-1 нанометров * 7.5/1000/1000/1000=16 метров
 *     в обе строны: [16м, 16м], т.е. всего 32 метра.
 *
 *     Для базовой единицы микрометр (микрон) рабочая область
 *     от -16км до 16км, всего 32км.
 */
void init_stepper(stepper* smotor,  char name, 
        int pin_step, int pin_dir, int pin_en,
        bool invert_dir, int pulse_delay,
        int distance_per_step);

/**
 * Задать настройки границ рабочей области для шагового мотора.
 * 
 * Примеры:
 * 1) область с заранее известными границами:
 *   init_stepper_ends(&sm_z, NO_PIN, NO_PIN, CONST, CONST, 0, 100000);
 * 
 * Движение влево ограничено значением min_pos, движение вправо ограничено значением max_pos
 * (min_pos<=curr_pos<=max_pos).
 * 
 * При калибровке начальной позиции мотора CALIBRATE_START_MIN_POS 
 * текущее положение мотора curr_pos сбрасывается в значение min_pos (curr_pos=min_pos)
 * на каждом шаге.
 *
 * При калибровке ширины рабочей области CALIBRATE_BOUNDS_MAX_POS 
 * текущее положение мотора curr_pos задает значение max_pos (max_pos=curr_pos)
 * на каждом шаге.
 *
 * 2) область с заранее известной позицией min_pos, значение max_pos не ограничено:
 *   init_stepper_ends(&sm_z, NO_PIN, NO_PIN, CONST, INF, 0, 100000);
 *
 * Движение влево ограничено начальной позицией min_pos (curr_pos не может стать меньше,
 * чем min_pos), движение вправо ничем не ограничено (curr_pos>=min_pos).
 * 
 * @param smotor
 * @param pin_min - номер пина для концевого датчика левой границы
 * @param pin_max - номер пина для концевого датчика правой границы
 * @param min_end_strategy - тип левой виртуальной границы:
 *     CONST - константа, фиксированное минимальное значение координаты
 *     INF - ограничения нет
 * @param max_end_strategy - тип правой виртуальной границы:
 *     CONST - константа, фиксированное максимальное значение координаты
 *     INF - ограничения нет
 * @param min_pos - минимальное значение координаты (для min_end_strategy=CONST)
 * @param max_pos - максимальное значение координаты (для max_end_strategy=CONST)
 */
void init_stepper_ends(stepper* smotor,
        int pin_min, int pin_max,
        end_strategy_t min_end_strategy, end_strategy_t max_end_strategy,
        long min_pos, long max_pos);

/**
 * Подготовить мотор к запуску ограниченной серии шагов - задать нужное количество 
 * шагов и задержку между шагами для регулирования скорости (0 для максимальной скорости).
 * 
 * @param step_count - количество шагов, знак задает направление вращения
 * @param step_delay - задержка между двумя шагами, микросекунды (0 для максимальной скорости)
 * @param calibrate_mode - режим калибровки
 *     NONE: режим калибровки выключен - останавливать вращение при выходе за виртуальные границы 
 *           рабочей области [min_pos, max_pos] (аппаратные проверяются ВСЕГДА);
 *     CALIBRATE_START_MIN_POS: установка начальной позиции (сбрасывать current_pos в min_pos при каждом шаге);
 *     CALIBRATE_BOUNDS_MAX_POS: установка размеров рабочей области (сбрасывать max_pos в current_pos при каждом шаге)
 * @param stepper_info - информация о цикле вращения шагового двигателя, обновляется динамически
 *        в процессе вращения двигателя
 */
void prepare_steps(stepper *smotor, long step_count, unsigned long step_delay, calibrate_mode_t calibrate_mode=NONE, 
        stepper_info_t *stepper_info=NULL);

/**
 * Подготовить мотор к запуску на беспрерывное вращение - задать направление и задержку между
 * шагами для регулирования скорости (0 для максимальной скорости).
 *
 * Мотор будет вращаться до тех пор, пока не будет вручную остановлен вызовом finish_stepper_cycle()
 *
 * @param dir - направление вращения: 1 - вращать вперед, -1 - назад.
 * @param step_delay - задержка между двумя шагами, микросекунды (0 для максимальной скорости).
 * @param calibrate_mode - режим калибровки
 *     NONE: режим калибровки выключен - останавливать вращение при выходе за виртуальные границы 
 *           рабочей области [min_pos, max_pos] (аппаратные проверяются ВСЕГДА);
 *     CALIBRATE_START_MIN_POS: установка начальной позиции (сбрасывать current_pos в min_pos при каждом шаге);
 *     CALIBRATE_BOUNDS_MAX_POS: установка размеров рабочей области (сбрасывать max_pos в current_pos при каждом шаге)
 * @param stepper_info информация о цикле вращения шагового двигателя, обновляется динамически
 *        в процессе вращения двигателя
 */
void prepare_whirl(stepper *smotor, int dir, unsigned long step_delay, calibrate_mode_t calibrate_mode=NONE, 
        stepper_info_t *stepper_info=NULL);

/**
 * Подготовить мотор к запуску ограниченной серии шагов с переменной скоростью - задержки на каждом 
 * шаге вычисляются заранее, передаются в буфере delay_buffer.
 * 
 * Масштабирование шага позволяет экономить место в буфере delay_buffer, жертвуя точностью 
 * (минимальной длиной шага в цикле); если цикл содержит серии шагов с одинаковой задержкой,
 * реальная точность не пострадает. Буфер delay_buffer содержит временные задержки перед каждым следующим шагом.
 * Можно использовать одну и ту же задержку (один элемент буфера) для нескольких последовательных шагов
 * при помощи параметра step_count (масштаб).
 * 
 * При step_count=1 на каждый элемент буфера delay_buffer ("виртуальный" шаг) мотор будет делать 
 *     один реальный (аппаратный) шаг из delay_buffer.
 * При step_count=2 на каждый элемент буфера delay_buffer (виртуальный шаг) мотор будет делать 
 *     два реальных (аппаратных) шага с одной и той же задержкой из delay_buffer.
 * При step_count=3 на каждый элемент буфера delay_buffer (виртуальный шаг) мотор будет делать 
 *     три реальных (аппаратных) шага с одной и той же задержкой из delay_buffer.
 * 
 * Допустим, в delay_buffer 2 элемента (2 виртуальных шага):
 *     delay_buffer[0]=1000
 *     delay_buffer[1]=2000
 * параметр step_count=3
 * 
 * Мотор сделает 3 аппаратных шага с задержкой delay_buffer[0]=1000 мкс перед каждым шагом и 
 * 3 аппаратных шага с задержкой delay_buffer[1]=2000мкс. Всего 2*3=6 аппаратных шагов, 
 * время на все шаги = 1000*3+2000*3=3000+6000=9000мкс
 * 
 * Значение параметра buf_size указываем 2 (количество элементов в буфере delay_buffer).
 *
 * Аналогичный результат можно достигнуть с delay_buffer[6]
 *     delay_buffer[0]=1000
 *     delay_buffer[1]=1000
 *     delay_buffer[2]=1000
 *     delay_buffer[3]=2000
 *     delay_buffer[4]=2000
 *     delay_buffer[5]=2000
 * step_count=1, buf_size=6
 *
 * Количество аппаратных шагов можно вычислять как buf_size*step_count.
 * 
 * @param buf_size - количество элементов в буфере delay_buffer (количество виртуальных шагов)
 * @param delay_buffer - массив задержек перед каждым следующим шагом, микросекунды
 * @param step_count - масштабирование шага - количество аппаратных шагов мотора в одном 
 *     виртуальном шаге, знак задает направление вращения мотора.
 * Значение по умолчанию step_count=1: виртуальные шаги соответствуют аппаратным
 * @param stepper_info - информация о цикле вращения шагового двигателя, обновляется динамически
 *        в процессе вращения двигателя
 */
void prepare_simple_buffered_steps(stepper *smotor, int buf_size, unsigned long* delay_buffer, long step_count=1, 
        stepper_info_t *stepper_info=NULL);

/**
 * @param buf_size - количество элементов в буфере delay_buffer
 * @param delay_buffer - (step delay buffer) - массив задержек перед каждым следующим шагом, микросекунды
 * @param step_buffer - (step count buffer) - массив с количеством шагов для каждого 
 *     значения задержки из delay_buffer. Может содержать положительные и отрицательные значения,
 *     знак задает направление вращения мотора. 
 *     Должен содержать ровно столько же элементов, сколько delay_buffer
 * @param stepper_info - информация о цикле вращения шагового двигателя, обновляется динамически
 *        в процессе вращения двигателя
 */
void prepare_buffered_steps(stepper *smotor, int buf_size, unsigned long* delay_buffer, long* step_buffer, 
        stepper_info_t *stepper_info=NULL);

/**
 * Подготовить мотор к запуску ограниченной серии шагов с переменной скоростью - задать нужное количество 
 * шагов и указатель на функцию, вычисляющую задержку перед каждым шагом для регулирования скорости.
 * 
 * @param step_count - количество шагов, знак задает направление вращения
 * @param curve_context - указатель на объект, содержащий всю необходимую информацию для вычисления
 *     времени до следующего шага
 * @param next_step_delay - указатель на функцию, вычисляющую задержку перед следующим шагом, микросекунды
 * @param stepper_info - информация о цикле вращения шагового двигателя, обновляется динамически
 *        в процессе вращения двигателя
 */
void prepare_dynamic_steps(stepper *smotor, long step_count, 
        void* curve_context, unsigned long (*next_step_delay)(int curr_step, void* curve_context), 
        stepper_info_t *stepper_info=NULL);
        
/**
 * Подготовить мотор к запуску на беспрерывное вращение с переменной скоростью - задать нужное количество 
 * шагов и указатель на функцию, вычисляющую задержку перед каждым шагом для регулирования скорости.
 * 
 * @param dir - направление вращения: 1 - вращать вперед, -1 - назад.
 * @param curve_context - указатель на объект, содержащий всю необходимую информацию для вычисления
 *     времени до следующего шага
 * @param next_step_delay - указатель на функцию, вычисляющую задержку перед следующим шагом, микросекунды
 * @param stepper_info - информация о цикле вращения шагового двигателя, обновляется динамически
 *        в процессе вращения двигателя
 */
void prepare_dynamic_whirl(stepper *smotor, int dir, 
        void* curve_context, unsigned long (*next_step_delay)(int curr_step, void* curve_context), 
        stepper_info_t *stepper_info=NULL);


//////////////////////////////////////////
// Управление циклом

/**
 * Запустить цикл шагов на выполнение - запускаем таймер с
 * обработчиком прерываний отрабатывать подготовленную программу.
 *
 * @return
 *     true - цикл запущен
 *     false - цикл не запущен, т.к. предыдущий цикл еще не завершен
 */
bool stepper_start_cycle();

/**
 * Завершить цикл шагов - остановить таймер, обнулить список моторов.
 */
void stepper_finish_cycle();

/**
 * Поставить вращение на паузу, не прирывая всего цикла
 */
void stepper_pause_cycle();

/**
 * Продолжить вращение, если оно было поставлено на паузу
 */
void stepper_resume_cycle();

/**
 * Текущий статус цикла:
 * true - в процессе выполнения,
 * false - ожидает запуска.
 */
bool stepper_cycle_running();

/**
 * Проверить, на паузе ли цикл:
 * true - цикл на паузе (выполняется)
 * false - цикл не на паузе (выполняется или остановлен).
 */
bool stepper_cycle_paused();

/**
 * Код ошибки цикла.
 * @return статус ошибки из перечисления stepper_cycle_error_t
 *     CYCLE_ERROR_NONE (== 0) - ошибки нет
 *     >0 - код ошибки из перечисления stepper_cycle_error_t
 */
stepper_cycle_error_t stepper_cycle_error_status();

/////////////////////////////////////////
// Системные настройки

/**
 * Настроить таймер для шагов.
 * Частота ядра PIC32MX - 80МГц=80млн операций в секунду.
 * Берем базовый предварительный масштаб таймера 
 * (например, TIMER_PRESCALER_1_8), дальше подбираем 
 * частоту под нужный период
 * 
 * Example: to set timer clock period to 20ms (50 operations per second)
 * use prescaler 1:64 (0x0060) and period=0x61A8:
 * 80000000/64/50=25000=0x61A8
 * 
 * для периода 1 микросекунда (1млн вызовов в секунду):
 * // (уже подглючивает)
 * 80000000/8/1000000=10=0xA
 *   target_period_us = 1
 *   prescalar = TIMER_PRESCALER_1_8 = 8
 *   period = 10
 * 
 * для периода 5 микросекунд (200тыс вызовов в секунду):
 * 80000000/8/1000000=10
 *   target_period_us = 5
 *   prescalar = TIMER_PRESCALER_1_8 = 8
 *   period = 50
 * 
 * для периода 10 микросекунд (100тыс вызовов в секунду):
 * // ок для движения по линии, совсем не ок для движения по дуге (по 90мкс на acos/asin)
 * 80000000/8/100000=100=0x64
 *   target_period_us = 10
 *   prescalar = TIMER_PRESCALER_1_8 = 8
 *   period = 100
 *
 * для периода 20 микросекунд (50тыс вызовов в секунду):
 * 80000000/8/50000=200
 *   target_period_us = 20
 *   prescalar = TIMER_PRESCALER_1_8 = 8
 *   period = 200
 *
 * для периода 80 микросекунд (12.5тыс вызовов в секунду):
 * 80000000/8/12500=200
 *   target_period_us = 80
 *   prescalar = TIMER_PRESCALER_1_8 = 8
 *   period = 800
 *
 * для периода 100 микросекунд (10тыс вызовов в секунду):
 * 80000000/8/10000=1000
 *   target_period_us = 100
 *   prescalar = TIMER_PRESCALER_1_8 = 8
 *   period = 1000
 *
 * для периода 200 микросекунд (5тыс вызовов в секунду):
 * // ок для движения по дуге (по 90мкс на acos/asin)
 * 80000000/8/5000=2000
 *   target_period_us = 200
 *   prescalar = TIMER_PRESCALER_1_8 = 8
 *   period = 2000
 *
 * @param target_period_us - целевой период таймера, микросекунды.
 * @param timer - системный идентификатор таймера (должен поддерживаться аппаратно)
 * @param prescalar - предварительный масштаб таймера
 * @param period - значение для периода таймера: делитель частоты таймера
 *     после того, как к ней применен предварительный масштаб (prescaler)
 */
void stepper_configure_timer(int target_period_us, int timer, int prescaler, int period);

/**
 * Стратегия реакции на некоторые исключительные ситуации, которые
 * могут произойти во время вращения моторов.
 *
 * @param hard_end_handle - выход за границы по аппаратному концевику.
 *     допустимые значения: STOP_MOTOR/CANCEL_CYCLE
 *     по умолчанию: CANCEL_CYCLE
 * @param soft_end_handle - выход за виртуальные границы.
 *     допустимые значения: STOP_MOTOR/CANCEL_CYCLE
 *     по умолчанию: CANCEL_CYCLE
 * @param small_pulse_delay_handle - задержка между шагами меньше 
 *       минимально допустимой для мотора.
 *     допустимые значения: FIX/STOP_MOTOR/CANCEL_CYCLE
 *     по умолчанию: CANCEL_CYCLE
 * @param cycle_timing_exceed_handle - обработчик прерывания выполняется дольше,
 *       чем период таймера.
 *     допустимые значения: IGNORE/CANCEL_CYCLE
 *     по умолчанию: CANCEL_CYCLE
 */
void stepper_set_error_handle_strategy(
        error_handle_strategy_t hard_end_handle,
        error_handle_strategy_t soft_end_handle,
        error_handle_strategy_t small_pulse_delay_handle,
        error_handle_strategy_t cycle_timing_exceed_handle);

#endif // STEPPER_H

