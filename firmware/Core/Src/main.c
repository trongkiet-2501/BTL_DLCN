/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include <stdlib.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "lidarvl53.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
typedef struct
{
    float x1;
    float y1;
    float x2;
    float y2;
    float a;
    float b;
    uint8_t p1_ok;
    uint8_t p2_ok;
    uint8_t calib_ok;
} LinearCalib_t;

#define UART_CMD_BUFFER_SIZE 100

static uint8_t uart_rx_byte = 0;

static char uart_rx_buffer[UART_CMD_BUFFER_SIZE];
static char uart_cmd_buffer[UART_CMD_BUFFER_SIZE];

static volatile uint16_t uart_rx_index = 0;
static volatile uint8_t uart_cmd_ready = 0;

static volatile uint8_t stream_enable = 0;

static LinearCalib_t calib_thermistor    = {0, 0, 0, 0, 1.0f, 0.0f, 0, 0, 0};
static LinearCalib_t calib_potentiometer = {0, 0, 0, 0, 1.0f, 0.0f, 0, 0, 0};
static LinearCalib_t calib_ultrasonic    = {0, 0, 0, 0, 1.0f, 0.0f, 0, 0, 0};
static LinearCalib_t calib_laser         = {0, 0, 0, 0, 1.0f, 0.0f, 0, 0, 0};

static volatile uint8_t flag_10ms = 0;
static volatile uint8_t flag_50ms = 0;
static volatile uint8_t flag_100ms = 0;
static volatile uint16_t timer_counter = 0;

static volatile uint16_t laser_raw_mm_last, l_int, l_frac = 0;
static volatile uint16_t laser_calib_mm_last = 0;
/* =========================
   SENSOR DATA STRUCT
   ========================= */
typedef struct
{
    float thermistor_degC;
    float potentiometer_deg;
    float ultrasonic_cm;
    float laser_mm;
} SensorData_t;

static SensorData_t sensor_data;

/* =========================
   ULTRASONIC INPUT CAPTURE
   ========================= */
static volatile uint32_t ultrasonic_rise_tick = 0;
static volatile uint32_t ultrasonic_fall_tick = 0;
static volatile uint32_t ultrasonic_echo_us = 0;
static volatile uint8_t ultrasonic_capture_state = 0;
static volatile uint8_t ultrasonic_data_ready = 0;

/* =========================
   SAMPLE ACCUMULATOR
   Lay mau roi cong don de tinh trung binh
   ========================= */
static float thermistor_sum = 0.0f;
static float potentiometer_sum = 0.0f;
static float laser_sum = 0.0f;
static float ultrasonic_sum = 0.0f;
static float ultrasonic_calib_a = 1.0f;
static float ultrasonic_calib_b = 0.0f;

static uint8_t thermistor_sample_count = 0;
static uint8_t potentiometer_sample_count = 0;
static uint8_t laser_sample_count = 0;
static uint8_t ultrasonic_sample_count = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint32_t last_send_time = 0;

// Giả lập data
static int thermistor_x10 = 253;    // 25.3
static int laser_x10 = 105;         // 10.5
static int potentiometer = 45;      // 45 %
static int ultrasonic = 120;        // 120 cm

