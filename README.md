# Librería Arduino AsyncStepper v2.0
Librería para Arduino que permite mover un motor paso a paso de forma no bloqueante, con aceleración y deceleración lineal. De esta forma se tiene un motor paso a paso que tiene un cierto comportamiento "asíncrono".

Más información https://www.luisllamas.es/libreria-arduino-asyncstepper/

## Instrucciones de uso
La clase AsyncStepper implementa un motor paso a paso cuyos movimientos están temporizados, en lugar de ser bloqueantes. El objetivo de la librería es poder mover uno o varios motores paso a paso en un proyecto, incluso a diferentes velocidades, mientras que no se impide la ejecución otras tareas en el bucle de control principal.

La clase AsyncStepper no realiza el control del motor paso a paso, únicamente realiza la temporaización de los pasos. Esto permite que sea muy versatil, ya que es independiente del motor paso a paso o controlador empleado.

Para conseguir controlar el motor paso a paso, debemos proporcionar las funciones de CallBack `actionCW` y `actionCCW` que contangan, respectivamente, el código necesario para avanzar un paso en sentido horario y anti horario. 

La clase AsyncStepper está especialmente pensada para funcionar de forma conjunta con controladores como el A4988 o el DRV8825. Este tipo de controladores avanzan un paso al recibir un pulso.

Para que el uso de estos controladores sea más sencillo, la clase AsyncStepper proporciona un constructor que recibe dos pines. En este caso, las acciones de CallBack por defecto se inicializan a la generación de un pulso digital para actuar sobre el controlador.

La actualización la posición del motor paso a paso se debe llamar a la función `Update()`, que comprueba el tiempo transcurrido y llama a las acciones de CallBack en caso necesario. Esta función se deberá invocar lo más frecuentemente posible desde el bucle principal.


### Constructor
La clase AsyncStepper se instancia a través de uno de sus constructores
```c++
AsyncStepper(uint16_t motorSteps, int pinDir, int pinStep)
AsyncStepper(uint16_t motorSteps, StepperCallback actionCW, StepperCallback actionCCW)
```

### Uso de AsyncStepper
```c++
// Mover un cierto ángulo
void Rotate(float angleDelta, StepDirection direction);
void Rotate(float angleDelta, StepDirection direction, StepCallback callback);
  
// Mover hasta un cierto angulo
void RotateToAngle(float angle, StepDirection direction, StepCallback callback);
void RotateToAngle(float angle, StepDirection direction);
 
 // Mover en un cierto tiempo en segundos
 void RotateAngleInTime(float angle, float time, StepperDirection direction, StepperCallback onFinish = nullptr);
 void RotateToAngleInTime(float angle, float time, StepperDirection direction, StepperCallback onFinish = nullptr);
 
 // Mover de forma continua
void RotateContinuos(StepDirection direction);

// Cambiar velocidad y aceleración
void SetSpeed(long speed)
void SetSpeedRpm(float rpm)
void SetSpeedDegreesBySecond(float degreesBySecond)
void SetAcceleration(long acceleration)
void SetAcceleration(long acceleration, long deceleration)

// Detener el motor paso a paso
void Stop();

// Actualiza la posicion del motor paso a paso
// Es necesario llamar a este metodo frecuentemente desde el bucle principal
bool Update();

//Obtener información
long GetRemainSteps() const
unsigned long GetTimeTraveling() const
float GetCurrentAngle() const
long GetCurrentInterval() const
long GetTravelCurrentStep() const
long GetTravelSteps() const
long GetAbsoluteStep() const
long GetMaxSpeed() const
long GetCurrentSpeed() const
float GetCurrentSpeedRpm() const
float GetCurrentSpeedDegreesBySecond() const
float GetSpeedForMove(long steps, float time)
float GetTimeForMove(long steps)
float GetTimeForMove(long steps, unsigned long speed)
```
