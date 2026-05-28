#include "menu.h"
#include "math.h"
#include <stdio.h>

char retVal;
char titleMenu[] = "Main Menu";
char option1[] = "1. IMU Data";
char option2[] = "2. Option 2";
char option3[] = "3. Option 3";
bool flagMenu = true;
bool flagSelected = false;
uint8_t menuOption = 1;

// Variáveis para IMU
static IMU_RawData_t imuRawData;
static IMU_Euler_t imuEuler;
static char buf[32];
static uint32_t lastUpdateTime = 0;

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
}

/**
 * @brief Exibe dados do IMU (Pitch, Roll, Yaw)
 */
static void option1Selected(void)
{
    uint32_t now = HAL_GetTick();
    static bool firstTime = true;
    static int updateCount = 0;

    // Sempre limpa e mantém a tela de IMU visível
    ssd1306_Fill(Black);

    // Atualiza dados apenas a cada 50ms (~20 Hz)
    if (firstTime || (now - lastUpdateTime) >= 50) {
        lastUpdateTime = now;
        firstTime = false;
        updateCount++;

        // Tenta ler dados do IMU
        if (!IMU_Update(&imuRawData, &imuEuler)) {
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString("ERROR: No data", Font_6x8, White);
            ssd1306_SetCursor(0, 16);
            snprintf(buf, sizeof(buf), "Updates: %d", updateCount);
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

    // Contador de atualizações
    ssd1306_SetCursor(0, 48);
    snprintf(buf, sizeof(buf), "C:%d", updateCount);
    ssd1306_WriteString(buf, Font_6x8, White);

    // Status
    ssd1306_SetCursor(0, 60);
    ssd1306_WriteString("OK", Font_6x8, White);

    ssd1306_UpdateScreen();
}

static void option2Selected(){
  ssd1306_Fill(Black);
  ssd1306_SetCursor(5, 5);
  retVal = ssd1306_WriteString("Option 2 selected", Font_6x8, White);
  ssd1306_UpdateScreen();
}


static void option3Selected(){
  ssd1306_Fill(Black);
  ssd1306_SetCursor(5, 5);
  retVal = ssd1306_WriteString("Option 3 selected", Font_6x8, White);
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

    // Desenho dos retângulos para as opções do menu
    ssd1306_FillRectangle(0, 0, 127, 15, White);  
    ssd1306_FillRectangle(2, 18, 125, 29, White);
    ssd1306_FillRectangle(2, 34, 125, 45, White);
    ssd1306_FillRectangle(2, 50, 125, 61, White);

    // Texto do menu
    ssd1306_SetCursor(5, 5);
    retVal = ssd1306_WriteString(titleMenu, Font_6x8, Black);
    ssd1306_SetCursor(4, 20);
    retVal = ssd1306_WriteString(option1, Font_6x8, Black);
    ssd1306_SetCursor(4, 36);
    retVal = ssd1306_WriteString(option2, Font_6x8, Black);
    ssd1306_SetCursor(4, 52);
    retVal = ssd1306_WriteString(option3, Font_6x8, Black);

    // Desenho do retângulo para destacar a opção selecionada
    if(flagMenu) {
        switch(menuOption) {
            case 1:
                ssd1306_DrawRectangle(0, 16, 127, 31, White);
                break;
            case 2:
                ssd1306_DrawRectangle(0, 32, 127, 47, White);
                break;
            case 3:
                ssd1306_DrawRectangle(0, 48, 127, 63, White);
                break;
            default:
                break;
        }
    }

    ssd1306_UpdateScreen();
}
