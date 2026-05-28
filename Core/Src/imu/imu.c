#include "imu/imu.h"
#include "MPU9250-DMP.h"
#include "math.h"
#include "main.h"

#define PI 3.14159265359f

/**
 * @brief Inicializa o módulo IMU e o sensor MPU9250
 */
bool IMU_Init(void)
{
    inv_error_t result;

    // Inicializa sensibilidades
    MPU9250_DMP();

    // Inicializa o MPU9250
    result = MPU9250_begin();
    if (result != INV_SUCCESS) {
        return false;
    }

    // Habilita DMP com calibração automática de giroscópio
    // DMP_FEATURE_6X_LP_QUAT: Calcula quaternion com acelerômetro + giroscópio
    // DMP_FEATURE_GYRO_CAL: Calibra automaticamente o bias do giroscópio após 8s imóvel
    // DMP_FEATURE_SEND_RAW_ACCEL/GYRO: Envia dados brutos ao FIFO
    result = MPU9250_dmpBegin(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL | 
                              DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_RAW_GYRO, 20);
    if (result != INV_SUCCESS) {
        return false;
    }

    HAL_Delay(100); // Aguarda DMP inicializar

    return true;
}

/**
 * @brief Atualiza dados do IMU lendo do FIFO
 */
bool IMU_Update(IMU_RawData_t *pRawData, IMU_Euler_t *pEuler)
{
    if (pRawData == NULL || pEuler == NULL) {
        return false;
    }

    // Lê dados do FIFO do DMP
    inv_error_t result = MPU9250_dmpUpdateFifo();
    if (result != INV_SUCCESS) {
        return false;
    }

    // Copia dados do MPU9250 (variáveis globais declaradas em MPU9250-DMP.h)
    pRawData->accel[0] = ax;
    pRawData->accel[1] = ay;
    pRawData->accel[2] = az;
    pRawData->gyro[0] = gx;
    pRawData->gyro[1] = gy;
    pRawData->gyro[2] = gz;
    pRawData->quat[0] = qw;
    pRawData->quat[1] = qx;
    pRawData->quat[2] = qy;
    pRawData->quat[3] = qz;

    // Converte quaternion para ângulos de Euler
    IMU_QuatToEuler(pRawData->quat, pEuler);

    return true;
}

/**
 * @brief Converte quaternion para ângulos de Euler
 * q = [w, x, y, z] (convenção w-first)
 */
void IMU_QuatToEuler(long *quat, IMU_Euler_t *pEuler)
{
    if (quat == NULL || pEuler == NULL) {
        return;
    }

    // Normaliza quaternion do formato Q30 (2^30)
    float q0 = quat[0] / 1073741824.0f;
    float q1 = quat[1] / 1073741824.0f;
    float q2 = quat[2] / 1073741824.0f;
    float q3 = quat[3] / 1073741824.0f;

    // Converte para ângulos de Euler (Pitch, Roll, Yaw)
    // Roll (rotação em X)
    pEuler->roll = atan2f(2.0f * (q0 * q1 + q2 * q3),
                          1.0f - 2.0f * (q1 * q1 + q2 * q2)) * 180.0f / PI;

    // Pitch (rotação em Y)
    float sinp = 2.0f * (q0 * q2 - q3 * q1);
    if (sinp >= 1.0f) {
        pEuler->pitch = 90.0f;
    } else if (sinp <= -1.0f) {
        pEuler->pitch = -90.0f;
    } else {
        pEuler->pitch = asinf(sinp) * 180.0f / PI;
    }

    // Yaw (rotação em Z)
    pEuler->yaw = atan2f(2.0f * (q0 * q3 + q1 * q2),
                         1.0f - 2.0f * (q2 * q2 + q3 * q3)) * 180.0f / PI;
}

/**
 * @brief Converte dados brutos para dados calibrados (G's, deg/s, µT)
 */