static const uint16_t lookup_raw[]  = {0, 132, 240, 331, 424, 534, 620, 709, 806, 893, 976};
static const uint16_t lookup_true[] = {0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

#define LOOKUP_SIZE (sizeof(lookup_raw) / sizeof(lookup_raw[0]))

float Apply_Calib(LinearCalib_t *calib, float raw_value)
{
    if (raw_value < 0.0f)
    {
        return raw_value;
    }

    if (calib->calib_ok)
    {
        return calib->a * raw_value + calib->b;
    }

    return raw_value;
}

uint8_t Compute_Linear_Calib(LinearCalib_t *calib)
{
    float dx = calib->x2 - calib->x1;

    if (fabsf(dx) < 0.0001f)
    {
        return 0;
    }

    calib->a = (calib->y2 - calib->y1) / dx;
    calib->b = calib->y1 - calib->a * calib->x1;
    calib->calib_ok = 1;

    return 1;
}

uint16_t get_calibrated_distance(uint16_t raw_mm)
{
    if (raw_mm <= lookup_raw[0])
    {
        return lookup_true[0];
    }

    if (raw_mm >= lookup_raw[LOOKUP_SIZE - 1])
    {
        uint16_t dx = lookup_raw[LOOKUP_SIZE - 1] - lookup_raw[LOOKUP_SIZE - 2];
        uint16_t dy = lookup_true[LOOKUP_SIZE - 1] - lookup_true[LOOKUP_SIZE - 2];

        if (dx == 0)
        {
            return lookup_true[LOOKUP_SIZE - 1];
        }

        return lookup_true[LOOKUP_SIZE - 1] +
               (uint32_t)(raw_mm - lookup_raw[LOOKUP_SIZE - 1]) * dy / dx;
    }

    for (int i = 0; i < LOOKUP_SIZE - 1; i++)
    {
        if (raw_mm >= lookup_raw[i] && raw_mm <= lookup_raw[i + 1])
        {
            uint16_t dx = lookup_raw[i + 1] - lookup_raw[i];
            uint16_t dy = lookup_true[i + 1] - lookup_true[i];

            if (dx == 0)
            {
                return lookup_true[i];
            }

            return lookup_true[i] +
                   (uint32_t)(raw_mm - lookup_raw[i]) * dy / dx;
        }
    }

    return raw_mm;
}

void Update_Fake_Sensor_Data(void)
{
    thermistor_x10 += 1;     // tăng 0.1
    laser_x10 += 2;          // tăng 0.2
    potentiometer += 1;
    ultrasonic += 1;

    if (thermistor_x10 > 350) thermistor_x10 = 253;
    if (laser_x10 > 300) laser_x10 = 105;
    if (potentiometer > 100) potentiometer = 0;
    if (ultrasonic > 200) ultrasonic = 120;
}

void Send_Sensor_Data_USB(void)
{
    char tx_buffer[128];

    int len = snprintf(tx_buffer, sizeof(tx_buffer),
                       "THERMISTOR:%d.%d;LASER:%d.%d;POTENTIOMETER:%d;ULTRASONIC:%d\r\n",
                       thermistor_x10 / 10,
                       thermistor_x10 % 10,
                       laser_x10 / 10,
                       laser_x10 % 10,
                       potentiometer,
                       ultrasonic);

    if (len > 0)
    {
        if (len % 64 == 0) {
            tx_buffer[len] = ' ';
            tx_buffer[len+1] = '\0';
            len++;
        }
        CDC_Transmit_FS((uint8_t*)tx_buffer, len);
    }
}

/* =========================
   HAM GIOI HAN GIA TRI
   ========================= */
static float Clamp_Float(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/* =========================
   DOC ADC DUNG CHUNG
   ========================= */
uint16_t Read_ADC_Value(ADC_HandleTypeDef *hadc)
{
    uint16_t adc_value = 0;

    HAL_ADC_Start(hadc);

    if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
    {
        adc_value = HAL_ADC_GetValue(hadc);
    }

    HAL_ADC_Stop(hadc);

    return adc_value;
}

/* =========================
   1. DOC THERMISTOR
   ADC1 PA0 -> Nhiet do
   ========================= */
float Read_Thermistor_Raw_degC(void)
{
    uint16_t adc_raw = Read_ADC_Value(&hadc1);

    const float Vref = 3.3f;
    const float Vmin = 0.1f;
    const float Vmax = 3.1f;

    float Vout = adc_raw * Vref / 4095.0f;

    if (Vout < Vmin || Vout > Vmax)
    {
        return -999.0f;
    }

    float temp_degC = (Vout - Vmin) * 100.0f / (Vmax - Vmin);

    return temp_degC;
}
float Read_Thermistor_degC(void)
{
    float raw = Read_Thermistor_Raw_degC();
    return Apply_Calib(&calib_thermistor, raw);
}
/* =========================
   2. DOC POTENTIOMETER
   ADC2 PA1 -> Goc 0-300 do
   ========================= */
float Read_Potentiometer_Raw_deg(void)
{
    uint16_t adc_raw = Read_ADC_Value(&hadc2);

    const float Vref = 3.3f;

    /*
       Neu mach scale cua ban la 0.1V den 3.1V
       tuong ung goc 0 den 300 do
    */
    const float Vmin = 0.1f;
    const float Vmax = 3.1f;

    float Vout = adc_raw * Vref / 4095.0f;

    Vout = Clamp_Float(Vout, Vmin, Vmax);

    float angle_deg = (Vout - Vmin) * 300.0f / (Vmax - Vmin);

    return angle_deg;
}
float Read_Potentiometer_deg(void)
{
    float raw = Read_Potentiometer_Raw_deg();
    return Apply_Calib(&calib_potentiometer, raw);
}

float Read_Ultrasonic_Raw_cm(float tempC)
{
    ultrasonic_data_ready = 0;
    ultrasonic_echo_us = 0;

    Ultrasonic_Trigger();

    uint32_t start_tick = HAL_GetTick();

    while (!ultrasonic_data_ready)
    {
        if (HAL_GetTick() - start_tick > 30)
        {
            return -1.0f;
        }
    }

    float speed_cm_us = (331.4f + 0.6f * tempC) * 100.0f / 1000000.0f;
    float distance_cm = ultrasonic_echo_us * speed_cm_us / 2.0f;

    return distance_cm;
}


/* =========================
   DELAY US DUNG TIM4
   Luu y: TIM4 nen cau hinh Prescaler = 72 - 1 de 1 tick = 1us
   ========================= */
void Delay_us_TIM4(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim4, 0);

    while (__HAL_TIM_GET_COUNTER(&htim4) < us)
    {
        // wait
    }
}

/* =========================
   PHAT XUNG TRIG CHO HC-SR04
   TRIG = PB6
   ECHO = PB7 / TIM4_CH2
   ========================= */
void Ultrasonic_Trigger(void)
{
    ultrasonic_data_ready = 0;
    ultrasonic_capture_state = 0;
    ultrasonic_echo_us = 0;

    __HAL_TIM_SET_COUNTER(&htim4, 0);

    __HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    Delay_us_TIM4(2);

    // phát xung trig 10us
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    Delay_us_TIM4(10);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
}

/* =========================
   3. DOC ULTRASONIC
   Echo time -> distance cm
   ========================= */
float Read_Ultrasonic_cm(float tempC)
{
    float raw = Read_Ultrasonic_Raw_cm(tempC);
    return Apply_Calib(&calib_ultrasonic, raw);
}
void Calib_Ultrasonic(float a, float b)
{
    ultrasonic_calib_a = a;
    ultrasonic_calib_b = b;
}
/* =========================
   4. DOC LASER VL53L0X
   Luu y: VL53L0X can driver rieng.
   Ham nay hien tai moi kiem tra co cam bien I2C hay chua.
   ========================= */
void Test_Laser_Raw_UART(void)
{
    char tx[120];

    uint16_t raw_mm = lidar_lee_mm(dir_s1);

    snprintf(tx, sizeof(tx),
             "TEST_LASER_RAW:%u\r\n",
             raw_mm);

    UART_Send_Text(tx);
}
float Read_Laser_Raw_mm(void)
{
    uint16_t raw_mm = lidar_lee_mm(dir_s1);

    laser_raw_mm_last = raw_mm;

    if (raw_mm == 0 || raw_mm == 8190 || raw_mm > 2000)
    {
        return -1.0f;
    }

    return (float)raw_mm;
}

float Read_Laser_VL53L0X_mm(void)
{
    float raw = Read_Laser_Raw_mm();

    if (raw < 0.0f)
    {
        laser_calib_mm_last = 0;
        return -1.0f;
    }

    float calib_value = Apply_Calib(&calib_laser, raw);

    laser_calib_mm_last = (uint16_t)calib_value;

    return calib_value;
}





typedef enum
{
    SENSOR_UNKNOWN = 0,
    SENSOR_THERMISTOR,
    SENSOR_POTENTIOMETER,
    SENSOR_ULTRASONIC,
    SENSOR_LASER
} SensorId_t;


SensorId_t Parse_Sensor_Name(char *name)
{
    if (strcmp(name, "THERMISTOR") == 0) return SENSOR_THERMISTOR;
    if (strcmp(name, "POTENTIOMETER") == 0) return SENSOR_POTENTIOMETER;
    if (strcmp(name, "ULTRASONIC") == 0) return SENSOR_ULTRASONIC;
    if (strcmp(name, "LASER") == 0) return SENSOR_LASER;

    return SENSOR_UNKNOWN;
}

LinearCalib_t* Get_Calib_By_Sensor(SensorId_t sensor)
{
    switch (sensor)
    {
        case SENSOR_THERMISTOR:    return &calib_thermistor;
        case SENSOR_POTENTIOMETER: return &calib_potentiometer;
        case SENSOR_ULTRASONIC:    return &calib_ultrasonic;
        case SENSOR_LASER:         return &calib_laser;
        default:                   return NULL;
    }
}

float Read_Sensor_Raw_For_Calib(SensorId_t sensor)
{
    switch (sensor)
    {
        case SENSOR_THERMISTOR:
            return Read_Thermistor_Raw_degC();

        case SENSOR_POTENTIOMETER:
            return Read_Potentiometer_Raw_deg();

        case SENSOR_LASER:
            return Read_Laser_Raw_mm();

        case SENSOR_ULTRASONIC:
        {
            float tempC = Read_Thermistor_Raw_degC();

            if (tempC < -100.0f)
            {
                tempC = 25.0f;
            }

            return Read_Ultrasonic_Raw_cm(tempC);
        }

        default:
            return -999.0f;
    }
}
void UART_Send_Text(const char *text)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)text, strlen(text), 100);
}
const char* SensorId_To_String(SensorId_t sensor)
{
    switch (sensor)
    {
        case SENSOR_THERMISTOR:    return "THERMISTOR";
        case SENSOR_POTENTIOMETER: return "POTENTIOMETER";
        case SENSOR_ULTRASONIC:    return "ULTRASONIC";
        case SENSOR_LASER:         return "LASER";
        default:                   return "UNKNOWN";
    }
}
void UART_Send_SetPoint1(SensorId_t sensor, float raw, float ref)
{
    char tx[160];
    const char *sensor_name = SensorId_To_String(sensor);

    long raw_x100 = lroundf(raw * 100.0f);
    long ref_x100 = lroundf(ref * 100.0f);

    snprintf(tx, sizeof(tx),
             "OK,SETP1,%s,RAW:%ld.%02ld,REF:%ld.%02ld\r\n",
             sensor_name,
             raw_x100 / 100,
             labs(raw_x100 % 100),
             ref_x100 / 100,
             labs(ref_x100 % 100));

    UART_Send_Text(tx);
}

