#include "menu.h"
#include "math.h"
#include <stdio.h>
#include "MPU9250-DMP.h"

char retVal;
char titleMenu[] = "Main Menu";
char option1[] = "1. Euler Angles";
char option2[] = "2. Raw Sensors";
char option3[] = "3. Temperatura";
bool flagMenu = true;
bool flagSelected = false;
uint8_t menuOption = 1;

// Variáveis para IMU
static IMU_RawData_t imuRawData;
static IMU_Euler_t imuEuler;
static IMU_CalibData_t imuCalibData;
static char buf[32];
static uint32_t lastUpdateTime = 0;
static uint32_t tempLastTime = 0;

/**
 * @brief Inicializa o módulo de menu (IMU, display, etc)
 */
void menuInit(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Initializing IMU...", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(1000);  // Aguarda I2C estar estável

    if (!IMU_Init()) {
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("ERROR: IMU Init", Font_7x10, White);
        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString("Check I2C wiring", Font_6x8, White);
        ssd1306_UpdateScreen();
        HAL_Delay(3000);  // Mostra erro por 3 segundos
        Error_Handler();
    }

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("IMU OK", Font_7x10, White);
    ssd1306_SetCursor(0, 20);
    ssd1306_WriteString("Calibrating gyro...", Font_6x8, White);
    ssd1306_SetCursor(0, 30);
    ssd1306_WriteString("Keep still (8s)", Font_6x8, White);
    ssd1306_UpdateScreen();
    HAL_Delay(8500);  // Calibração automática do DMP leva 8 segundos

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Ready!", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(3000);  // Aguarda FIFO popular após ativar features
}

/**
 * @brief Exibe dados do IMU (Pitch, Roll, Yaw)
 */
static void option1Selected(void)
{
    uint32_t now = HAL_GetTick();
    static bool firstTime = true;

    // Sempre limpa e mantém a tela de IMU visível
    ssd1306_Fill(Black);

    // Atualiza dados apenas a cada 50ms (~20 Hz)
    if (firstTime || (now - lastUpdateTime) >= 50) {
        lastUpdateTime = now;
        firstTime = false;

        // Tenta ler dados do IMU
        if (!IMU_Update(&imuRawData, &imuEuler)) {
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString("ERROR: No data", Font_6x8, White);
            ssd1306_SetCursor(0, 16);
            ssd1306_WriteString(buf, Font_6x8, White);
            ssd1306_UpdateScreen();
            return;
        }
    }

    // Título
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("IMU DATA", Font_6x8, White);

    // Pitch - use int conversion
    ssd1306_SetCursor(0, 12);
    int p = (int)imuEuler.pitch;
    int pfrac = (int)((imuEuler.pitch - p) * 10);
    snprintf(buf, sizeof(buf), "P:%d.%d", p, pfrac < 0 ? -pfrac : pfrac);
    ssd1306_WriteString(buf, Font_6x8, White);

    // Roll
    ssd1306_SetCursor(0, 24);
    int r = (int)imuEuler.roll;
    int rfrac = (int)((imuEuler.roll - r) * 10);
    snprintf(buf, sizeof(buf), "R:%d.%d", r, rfrac < 0 ? -rfrac : rfrac);
    ssd1306_WriteString(buf, Font_6x8, White);

    // Yaw
    ssd1306_SetCursor(0, 36);
    int y = (int)imuEuler.yaw;
    int yfrac = (int)((imuEuler.yaw - y) * 10);
    snprintf(buf, sizeof(buf), "Y:%d.%d", y, yfrac < 0 ? -yfrac : yfrac);
    ssd1306_WriteString(buf, Font_6x8, White);

    // Status
    ssd1306_SetCursor(0, 60);
    ssd1306_WriteString("OK", Font_6x8, White);

    ssd1306_UpdateScreen();
}