void IMU_GetCalibData(IMU_RawData_t *pRawData, IMU_CalibData_t *pCalibData)
{
    if (pRawData == NULL || pCalibData == NULL) {
        return;
    }

    // Converte aceleração bruta para G's
    pCalibData->accel_g[0] = MPU9250_calcAccel(pRawData->accel[0]);
    pCalibData->accel_g[1] = MPU9250_calcAccel(pRawData->accel[1]);
    pCalibData->accel_g[2] = MPU9250_calcAccel(pRawData->accel[2]);

    // Converte giroscópio bruto para graus/segundo
    pCalibData->gyro_dps[0] = MPU9250_calcGyro(pRawData->gyro[0]);
    pCalibData->gyro_dps[1] = MPU9250_calcGyro(pRawData->gyro[1]);
    pCalibData->gyro_dps[2] = MPU9250_calcGyro(pRawData->gyro[2]);

    // Converte magnetômetro bruto para µT (usa globais mx, my, mz preenchidos por updateCompass)
    pCalibData->mag_ut[0] = MPU9250_calcMag(mx);
    pCalibData->mag_ut[1] = MPU9250_calcMag(my);
    pCalibData->mag_ut[2] = MPU9250_calcMag(mz);
}

/**
 * @brief Obtém heading da bússola (mx, my, mz devem estar preenchidos do FIFO)
 */
bool IMU_GetCompass(IMU_Compass_t *pCompass)
{
    if (pCompass == NULL) {
        return false;
    }

    // Calcula heading usando dados globais mx, my, mz do MPU9250-DMP.h
    pCompass->heading = MPU9250_computeCompassHeading();
    
    // Armazena componentes brutos para referência
    pCompass->compassX = (float)mx;
    pCompass->compassY = (float)my;

    return true;
}

/**
 * @brief Ativa detecção de taps
 */
bool IMU_EnableTap(void)
{
    // Habilita DMP_FEATURE_TAP (se não estiver já ativado)
    // Configura threshold em 250mg/ms para cada eixo, tapTime 100ms, tapMulti 500ms, min 1 tap
    inv_error_t result = MPU9250_dmpSetTap(250, 250, 250, 1, 100, 500);
    if (result != INV_SUCCESS) {
        return false;
    }

    // Obtém features ativas e adiciona TAP, preservando tudo
    unsigned short enabledFeatures = MPU9250_dmpGetEnabledFeatures();
    enabledFeatures |= DMP_FEATURE_TAP;
    // Garante que dados brutos continuam sendo enviados
    enabledFeatures |= (DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_RAW_GYRO);
    
    result = MPU9250_dmpEnableFeatures(enabledFeatures);
    if (result != INV_SUCCESS) {
        return false;
    }

    return true;
}

/**
 * @brief Verifica e obtém dados de tap
 */
bool IMU_GetTap(IMU_Tap_t *pTap)
{
    if (pTap == NULL) {
        return false;
    }

    pTap->tapDetected = MPU9250_tapAvailable();
    
    if (pTap->tapDetected) {
        pTap->tapDir = MPU9250_getTapDir();    // 0-5: X-, X+, Y-, Y+, Z-, Z+
        pTap->tapCount = MPU9250_getTapCount();
    } else {
        pTap->tapDir = 0;
        pTap->tapCount = 0;
    }

    return pTap->tapDetected;
}

/**
 * @brief Ativa pedômetro
 */
bool IMU_EnablePedometer(void)
{
    // Obtém features ativas e adiciona PEDOMETER, preservando tudo
    unsigned short enabledFeatures = MPU9250_dmpGetEnabledFeatures();
    enabledFeatures |= DMP_FEATURE_PEDOMETER;
    // Garante que dados brutos continuam sendo enviados
    enabledFeatures |= (DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_RAW_GYRO);
    
    inv_error_t result = MPU9250_dmpEnableFeatures(enabledFeatures);
    if (result != INV_SUCCESS) {
        return false;
    }

    return true;
}

/**
 * @brief Obtém número de passos
 */
uint32_t IMU_GetSteps(void)
{
    return (uint32_t)MPU9250_dmpGetPedometerSteps();
}

/**
 * @brief Reseta contador de passos
 */
void IMU_ResetSteps(void)
{
    MPU9250_dmpSetPedometerSteps(0);
}

/**
 * @brief Obtém temperatura do sensor em graus Celsius
 * A variável temperature está em formato Q16 (multiplicada por 65536)
 * Fórmula já inclusa na biblioteca: (35 + ((raw - offset) / sensitivity)) * 65536
 */
float IMU_GetTemperature(void)
{
    extern long temperature;  // Variável global da biblioteca MPU9250 em Q16
    // Atualiza leitura de temperatura
    MPU9250_update(UPDATE_TEMP);
    HAL_Delay(5);
    // Converte de Q16 para graus Celsius
    float temp_c = (float)temperature / 65536.0f;
    return temp_c;
}