void UART_Send_SetPoint2(SensorId_t sensor, float raw, float ref, float a, float b)
{
    char tx[180];
    const char *sensor_name = SensorId_To_String(sensor);

    long raw_x100 = lroundf(raw * 100.0f);
    long ref_x100 = lroundf(ref * 100.0f);
    long a_x1000000 = lroundf(a * 1000000.0f);
    long b_x1000000 = lroundf(b * 1000000.0f);

    snprintf(tx, sizeof(tx),
             "OK,SETP2,%s,RAW:%ld.%02ld,REF:%ld.%02ld,A:%ld.%06ld,B:%ld.%06ld\r\n",
             sensor_name,
             raw_x100 / 100,
             labs(raw_x100 % 100),
             ref_x100 / 100,
             labs(ref_x100 % 100),
             a_x1000000 / 1000000,
             labs(a_x1000000 % 1000000),
             b_x1000000 / 1000000,
             labs(b_x1000000 % 1000000));

    UART_Send_Text(tx);
}

uint8_t Is_Raw_Value_Valid(SensorId_t sensor, float raw)
{
    switch (sensor)
    {
        case SENSOR_THERMISTOR:
            return raw > -100.0f;

        case SENSOR_POTENTIOMETER:
            return raw >= 0.0f;

        case SENSOR_ULTRASONIC:
            return raw > 0.0f;

        case SENSOR_LASER:
            return raw > 0.0f;

        default:
            return 0;
    }
}