static void option2Selected(){
    uint32_t now = HAL_GetTick();
    static bool firstTime = true;
    static int updateCount = 0;

    ssd1306_Fill(Black);

    if (firstTime || (now - lastUpdateTime) >= 50) {
        lastUpdateTime = now;
        firstTime = false;
        updateCount++;

        if (!IMU_Update(&imuRawData, &imuEuler)) {
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString("ERROR: No data", Font_6x8, White);
            ssd1306_UpdateScreen();
            return;
        }

        IMU_GetCalibData(&imuRawData, &imuCalibData);
    }

    // Exibe aceleração em G's
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("ACCEL (G)", Font_6x8, White);
    
    ssd1306_SetCursor(0, 10);
    int ax_int = (int)imuCalibData.accel_g[0];
    int ax_frac = (int)((imuCalibData.accel_g[0] - ax_int) * 10);
    snprintf(buf, sizeof(buf), "X:%d.%d", ax_int, ax_frac < 0 ? -ax_frac : ax_frac);
    ssd1306_WriteString(buf, Font_6x8, White);

    ssd1306_SetCursor(0, 20);
    int ay_int = (int)imuCalibData.accel_g[1];
    int ay_frac = (int)((imuCalibData.accel_g[1] - ay_int) * 10);
    snprintf(buf, sizeof(buf), "Y:%d.%d", ay_int, ay_frac < 0 ? -ay_frac : ay_frac);
    ssd1306_WriteString(buf, Font_6x8, White);

    ssd1306_SetCursor(0, 30);
    int az_int = (int)imuCalibData.accel_g[2];
    int az_frac = (int)((imuCalibData.accel_g[2] - az_int) * 10);
    snprintf(buf, sizeof(buf), "Z:%d.%d", az_int, az_frac < 0 ? -az_frac : az_frac);
    ssd1306_WriteString(buf, Font_6x8, White);

    // Exibe giroscópio em deg/s
    ssd1306_SetCursor(0, 42);
    ssd1306_WriteString("GYRO (d/s)", Font_6x8, White);

    ssd1306_SetCursor(0, 52);
    int gx_int = (int)imuCalibData.gyro_dps[0];
    int gx_frac = (int)((imuCalibData.gyro_dps[0] - gx_int) * 10);
    snprintf(buf, sizeof(buf), "Gx:%d.%d", gx_int, gx_frac < 0 ? -gx_frac : gx_frac);
    ssd1306_WriteString(buf, Font_6x8, White);

    ssd1306_UpdateScreen();
}

static void option3Selected(){
    uint32_t now = HAL_GetTick();
    static bool firstTime = true;
    static float temperature = 0.0f;

    ssd1306_Fill(Black);

    if (firstTime || (now - tempLastTime) >= 500) {
        tempLastTime = now;
        firstTime = false;
        
        // Lê temperatura do sensor
        temperature = IMU_GetTemperature();
    }

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("TEMPERATURA", Font_7x10, White);

    ssd1306_SetCursor(0, 20);
    snprintf(buf, sizeof(buf), "T: %.2f C", temperature);
    ssd1306_WriteString(buf, Font_7x10, White);

    ssd1306_SetCursor(0, 35);
    if (temperature < 20.0f) {
        ssd1306_WriteString("Muito frio!", Font_6x8, White);
    } else if (temperature > 40.0f) {
        ssd1306_WriteString("Muito quente!", Font_6x8, White);
    } else {
        ssd1306_WriteString("Normal", Font_6x8, White);
    }

    ssd1306_UpdateScreen();
}

void pressButtonSelect(){
      if(!flagSelected){

      if(menuOption < 3){
        menuOption++;
      }
      else{
        menuOption = 1;
      }
    }
}

void pressButtonConfirm(){
    flagSelected = !flagSelected;
    // Reseta o timer de atualização quando entra em uma opção
    if(flagSelected) {
        lastUpdateTime = 0;
    }
}

void menuUpdate(){
    // Se uma opção está selecionada, exibe apenas essa opção
    if(flagSelected) {
        switch(menuOption) {
            case 1:
                option1Selected();
                break;
            case 2:
                option2Selected();
                break;
            case 3:
                option3Selected();
                break;
            default:
                break;
        }
        return;  // Sai sem desenhar o menu
    }

    // Caso contrário, desenha o menu principal
    ssd1306_Fill(Black);

    // Com apenas 3 opções, sempre mostra todas
    uint8_t startOption = 1;

    // Desenho dos retângulos para as opções do menu
    ssd1306_FillRectangle(0, 0, 127, 15, White);  
    ssd1306_FillRectangle(2, 18, 125, 29, White);
    ssd1306_FillRectangle(2, 34, 125, 45, White);
    ssd1306_FillRectangle(2, 50, 125, 61, White);

    // Texto do menu
    ssd1306_SetCursor(5, 5);
    retVal = ssd1306_WriteString(titleMenu, Font_6x8, Black);

    // Exibe as 3 opções visíveis
    char *optionStrings[] = {NULL, option1, option2, option3};
    
    ssd1306_SetCursor(4, 20);
    retVal = ssd1306_WriteString(optionStrings[startOption], Font_6x8, Black);
    ssd1306_SetCursor(4, 36);
    retVal = ssd1306_WriteString(optionStrings[startOption + 1], Font_6x8, Black);
    ssd1306_SetCursor(4, 52);
    retVal = ssd1306_WriteString(optionStrings[startOption + 2], Font_6x8, Black);

    // Desenho do retângulo para destacar a opção selecionada
    if(flagMenu) {
        uint8_t relativePos = menuOption - startOption;
        switch(relativePos) {
            case 0:
                ssd1306_DrawRectangle(0, 16, 127, 31, White);
                break;
            case 1:
                ssd1306_DrawRectangle(0, 32, 127, 47, White);
                break;
            case 2:
                ssd1306_DrawRectangle(0, 48, 127, 63, White);
                break;
            default:
                break;
        }
    }

    ssd1306_UpdateScreen();
}
