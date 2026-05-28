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
 * @brief Estrutura para dados calibrados do IMU
 */
typedef struct {
    float accel_g[3];   // Aceleração em G's
    float gyro_dps[3];  // Giroscópio em graus/segundo
    float mag_ut[3];    // Magnetômetro em µT
} IMU_CalibData_t;

/**
 * @brief Estrutura para bússola/heading
 */
typedef struct {
    float heading;      // Ângulo de direção (0-360 graus)
    float compassX;     // Componente X do magnômetro calibrado
    float compassY;     // Componente Y do magnômetro calibrado
} IMU_Compass_t;

/**
 * @brief Estrutura para dados de tap detection
 */
typedef struct {
    uint8_t tapDir;     // Direção do tap (eixo)
    uint8_t tapCount;   // Quantas vezes foi detectado o tap
    bool tapDetected;   // Flag se tap foi detectado
} IMU_Tap_t;

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
 * @brief Obtém dados calibrados (G's, deg/s, µT)
 * @param pRawData Ponteiro para dados brutos
 * @param pCalibData Ponteiro para dados calibrados
 */
void IMU_GetCalibData(IMU_RawData_t *pRawData, IMU_CalibData_t *pCalibData);

/**
 * @brief Calcula ângulos de Euler a partir de quaternion
 * @param quat Array com 4 elementos do quaternion (W, X, Y, Z)
 * @param pEuler Ponteiro para estrutura com os ângulos calculados
 */
void IMU_QuatToEuler(long *quat, IMU_Euler_t *pEuler);

/**
 * @brief Obtém o heading da bússola
 * @param pCompass Ponteiro para estrutura de bússola
 * @return true se dados foram obtidos
 */
bool IMU_GetCompass(IMU_Compass_t *pCompass);

/**
 * @brief Ativa tap detection
 * @return true se ativado com sucesso
 */
bool IMU_EnableTap(void);

/**
 * @brief Verifica se houeve tap e obtém informações
 * @param pTap Ponteiro para estrutura de tap
 * @return true se tap foi detectado
 */
bool IMU_GetTap(IMU_Tap_t *pTap);

/**
 * @brief Ativa pedômetro
 * @return true se ativado com sucesso
 */
bool IMU_EnablePedometer(void);

/**
 * @brief Obtém número de passos
 * @return número de passos contados
 */
uint32_t IMU_GetSteps(void);

/**
 * @brief Reseta contador de passos
 */
void IMU_ResetSteps(void);

/**
 * @brief Obtém temperatura do sensor
 * @return temperatura em graus Celsius
 */
float IMU_GetTemperature(void);

#endif // IMU_H