void Handle_SetPoint(uint8_t point, SensorId_t sensor, float ref_value)
{
    LinearCalib_t *calib = Get_Calib_By_Sensor(sensor);

    if (calib == NULL)
    {
        UART_Send_Text("ERR,UNKNOWN_SENSOR\r\n");
        return;
    }

    float raw_value = Read_Sensor_Raw_For_Calib(sensor);

    if (!Is_Raw_Value_Valid(sensor, raw_value))
    {
        UART_Send_Text("ERR,READ_SENSOR_FAIL\r\n");
        return;
    }

    if (point == 1)
    {
        calib->x1 = raw_value;
        calib->y1 = ref_value;
        calib->p1_ok = 1;
        calib->p2_ok = 0;
        calib->calib_ok = 0;

        UART_Send_SetPoint1(sensor, raw_value, ref_value);
    }
    else if (point == 2)
    {
        if (!calib->p1_ok)
        {
            UART_Send_Text("ERR,NEED_SETP1_FIRST\r\n");
            return;
        }

        calib->x2 = raw_value;
        calib->y2 = ref_value;
        calib->p2_ok = 1;

        if (Compute_Linear_Calib(calib))
        {
            UART_Send_SetPoint2(sensor, raw_value, ref_value, calib->a, calib->b);
        }
        else
        {
            UART_Send_Text("ERR,CALIB_DX_ZERO\r\n");
        }
    }
}

