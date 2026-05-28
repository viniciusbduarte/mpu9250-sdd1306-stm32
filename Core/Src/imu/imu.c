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
    result = MPU9250_dmpBegin(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL, 20);
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
