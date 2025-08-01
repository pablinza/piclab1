# PICMCU LAB1 pablinza@me.com
Programacion del Microcontrolador PIC16F887 LAB1, este es repositorio con las carpetas de proyectos generados por MPLABX utilizando el compilador XC8 y el PIC16F887 durante la actividad practica de la materia Microcontroladores PIC <br />
El software __MPLABX__ disponible en la pagina del fabricante Microchip [ --> Click](https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/SoftwareTools/MPLABX-v6.20-windows-installer.exe?authuser=0) <br />
El compilador __XC8__ puedes descargalo utilizando este enlace [ --> Click](https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/SoftwareTools/xc8-v2.50-full-install-windows-x64-installer.exe?authuser=0) <br />

Para cargar el firmware al microcontrolador necesitaras un programador ICSP, como alternativa se utiliza el software __SimulIDE__ [ -->Click](https://simulide.com/p/) a efectos de verificar el funcionamiento. Cada carpeta del proyecto MPLABX tiene el nombre precedido por el numero de actividad y en su estructura encontrara el programa principal con el nombre __main.c__ y librerias de uso local, una vez compilado el codigo fuente, se generara el firmware archivo __.hex__ en la carpeta dist/default/production. <br />

Utilizaremos la placa B8P40 que lleva un Cristal HS de 20MHz, un LED, Pulsador Reset y Puerto para comunicacion UART, en todos los casos la programacion del PIC16F887 sera utilizando el oscilador interno RC a 8MHz, salvo casos donde se requiera mas velocidad se usara el HS a 20MHz. En caso de utilizar un __bootloader__, se debe reservar el uso de memoria en MPLABX. <br />

## ESQUEMAS DEL CIRCUITO PIC16F887 PARA IMPLEMENTAR
Descripcion de la Placa B8P40 utilizada para los ejercico de programacion en Laboratorio U.E.B Asignatura ELT
<p align="center">
  <img src="/images/b8p40.png"></img>
</p>

### - BLoque 1 - Conexiones del Microcontrolador
  <p align="center">
  <img src="/images/b8p40_sch_p1.png"></img>
</p>

Programa base __main.c__ para destellar un LED y que se utilizara en todas las practicas de este repositorio.

```c
#pragma config FOSC=INTRC_NOCLKOUT, WDTE = OFF, BOREN = OFF, LVP = OFF
#include <xc.h>
#define LEDpin PORTEbits.RE2 //Led de la tarjeta
volatile __bit tickms;
void setupMCU(void); //Configuracion del PIC
void taskLED(void); //Tarea para destellar el LED
void __interrupt() isr(void) //Rutina de servicio a la Interrupcion
{
    if(INTCONbits.T0IF) //Evento del temporizador 0.001s
    {
        INTCONbits.T0IF = 0;
        TMR0 += 131; //Recarga el contador
        tickms = 1;
    }
}
void main(void) 
{
    setupMCU();
    while(1)
    {
        if(tickms)
        {
            tickms = 0;
            taskLED(); //Destella LED
        }
    }
}
void setupMCU(void)
{
    OSCCONbits.IRCF = 0b111; //Oscilador Interno 8MHz
    while(OSCCONbits.HTS == 0);
    ANSEL = 0; //Desactiva pines ADC AN0 al AN7
    ANSELH = 0;//Desactiva pines ADC AN8 al AN13
    TRISEbits.TRISE2 = 0; //Salida LED
    OPTION_REGbits.nRBPU = 0; //Activa las pull-ups PORTB
    /* CONFIGURACION TIMER0 1MS a Fosc=8MHz*/
    OPTION_REGbits.T0CS = 0;//Modo Termporizador
    OPTION_REGbits.PSA = 0; //Con prescala
    OPTION_REGbits.PS = 0b011; //Prescala 1:16
    TMR0 = 131; //256-(time/((pre)*(4/Fosc))) time=0.001 seg
    INTCONbits.T0IF = 0; //Limpia bandera
    INTCONbits.T0IE = 1; //Activa interrupcion del TMR0
    INTCONbits.GIE = 1; //Habilitador global ISR
}
void taskLED(void) //Destello de LED1 1Hz al 20%
{
    static uint16_t cnt = 0;
    if(cnt++ > 999) 
    {
        cnt = 0;
        LEDpin = 1; //Activa LED
    }
    if(cnt == 200) LEDpin = 0; //Apaga LED
}
```
<br />
Adjunto el siguiente link que muestra como compilar estos ejemplos en MPLABX (http://www.youtube.com/watch?v=w-GRu89glrg)

## Lista de practicas desarrolladas en la Materia
### - P01 - Destello de Luz Secuencial   [APPBASE]
  <p align="center">
  <img src="/images/ap1base.png"></img>
</p>
El programa destella los leds conectados el PIC con frecuencia y ciclo de trabajo diferente, se hace uso del temporizador TMR0 para realizar las tareas de forma concurrente sin bloquear le ejecucion del programa principal. EL modulo TMR0 esta configurado para temporizar intervalos de 1ms. Para descripcion(https://youtu.be/rYyD9MK57Po)

### - P02 - Control de Trafico Vehicular [APTRAFFIC]
<p align="center">
  <img src="/images/ap2traffic.png"></img>
</p>
El programa muestra como implementar un contrlador de trafico para dos fases, utilizando como base de tiempo el temporizador TMR0. Ademas se cuenta con una entrada de alerta SW1 que en caso de fall obliga al controlador a ingreear al modo de mantenimiento en el cual las luces amarillas destellaran. Para descripcion(https://youtu.be/hlDj-E___kY)

### - P03 - Monitor de Datos Serial ADC  [APMONITOR]
<p align="center">
  <img src="/images/ap3monitor.png"></img>
</p>
Este programa realiza la lectura de los canales analogicos AN0 y AN1, y asi mismo lee el estado del pulsador SW1 de manera concurrente, ademas se enviara por el puerto serial UART la magnitud de las conversiones y el estado del pulsador, en la practica se utilizara un joystick de doble eje y una interfaz graficas para visualizar la informaicon. Para descripcion(https://youtu.be/-FM--FO0nE8)

### - P04 - Controlador para Servomotor  [APSERVO]
<p align="center">
  <img src="/images/ap4servo.png"></img>
</p>
El programa permite controladr la posicion de un servomotor del tipo SG90, el control se lleva a cabo en porcentaje 0 al 100% del movimiento, Ademas se efectuara la lectura de un sensor de distancia por ultrasonido para detectar la presencia de un objeto y desplazar una barra utilizando el sermovotor. Para descripcion(https://youtu.be/369__Zxra0M)

### - P05 - Control de Acceso Autorizado [APKEYPAS]
<p align="center">
  <img src="/images/ap5keypas.png"></img>
</p>
Programa basico que muestra como leer desde un teclado matricial 3x4 una contraseña de cuatro digitos y mostrar su validacion en una pantalla tipo LCD 16x2, durante el proceso la pantalla muestra el ingreso de los datos y un mensaje para indicar que la contraseña fue aceptada o rechazada, despues de tres intentos fallidos el acceso se bloqueara hasta reiniciar el programa. Para descripcion(https://youtu.be/mYJeusZr0Jg)

### - P06 - Velocidad y Sentido de un Motor DC [APDCMOTOR]
<p align="center">
  <img src="/images/ap6dcmotor.png"></img>
</p>
Este programa permite controlar la velodidad de giro y el sentido o direccion de un motor tipo DC, donde se utiliza el modulo CCP del PIC para generar una señal de control PWM con frecuencia superior a los 20Khz. La conexion del motor se llevara a cabo utilizando un puente H con el circuito integrado L293D. Para descripcion(https://youtu.be/J0C-R59uOTY)

### - P07 - Control con Tarjeta RFID [APRFIDCOM]
<p align="center">
  <img src="/images/ap7rfidcom.png"></img>
</p>
Se muestra como utilizar el modulo lector de tarjetas chip RFID RC522 a travez del puerto serie SPI del Microcontrolador, el programa realizara la lectura de una tarjeta RFID y enviara a traves del puerto serie UART, mensajes con el numero serial de la tarjeta y un valor decimal que representa los creditos asociados a la tarjeta, dependiendo de la cantidad de credito disponible, una luz roja y verde indicara si el acceso esta autorizado. Para descripcion (https://youtu.be/GMP5CGrTA_I)

### - P08 - Temporizador con Reloj RTC [APRTCCOM]
<p align="center">
  <img src="/images/ap8rtcalarm.png"></img>
</p>
Este programa hace uso del protocolo I2C para la comunicacion con un RTC DS1307, el objetivo es configurar la fecha y hora del RTC mediante comandos recibidos por el puerto serie UART, asi como establecer una fecha y hora limite para la activacion de un LED. La configuracion de la fecha y hora limite se almacenara en la memoria de datos EEPROM del PIC. Para descripcion (https://youtu.be/NIsh_5g4Zro)

### - P09 - Recepcion de datos GPS [APGPSLOG]
<p align="center">
  <img src="/images/ap9gpslive.png"></img>
</p>
Programa que utiliza el puerto serial UART para recibir los mensajes de un modulo GPS, los mensajes NMEA emitidos por el receptor GPS son decodificados para extraer la posicion en coordenadas Latitud, Longitud que se muestran en una pantalla LCD utilizado el modulo I2C (PCF). Para descripcion (https://youtu.be/aT-w1kmcUjA)

### - P10 - Control Remoto por Infrarojo IR [APREMOTEIR]
<p align="center">
  <img src="/images/ap10remoteir.png"></img>
</p>
Este programa muestra como utilizar el kit HX1838 control remoto y receptor de infrarojos Keyes-022, donde el PIC16F887 decodifica la trama de datos IR con protocolo NEC haciendo uso de interrupciones por cambio de estado.  En funcion de los botones presionados desde el control remoto se activaran y desactivaran cuatro luces tipo LED. Para descripcion (https://youtu.be/r2r3zhkY5MA) <br />

Adjunto el siguiente link que muestra como compilar estos ejemplos en MPLABX <br />
[![](http://img.youtube.com/vi/w-GRu89glrg/0.jpg)](http://www.youtube.com/watch?v=w-GRu89glrg "Compilar en MPLABX")



<img src="/images/pablo_ueb.png" align="left" width="200px"/>
Autor: Pablo Zarate, puedes contactarme a pablinza@me.com / pablinzte@gmail.com.  <br />
Visita mi Blog  [Blog de Contenidos](https://pablinza.blogspot.com/). <br />
Visita mi Canal [Blog de Contenidos](http://www.youtube.com/@pablozarate7524)). <br />
Santa Cruz - Bolivia 
<br clear="left"/>
***********************

