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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
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
static volatile uint8_t flag_10ms = 0;
static volatile uint8_t flag_50ms = 0;
static volatile uint8_t flag_100ms = 0;
static volatile uint16_t timer_counter = 0;
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
float Read_Thermistor_degC(void)
{
    uint16_t adc_raw = Read_ADC_Value(&hadc1);

    const float Vref = 3.3f;
    const float R_fixed = 10000.0f;   // dien tro co dinh 10k
    const float R0 = 10000.0f;        // NTC 10k tai 25 do C
    const float T0 = 298.15f;         // 25 do C = 298.15 K
    const float BETA = 3950.0f;       // he so beta, sau nay sua theo datasheet

    float Vout = adc_raw * Vref / 4095.0f;

    if (Vout <= 0.01f || Vout >= (Vref - 0.01f))
    {
        return -999.0f;
    }

    /*
       Mach gia dinh:
       3.3V --- R_fixed --- ADC --- NTC --- GND

       R_ntc = R_fixed * Vout / (Vref - Vout)
    */
    float R_ntc = R_fixed * Vout / (Vref - Vout);

    /*
       Cong thuc Beta:
       T(K) = 1 / [1/T0 + (1/BETA)*ln(R/R0)]
       T(C) = T(K) - 273.15
    */
    float T_kelvin = 1.0f / ((1.0f / T0) + (1.0f / BETA) * logf(R_ntc / R0));
    float T_celsius = T_kelvin - 273.15f;

    return T_celsius;
}

/* =========================
   2. DOC POTENTIOMETER
   ADC2 PA1 -> Goc 0-300 do
   ========================= */
float Read_Potentiometer_deg(void)
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
	// reset cờ
	ultrasonic_data_ready = 0;
	ultrasonic_echo_us = 0;

    Ultrasonic_Trigger();
    /*
       Cho echo toi da khoang 30ms.
       Neu qua 30ms ma khong co data thi bao loi.
    */
    uint32_t start_tick = HAL_GetTick();

    while (!ultrasonic_data_ready)
    {
        if (HAL_GetTick() - start_tick > 30)
        {
            return -1.0f;
        }
    }

    /*
       v = 331.4 + 0.6*T  m/s
       doi sang cm/us: chia 10000
    */
    float speed_cm_us = (331.4f + 0.6f * tempC) * 100.0f / 1000000.0f;

    /*
       Song di va ve nen chia 2
    */
    float distance_cm = ultrasonic_echo_us * speed_cm_us / 2.0f;

    /*
       Calib tuyen tinh:
       distance_calib = a * distance_read + b
       Tạo hàm calib return về giá trị a, b
    */
    const float a = 1.0f;
    const float b = 0.0f;

    float distance_calib = a * distance_cm + b;

    return distance_calib;
}
float Calib_Ultrasonic(float a, float b){

}

/* =========================
   4. DOC LASER VL53L0X
   Luu y: VL53L0X can driver rieng.
   Ham nay hien tai moi kiem tra co cam bien I2C hay chua.
   ========================= */
float Read_Laser_VL53L0X_mm(void)
{
    /*
       Dia chi mac dinh cua VL53L0X thuong la 0x29.
       HAL can dia chi dich trai 1 bit.
    */
    uint16_t VL53L0X_ADDR = 0x29 << 1;

    if (HAL_I2C_IsDeviceReady(&hi2c1, VL53L0X_ADDR, 2, 10) != HAL_OK)
    {
        return -1.0f;   // khong tim thay cam bien
    }

    /*
       VL53L0X khong the doc khoang cach chi bang 1 lenh I2C don gian.
       Can them driver khoi tao va ham ranging.

       Sau khi co driver, thay phan nay bang:
       D_read_mm = VL53L0X_ReadRange();
    */

    float D_read_mm = 0.0f;

    /*
       Calib tuyen tinh:
       D_calib = a * D_read + b
    */
    const float a = 1.0f;
    const float b = 0.0f;

    float D_calib_mm = a * D_read_mm + b;

    return D_calib_mm;
}

/* =========================
   GUI DU LIEU 4 CAM BIEN DA XU LY
   USB CDC
   ========================= */
void Send_Real_Sensor_Data_USB(void)
{
    char tx_buffer[160];

    int len = snprintf(tx_buffer, sizeof(tx_buffer),
                       "THERMISTOR:%.1f;POTENTIOMETER:%.1f;ULTRASONIC:%.1f;LASER:%.1f\r\n",
                       sensor_data.thermistor_degC,
                       sensor_data.potentiometer_deg,
                       sensor_data.ultrasonic_cm,
                       sensor_data.laser_mm);

    if (len > 0)
    {
        CDC_Transmit_FS((uint8_t*)tx_buffer, len);
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

    float laser_value = Read_Laser_VL53L0X_mm();

    if (laser_value >= 0.0f)
    {
        laser_sum += laser_value;
        laser_sample_count++;
    }
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
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /*
	        flag_10ms:
	        Moi 10ms lay 1 mau thermistor, potentiometer, laser.
	     */
	     if (flag_10ms)
	     {
	         flag_10ms = 0;

	         Sensor_Task_10ms();
	     }

	     /*
	        flag_50ms:
	        Moi 50ms lay 1 mau ultrasonic.
	     */
	     if (flag_50ms)
	     {
	         flag_50ms = 0;

	         Sensor_Task_50ms();
	     }

	     /*
	        flag_100ms:
	        Moi 100ms tinh trung binh va gui USB.
	     */
	     if (flag_100ms)
	     {
	         flag_100ms = 0;

	         Sensor_Compute_Average();
	         Send_Real_Sensor_Data_USB();

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
  htim4.Init.Prescaler = 0;
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
  huart1.Init.BaudRate = 9600;
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
