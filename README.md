# PICMCU LAB1 pablinza@me.com
Programacion del Microcontrolador PIC16F887 LAB1

<p align="center">
  <img src="/images/pic1.png"></img>
</p>
Carpeta con los proyectos generados por MPLABX utilizando el compilador XC8 para el modelo:
- PIC16F887 a 8MHz, utilamos la placa B8P40 descrita en el siguinte enlace [Click Aqui](https://github.com/pablinza/elt436)
Los archivos con extension .c corresponden al codigo fuente de un programa o practica
Los archivos con extension .h y .c con mismo nombre, son procedimientos y funciones basicas
Los archivos con extension .hex representan el codigo de maquina generado para el PIC
Los archivos con extension .png corresponden a una imagen de esquema de circuito

Los proyectos se crearon con la version MPLABX 6.25 y el compilador XC8 2.50 version FREE
Practicas de Programacion para Microcontroladores PIC16F.

## Lista de practicas desarrolladas en la Materia
### - P01 - Destello de Luz Secuencial   [APPBASE]
  <p align="center">
  <img src="/images/ap1base.png"></img>
</p>
El programa destella los leds conectados el PIC con frecuencia y ciclo de trabajo diferente, se hace uso del temporizador TMR0 para realizar las tareas de forma concurrente sin bloquear le ejecucion del programa principal. EL modulo TMR0 esta configurado para temporizar intervales de 1ms.

### - P02 - Control de Trafico Vehicular [APTRAFFIC]
<p align="center">
  <img src="/images/ap2traffic.png"></img>
</p>
El programa muestra como implementar un contrlador de trafico para dos fases, utilizando como base de tiempo el temporizador TMR0. Ademas se cuenta con una entrada de alerta SW1 que en caso de fall obliga al controlador a ingreear al modo de mantenimiento en el cual las luces amarillas destellaran. 

### - P03 - Monitor de Datos Serial ADC  [APMONITOR]
<p align="center">
  <img src="/images/ap3monitor.png"></img>
</p>
Este programa realiza la lectura de los canales analogicos AN0 y AN1, y asi mismo lee el estado del pulsador SW1 de manera concurrente, ademas se enviara por el puerto serial UART la magnitud de las conversiones y el estado del pulsador, en la practica se utilizara un joystick de doble eje y una interfaz graficas para visualizar la informaicon. 

### - P04 - Controlador para Servomotor  [APSERVO]
<p align="center">
  <img src="/images/ap4servo.png"></img>
</p>
El programa permite controladr la posicion de un servomotor del tipo SG90, el control se lleva a cabo en porcentaje 0 al 100% del movimiento, Ademas se efectuara la lectura de un sensor de distancia por ultrasonido para detectar la presencia de un objeto y desplazar una barra utilizando el sermovotor. 

### - P05 - Control de Acceso Autorizado [APKEYPAS]
<p align="center">
  <img src="/images/ap5keypas.png"></img>
</p>
Programa basico que muestra como utilizar un telcado matricial para ingresar datos y una pantalla del tipo LCD, el proceso permite ingresar de manera controlada una clave de acceso de cuatro digitos para luego validar el dato y mostrar si la clave fue aceptada o rechazada.

Adjunto el siguiente link que muestra como compilar estos ejemplos en MPLABX
[![](http://img.youtube.com/vi/w-GRu89glrg/0.jpg)](http://www.youtube.com/watch?v=w-GRu89glrg "Compilar en MPLABX")



<img src="/images/pablo_ueb.png" align="left" width="200px"/>
Autor: Pablo Zarate, puedes contactarme a pablinza@me.com / pablinzte@gmail.com.  <br />
Visita mi Blog  [Blog de Contenidos](https://pablinza.blogspot.com/). <br />
Visita mi Canal [Blog de Contenidos](http://www.youtube.com/@pablozarate7524)). <br />
Facultad de Ciencias y Tecnologia UEB - Ingenieria Electronica. <br />
Santa Cruz - Bolivia 
<br clear="left"/>
***********************

