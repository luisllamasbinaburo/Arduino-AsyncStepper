# Librería Arduino AsyncStepper
Librería para Arduino que permite mover un motor paso a paso de forma no bloqueante. De esta forma se tiene un motor paso a paso que tiene un cierto comportamiento "asíncrono".

Más información https://www.luisllamas.es/libreria-arduino-asyncstepper/

## Instrucciones de uso
La clase AsyncStepper implementa un motor paso a paso cuyos movimientos están temporizados, en lugar de ser bloqueantes. El objetivo de la librería es poder mover uno o varios motores paso a paso en un proyecto, incluso a diferentes velocidades, mientras que no se impide la ejecución otras tareas en el bucle de control principal.

La clase AsyncStepper no realiza el control del motor paso a paso, únicamente realiza la temporaización de los pasos. Esto permite que sea muy versatil, ya que es independiente del motor paso a paso o controlador empleado.

Para conseguir controlar el motor paso a paso, debemos proporcionar las funciones de CallBack `actionCW' y `actionCCW` que contangan, respectivamente, el código necesario para avanzar un paso en sentido horario y anti horario. 

La clase AsyncStepper está especialmente pensada para funcionar de forma conjunta con controladores como el A4988 o el DRV8825. Este tipo de controladores avanzan un paso al recibir un pulso.

Para que el uso de estos controladores sea más sencillo, la clase AsyncStepper proporciona un constructor que recibe dos pines. En este caso, las acciones de CallBack por defecto se inicializan a la generación de un pulso digital para actuar sobre el controlador.

La actualización la posición del motor paso a paso se debe llamar a la función `Update()`, que comprueba el tiempo transcurrido y llama a las acciones de CallBack en caso necesario. Esta función se deberá invocar lo más frecuentemente posible desde el bucle principal.


### Constructor
La clase AsyncStepper se instancia a través de uno de sus constructores
```c++
AsyncStepper(uint16_t motorSteps, int pinCW, int pinCCW);
AsyncStepper(uint16_t motorSteps, StepCallback actionCW, StepCallback actionCCW);
```

### Uso de AsyncStepper
```c++
// Mover un cierto ángulo
void Rotate(float angleDelta, StepDirection direction);
void Rotate(float angleDelta, StepDirection direction, StepCallback callback);
  
// Mover hasta un cierto angulo
void RotateToAngle(float angle, StepDirection direction, StepCallback callback);
void RotateToAngle(float angle, StepDirection direction);
 
 // Mover de forma continua
void RotateContinuos(StepDirection direction);

// Cambiar velocidad del motor paso a paso
void SetSpeedRpm(float rpm);
void SetSpeedDegreesBySecond(float degreesBySecond);
void SetSpeedRadiansBySecong(float radiansBySecond);
void SetMetersBySecond(float metersBySecond, float radius);

// Detener el motor paso a paso
void Stop();

// Actualiza la posicion del motor paso a paso
// Es necesario llamar a este metodo frecuentemente desde el bucle principal
bool Update();

//Obtener el angulo actual
float GetCurrentAngle() const;

// Funciones de Callback para el avance de un paso
StepCallback ActionCW;
StepCallback ActionCCW;

// Pines de salida para uso con controladores paso a paso externos
int PinCW;
int PinCCW;
```

## Ejemplos
La librería AsyncStepper incluye los siguientes ejemplos para ilustrar su uso.

* AsyncStepper: Ejemplo que demuestra el control de un motor paso a paso sin controlador

```c++
#include "AsyncStepperLib.h"
#include <Stepper.h>

const int motorPin1 = 8;  
const int motorPin2 = 9;  
const int motorPin3 = 10; 
const int motorPin4 = 11; 
const int numSteps = 8;
const int stepsLookup[8] = { B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001 };
int stepCounter = 0; 

void clockwise()
{
	stepCounter++;
	if (stepCounter >= numSteps) stepCounter = 0;
	setOutput(stepCounter);
}

void anticlockwise()
{
	stepCounter--;
	if (stepCounter < 0) stepCounter = numSteps - 1;
	setOutput(stepCounter);
}

void setOutput(int step)
{
	digitalWrite(motorPin1, bitRead(stepsLookup[step], 0));
	digitalWrite(motorPin2, bitRead(stepsLookup[step], 1));
	digitalWrite(motorPin3, bitRead(stepsLookup[step], 2));
	digitalWrite(motorPin4, bitRead(stepsLookup[step], 3));
}

const int stepsPerRevolution = 4076;
AsyncStepper stepper1(stepsPerRevolution,
	[]() {clockwise(); },
	[]() {anticlockwise(); }
);

void rotateCW()
{
	stepper1.Rotate(90, AsyncStepper::CW, rotateCCW);
}

void rotateCCW()
{
	stepper1.Rotate(90, AsyncStepper::CCW, rotateCW);
}

void setup()
{
	Serial.begin(9600);

	pinMode(motorPin1, OUTPUT);
	pinMode(motorPin2, OUTPUT);
	pinMode(motorPin3, OUTPUT);
	pinMode(motorPin4, OUTPUT);

	stepper1.SetSpeedRpm(10);
	stepper1.RotateContinuos(AsyncStepper::CCW);
}

void loop()
{
	stepper1.Update();
}
```
