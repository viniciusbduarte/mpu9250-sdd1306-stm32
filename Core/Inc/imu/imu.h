#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Estrutura para armazenar dados de ângulos de Euler
 */
typedef struct {
    float pitch;
    float roll;
    float yaw;
} IMU_Euler_t;

/**
 * @brief Estrutura para armazenar dados brutos do IMU
 */
typedef struct {
    int16_t gyro[3];    // Gyroscope: GX, GY, GZ
    int16_t accel[3];   // Accelerometer: AX, AY, AZ
    long quat[4];       // Quaternion: W, X, Y, Z
} IMU_RawData_t;

/**
 * @brief Inicializa o módulo IMU
 * @return true se inicializado com sucesso, false caso contrário
 */
bool IMU_Init(void);

/**
 * @brief Atualiza dados do IMU lendo do FIFO do DMP
 * @param pRawData Ponteiro para estrutura com dados brutos
 * @param pEuler Ponteiro para estrutura com ângulos de Euler calculados
 * @return true se dados foram lidos com sucesso, false caso contrário
 */
bool IMU_Update(IMU_RawData_t *pRawData, IMU_Euler_t *pEuler);

/**
 * @brief Calcula ângulos de Euler a partir de quaternion
 * @param quat Array com 4 elementos do quaternion (W, X, Y, Z)
 * @param pEuler Ponteiro para estrutura com os ângulos calculados
 */
void IMU_QuatToEuler(long *quat, IMU_Euler_t *pEuler);

#endif // IMU_H