void Test_VL53_ID_UART(void)
 {
     char tx[120];
     uint8_t model_id = 0;
     uint8_t module_type = 0;
     uint8_t revision_id = 0;

     HAL_StatusTypeDef s1 = HAL_I2C_Mem_Read(
         &hi2c1,
         0x52,
         0xC0,
         I2C_MEMADD_SIZE_8BIT,
         &model_id,
         1,
         100
     );

     HAL_StatusTypeDef s2 = HAL_I2C_Mem_Read(
         &hi2c1,
         0x52,
         0xC1,
         I2C_MEMADD_SIZE_8BIT,
         &module_type,
         1,
         100
     );

     HAL_StatusTypeDef s3 = HAL_I2C_Mem_Read(
         &hi2c1,
         0x52,
         0xC2,
         I2C_MEMADD_SIZE_8BIT,
         &revision_id,
         1,
         100
     );

     snprintf(tx, sizeof(tx),
              "VL53_ID S:%d,%d,%d MODEL:0x%02X MODULE:0x%02X REV:0x%02X\r\n",
              s1, s2, s3,
              model_id,
              module_type,
              revision_id);

     UART_Send_Text(tx);
 }

void Process_Command(char *cmd)
{
    char *token1;
    char *token2;
    char *token3;

    token1 = strtok(cmd, ",");

    if (token1 == NULL)
    {
        return;
    }

    if (strcmp(token1, "START") == 0)
    {
        stream_enable = 1;
        UART_Send_Text("OK,START\r\n");
        return;
    }

    if (strcmp(token1, "STOP") == 0)
    {
        stream_enable = 0;
        UART_Send_Text("OK,STOP\r\n");
        return;
    }
    if (strcmp(token1, "TESTLASER") == 0)
    {
        Test_Laser_Raw_UART();
        return;
    }
    token2 = strtok(NULL, ",");
    token3 = strtok(NULL, ",");

    if (token2 == NULL || token3 == NULL)
    {
        UART_Send_Text("ERR,BAD_FORMAT\r\n");
        return;
    }

    SensorId_t sensor = Parse_Sensor_Name(token2);
    float ref_value = atof(token3);

    if (strcmp(token1, "SETP1") == 0)
    {
        Handle_SetPoint(1, sensor, ref_value);
    }
    else if (strcmp(token1, "SETP2") == 0)
    {
        Handle_SetPoint(2, sensor, ref_value);
    }
    else
    {
        UART_Send_Text("ERR,UNKNOWN_CMD\r\n");
    }
    if (strcmp(token1, "TESTID") == 0)
    {
        Test_VL53_ID_UART();
        return;
    }
}



/* =========================
   GUI DU LIEU 4 CAM BIEN DA XU LY
   USB CDC
   ========================= */
void Send_Real_Sensor_Data_UART(void)
{
    char tx_buffer[160];

    long t_x10 = lroundf(sensor_data.thermistor_degC * 10.0f);
    long l_x10 = lroundf(sensor_data.laser_mm * 10.0f);
    long p_x10 = lroundf(sensor_data.potentiometer_deg * 10.0f);
    long u_x10 = lroundf(sensor_data.ultrasonic_cm * 10.0f);

    long t_int = t_x10 / 10;
    long t_frac = labs(t_x10 % 10);

    long l_int = l_x10 / 10;
    long l_frac = labs(l_x10 % 10);

    long p_int = p_x10 / 10;
    long p_frac = labs(p_x10 % 10);

    long u_int = u_x10 / 10;
    long u_frac = labs(u_x10 % 10);

    int len = snprintf(tx_buffer, sizeof(tx_buffer),
                       "THERMISTOR:%ld.%01ld;LASER:%ld.%01ld;POTENTIOMETER:%ld.%01ld;ULTRASONIC:%ld.%01ld\r\n",
                       t_int, t_frac,
                       l_int, l_frac,
                       p_int, p_frac,
                       u_int, u_frac);

    if (len > 0 && len < sizeof(tx_buffer))
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)tx_buffer, len, 100);
    }
}



/* =========================
   RESET BO DEM MAU
   Goi sau khi da tinh trung binh va gui USB
   ========================= */
void Reset_Sensor_Sample_Buffer(void)
{
    thermistor_sum = 0.0f;
    potentiometer_sum = 0.0f;
    laser_sum = 0.0f;
    ultrasonic_sum = 0.0f;

    thermistor_sample_count = 0;
    potentiometer_sample_count = 0;
    laser_sample_count = 0;
    ultrasonic_sample_count = 0;
}

/* =========================
   TASK 10ms
   Moi 10ms lay 1 mau:
   - Thermistor
   - Potentiometer
   - Laser
   ========================= */
void Sensor_Task_10ms(void)
{
    float thermistor_value = Read_Thermistor_degC();

    if (thermistor_value > -100.0f)
    {
        thermistor_sum += thermistor_value;
        thermistor_sample_count++;
    }

    float potentiometer_value = Read_Potentiometer_deg();

    potentiometer_sum += potentiometer_value;
    potentiometer_sample_count++;
}
/* =========================
   TASK 50ms
   Moi 50ms lay 1 mau ultrasonic
   ========================= */
void Sensor_Task_50ms(void)
{
    /*
       Dung nhiet do gan nhat de bu van toc am thanh.
       Neu chua co mau thermistor hop le thi tam dung 25 do C.
    */
    float tempC = 25.0f;

    if (thermistor_sample_count > 0)
    {
        tempC = thermistor_sum / thermistor_sample_count;
    }

    float ultrasonic_value = Read_Ultrasonic_cm(tempC);

    if (ultrasonic_value > 0.0f)
    {
        ultrasonic_sum += ultrasonic_value;
        ultrasonic_sample_count++;
    }
}

void Sensor_Task_100ms(void)
{
    float laser_value = Read_Laser_VL53L0X_mm();

    if (laser_value >= 0.0f)
    {
        laser_sum += laser_value;
        laser_sample_count++;
    }
}

/* =========================
   TINH TRUNG BINH CAC MAU
   Goi truoc khi gui USB
   ========================= */
void Sensor_Compute_Average(void)
{
    if (thermistor_sample_count > 0)
    {
        sensor_data.thermistor_degC =
            thermistor_sum / thermistor_sample_count;
    }
    else
    {
        sensor_data.thermistor_degC = -999.0f;
    }

    if (potentiometer_sample_count > 0)
    {
        sensor_data.potentiometer_deg =
            potentiometer_sum / potentiometer_sample_count;
    }
    else
    {
        sensor_data.potentiometer_deg = -1.0f;
    }

    if (laser_sample_count > 0)
    {
        sensor_data.laser_mm =
            laser_sum / laser_sample_count;
    }
    else
    {
        sensor_data.laser_mm = -1.0f;
    }

    if (ultrasonic_sample_count > 0)
    {
        sensor_data.ultrasonic_cm =
            ultrasonic_sum / ultrasonic_sample_count;
    }
    else
    {
        sensor_data.ultrasonic_cm = -1.0f;
    }
}

/* =========================
   GUI DU LIEU RAW QUA UART (USART1)
   Phuc vu cho viec Calib tren Hercules
   ========================= */
void Send_Raw_Calibration_Data_UART(void)
{
    char uart_buf[300];
    
    // 1. Doc ADC1 (Thermistor RAW)
    uint16_t adc1_raw = Read_ADC_Value(&hadc1);
    
    // 2. Doc ADC2 (Potentiometer RAW)
    uint16_t adc2_raw = Read_ADC_Value(&hadc2);
    
    // Tinh Voltage va Angle cho Potentiometer
    float voltage = (float)adc2_raw * 3.3f / 4095.0f;
    float angle = (voltage - 0.1f) * (300.0f / 2.9f);
    if (angle < 0.0f) angle = 0.0f;     // Gioi han duoi
    if (angle > 300.0f) angle = 300.0f; // Gioi han tren
    
    // Xu ly in so thuc tren STM32 ma khong can enable -u _printf_float
    int v_int = (int)voltage;
    int v_frac = (int)((voltage - v_int) * 1000);
    if (v_frac < 0) v_frac = -v_frac;
    
    int a_int = (int)angle;
    int a_frac = (int)((angle - a_int) * 10);
    if (a_frac < 0) a_frac = -a_frac;

    // 3. Doc Ultrasonic RAW (Echo time in us)
    ultrasonic_data_ready = 0;
    ultrasonic_echo_us = 0;
    Ultrasonic_Trigger();
    uint32_t start_tick = HAL_GetTick();
    while (!ultrasonic_data_ready)
    {
        if (HAL_GetTick() - start_tick > 30) break; // Timeout
    }
    uint32_t echo_us = ultrasonic_echo_us;
    
    // 4. Doc Laser RAW/CALIB
    float laser_calib = Read_Laser_VL53L0X_mm();

    int l_calib_int = (int)laser_calib;
    int l_calib_frac = (int)((laser_calib - l_calib_int) * 10);

    if (l_calib_frac < 0)
    {
        l_calib_frac = -l_calib_frac;
    }

    // 5. In ra UART
    int len = snprintf(uart_buf, sizeof(uart_buf),
        "\r\n--- RAW DATA FOR CALIB ---\r\n"
        "ADC1 (Thermistor) = %u\r\n"
        "ADC2 (Poten)      = %u\r\n"
        "-> Voltage        = %d.%03d V\r\n"
        "-> Angle          = %d.%01d deg\r\n"
        "Ultrasonic (us)   = %lu\r\n"
        "Laser raw (mm)    = %u\r\n"
        "Laser calib (mm)  = %d.%01d\r\n"
        "--------------------------\r\n",
        adc1_raw,
        adc2_raw,
        v_int,
        v_frac,
        a_int,
        a_frac,
        echo_us,
        laser_raw_mm_last,
        l_calib_int,
        l_calib_frac);
        
    if (len > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, len, 100);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_USB_DEVICE_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_Delay(100);

  if (HAL_I2C_IsDeviceReady(&hi2c1, 0x52, 3, 100) == HAL_OK)
  {
      UART_Send_Text("VL53L0X I2C OK\r\n");
  }
  else
  {
      UART_Send_Text("VL53L0X I2C FAIL\r\n");
  }

  lidar_init(dir_s1);
  HAL_Delay(100);

  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
  HAL_TIM_Base_Start_IT(&htim2);

  HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);

  UART_Send_Text("STM32 READY\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if (uart_cmd_ready)
      {
          char cmd_copy[UART_CMD_BUFFER_SIZE];

          __disable_irq();
          strcpy(cmd_copy, uart_cmd_buffer);
          uart_cmd_ready = 0;
          __enable_irq();

          Process_Command(cmd_copy);
      }

      if (stream_enable)
      {
          if (flag_10ms)
          {
              flag_10ms = 0;
              Sensor_Task_10ms();
          }

          if (flag_50ms)
          {
              flag_50ms = 0;
              Sensor_Task_50ms();
          }

          if (flag_100ms)
          {
              flag_100ms = 0;

              Sensor_Task_100ms();

              Sensor_Compute_Average();
              Send_Real_Sensor_Data_UART();

              Reset_Sensor_Sample_Buffer();
          }
      }
      else
      {
          flag_10ms = 0;
          flag_50ms = 0;
          flag_100ms = 0;
          Reset_Sensor_Sample_Buffer();
      }
      /* USER CODE END WHILE */
      /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADCEx_Calibration_Start(&hadc2);
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7200-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 99;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 72-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */
  HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        /*
           TIM2 ngắt mỗi 10ms.
           Mỗi lần ngắt tăng counter.
        */
        timer_counter++;
        /*
           Task 10ms:
           Mỗi lần ngắt đều bật flag_10ms.
        */
        flag_10ms = 1;
        /*
           Task 50ms:
           5 lần ngắt x 10ms = 50ms.
        */
        if (timer_counter % 5 == 0)
        {
            flag_50ms = 1;
        }

        /*
           Task 100ms:
           10 lần ngắt x 10ms = 100ms.
        */
        if (timer_counter % 10 == 0)
        {
            flag_100ms = 1;
        }

        /*
           Reset counter để tránh tăng mãi.
           100 lần ngắt x 10ms = 1000ms = 1s.
        */
        if (timer_counter >= 100)
        {
            timer_counter = 0;
        }
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        if (ultrasonic_capture_state == 0)
        {
            /*
               Bat canh len cua Echo
            */
            ultrasonic_rise_tick = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);

            ultrasonic_capture_state = 1;
        }
        else
        {
            /*
               Bat canh xuong cua Echo
            */
            ultrasonic_fall_tick = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

            if (ultrasonic_fall_tick >= ultrasonic_rise_tick)
            {
                ultrasonic_echo_us = ultrasonic_fall_tick - ultrasonic_rise_tick;
            }
            else
            {
                ultrasonic_echo_us = (65535 - ultrasonic_rise_tick) + ultrasonic_fall_tick + 1;
            }

            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);

            ultrasonic_capture_state = 0;
            ultrasonic_data_ready = 1;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        char c = (char)uart_rx_byte;

        if (c == '\n' || c == '\r')
        {
            if (uart_rx_index > 0)
            {
                uart_rx_buffer[uart_rx_index] = '\0';

                if (!uart_cmd_ready)
                {
                    strcpy(uart_cmd_buffer, uart_rx_buffer);
                    uart_cmd_ready = 1;
                }

                uart_rx_index = 0;
            }
        }
        else
        {
            if (uart_rx_index < UART_CMD_BUFFER_SIZE - 1)
            {
                uart_rx_buffer[uart_rx_index++] = c;
            }
            else
            {
                uart_rx_index = 0;
            }
        }

        HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
