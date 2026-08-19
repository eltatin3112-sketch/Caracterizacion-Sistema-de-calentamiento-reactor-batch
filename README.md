# CARACTERIZACIÓN TÉRMICA Y SINTONIZACIÓN PID DEL REACTOR BATCH - GRUPO 5
## Identificación Experimental Térmica y Dinámica de Sistemas - Sistemas de Control II (UNET)

Elaborado por: Daniel Torres y Yermey Castro

---

### 1. OBJETIVO DEL PROYECTO DE CARACTERIZACIÓN TÉRMICA
Este repositorio contiene el firmware, los esquemas de conexión en Fritzing, los scripts de simulación en MATLAB y los datos crudos utilizados para la caracterización termodinámica y el diseño de la estrategia de control PID del Reactor Batch.

El propósito de esta fase experimental es identificar y modelar matemáticamente el comportamiento de transferencia calórica por efecto Joule y convección forzada dentro del reactor. A través de este ensayo se busca:
*   Identificar los parámetros del modelo de Primer Orden con Tiempo Muerto (FOPDT): Ganancia térmica (K), Constante de tiempo (Tau) y Tiempo Muerto (Theta) para las cargas operativas de 2 Litros y 3 Litros.
*   Evaluar el impacto de la agitación mecánica en la transferencia de calor, comparando ensayos de calentamiento bajo convección natural (motor al 0%) frente a convección forzada (motor al 50% y 100% de PWM).
*   Analizar la variación del tiempo muerto (Theta) según la profundidad física del sensor de temperatura (pruebas secuenciales en Fondo, Zona Media y Superficie de la jarra).
*   Calcular las constantes teóricas del controlador PID utilizando las ecuaciones de Cohen-Coon a partir del modelo FOPDT identificado.
*   Simular y justificar en MATLAB el cambio de la sintonización teórica agresiva hacia una sintonización fina suave para evitar el sobrecalentamiento crítico de la solución farmacéutica.

---

### 2. LISTADO DE COMPONENTES UTILIZADOS
Para este ensayo específico de caracterización térmica, se utilizó el siguiente hardware:

*   **Microcontrolador Principal:** ESP32 DevKit V1 (Encargado del cálculo del algoritmo, la telemetría web y el control de potencia).
*   **Actuador de Calor:** Resistencia de inmersión de 110V AC / 800 Watts.
*   **Actuador de Mezcla:** Motorreductor DC tipo TT de 12V acoplado a la hélice del agitador.
*   **Interfaces de Potencia:** Relé de Estado Sólido (SSR-50 DA) para la resistencia y Driver Puente H L298N para el motor de agitación.
*   **Aislamiento de Señal:** Transistor NPN (2N2222) con resistencia de 220 Ohmios para el disparo seguro del SSR.
*   **Sonda de Medición:** Sonda digital sumergible de precisión DS18B20 con bus de datos One-Wire.
*   **Fuentes de Alimentación:** Riel de 12V y 5V de la fuente ATX reciclada (potencia de motor y SSR) y cargador de pared de 5V/2A para la lógica del ESP32.

---


### 3. TABLA DE CONEXIONES Y PINOUT (CABLEADO CON ACOPLAMIENTO DE POTENCIA)
Para garantizar la estabilidad de las señales digitales de control y temperatura frente al ruido electromagnético de la resistencia de 110V AC y los motores, se diseñó un esquema con tierras comunes, cargador independiente para la lógica y disparo aislado por transistor. Las conexiones del módulo ESP32, los drivers y los sensores se estructuran de la siguiente manera:

| Conexión Origen | Señal / Pin | Destino Hardware | Notas Técnicas |
| :--- | :--- | :--- | :--- |
| **Cargador Pared 5V** | `USB` | ESP32 `Micro-USB` | Alimentacion logica aislada de 5V y 2A |
| **Fuente ATX** | `+12V` (Cable Amarillo) | L298N `VCC` | Potencia para el motor TT de agitación |
| **Fuente ATX** | `+5V` (Cable Rojo) | SSR `Pin 3 (+)` | Alimentacion de conmutacion del SSR |
| **Fuente ATX** | `GND` (Cable Negro) | **GND COMÚN** | Union de tierras del sistema en borne L298N |
| **ESP32** | `3V3` | Sonda DS18B20 `VCC` | Alimentacion de datos a 3.3V (cable rojo) |
| **ESP32** | `3V3` | Sensor FC-03 `VCC` | Alimentacion del encoder optico a 3.3V |
| **ESP32** | `GND` | **GND COMÚN** | Conectado al pin GND del ESP32 (cable negro) |
| **ESP32** | `GPIO 04` (D4) | Sonda DS18B20 `DATA` | Bus digital de un solo hilo One-Wire |
| **ESP32** | `GPIO 18` (D18) | Base Transistor 2N2222 | Senal de disparo termico (resistencia 220R) |
| **ESP32** | `GPIO 25` (D25) | L298N `ENA` | Senal PWM de velocidad del agitador |
| **ESP32** | `GPIO 26` (D26) | L298N `IN1` | Senal de sentido de giro del agitador |
| **ESP32** | `GPIO 27` (D27) | L298N `IN2` | Senal de sentido de giro del agitador |
| **ESP32** | `GPIO 33` (D33) | Sensor FC-03 `OUT` | Pin con soporte de interrupciones externas |
| **Transistor 2N2222**| `Emisor` | **GND COMÚN** | Pata izquierda mirando la cara plana |
| **Transistor 2N2222**| `Colector` | SSR `Pin 4 (-)` | Pata derecha. Conmuta el negativo de control |
| **L298N** | `OUT1 / OUT2` | Bornes Motor Agitación | Conexión de potencia directa al motor TT |
| **Enchufe 110V AC** | `Fase` | SSR `Pin 1 (Load)` | Interrupción de potencia de CA de la red |
| **SSR** | `Pin 2 (Load)` | Borne Resistencia | Retorno de potencia de CA hacia el calentador |
| **Enchufe 110V AC** | `Neutro` | Borne Resistencia | Cierre del circuito de CA directo a la red |

#### Notas Críticas de Proteccion Electrica Aplicadas:
*   **Resistencia de Pull-Up:** Se coloco una resistencia de **4.7k Ohmios** entre los pines `3V3` y `GPIO 04` del ESP32 para habilitar de forma estable las lecturas del bus One-Wire.
*   **Aislamiento del SSR:** El terminal positivo `3 (+)` del SSR se conecta directo a los **5V de la fuente ATX** y el negativo al colector del transistor. Esto evita que el ESP32 sufra caidas de voltaje de conmutacion (Brownout).
*   **Tierra de Referencia (GND):** Se unio mediante un puente fisico el pin GND del ESP32 con el borne de tierra del L298N (donde se conecta el negativo de la fuente ATX) para evitar voltajes flotantes en la senal PWM.

---

### 4. ESTRUCTURA DEL PROYECTO (PLATFORMIO)
El software se encuentra organizado bajo el estándar de PlatformIO en Visual Studio Code. Las dependencias lógicas y los archivos de la interfaz web se distribuyen en las siguientes carpetas:

*   **platformio.ini:** Archivo de configuración del entorno (board: esp32dev, framework: arduino, librerías y monitor_speed en 115200).
*   **src/main.cpp:** Código central de la prueba (conteo de pulsos por interrupción, cálculo de RPM, lectura digital de la temperatura por One-Wire y servidor de telemetría asíncrono).
*   **data/index.html:** Interfaz gráfica del usuario (HMI) diseñada en HTML5.
*   **data/app.js:** Lógica de telemetría y actualización de gráficas dinámicas en JavaScript.
*   **data/style.css:** Diseño visual responsivo del panel de control.
*   **data/chart.umd.min.js:** Librería de gráficas alojada localmente en la flash para funcionamiento sin internet.
*   **data/xlsx.full.min.js:** Librería de exportación local para Excel.

---

### 5. INSTRUCCIONES DE DESARROLLO Y CARGA
Siga esta secuencia de comandos en la terminal de VS Code para compilar y subir todo el sistema a la placa ESP32:

1. **Compilar el código fuente:** 
   `pio run`
2. **Subir el Firmware al ESP32:** 
   `pio run --target upload`
3. **Subir la Interfaz Web (LittleFS) a la memoria Flash:** 
   `pio run --target uploadfs`
4. **Abrir Monitor Serial para diagnóstico:** 
   `pio device monitor --baud 115200`

---

### 6. VALIDACIÓN DE LIBRERÍAS LOCALES (MODO OFFLINE)
Para garantizar el funcionamiento del banco de pruebas en entornos industriales o laboratorios sin acceso a internet (como los de la UNET), las librerías de visualización y exportación se cargan directamente desde la memoria interna del ESP32 (LittleFS).

Al ingresar a la interfaz en tu navegador (`192.168.4.1`), verifique el estado en la sección de diagnóstico:
*   **Chart.js local:** cargado
*   **SheetJS local (XLSX):** cargado
*   **Modo offline:** OK

*Nota:* Si alguna librería aparece como "no cargado", asegúrese de haber ejecutado con éxito el comando `uploadfs` en el Paso 3 de la sección anterior.

---

### 7. PROCEDIMIENTO DEL ENSAYO DE CARACTERIZACIÓN TÉRMICA (LAZO ABIERTO)
Este protocolo experimental se ejecutó en **Lazo Abierto** (sin algoritmo de control automático en el ESP32, operando de forma manual la potencia del SSR al 100%) para capturar las curvas de reacción térmicas y analizar los gradientes del fluido:

#### Etapa 1: Ensayos de Homogeneidad Espacial (Sensor en 3 Profundidades)
Para comprobar si la agitación mecánica es efectiva para eliminar los gradientes térmicos, se realizaron tres pruebas secuenciales idénticas bajo una carga de 2 Litros de agua y agitación constante al 50% de PWM, moviendo la altura del sensor DS18B20:
1.  **Prueba A (Fondo):** Fijar la sonda al fondo del tanque (cerca de la resistencia). Resultado: Alcanzó los 40C en **2:51 minutos**.
2.  **Prueba B (Zona Media):** Fijar la sonda en el centro geométrico del fluido. Resultado: Alcanzó los 40C en **2:48 minutos**.
3.  **Prueba C (Superficie):** Fijar la sonda cerca de la superficie del agua. Resultado: Alcanzó los 40C en **2:42 minutos**.
*   **Análisis de Homogeneidad:** La diferencia de apenas **9 segundos** entre el punto más lento y el más rápido valida que la mezcla forzada homogeneiza la temperatura, aproximando el reactor al modelo de parámetros concentrados (mezcla perfecta).

#### Etapa 2: Ensayo de Convección Natural (Motor al 0%) vs. Forzada (Motor al 100%)
1.  **Convección Natural (Motor al 0%):** Con el agitador apagado, aplicar un escalón del 100% de potencia térmica y registrar la curva de temperatura.
2.  **Convección Forzada (Motor al 100%):** Con el motor a máxima velocidad (255 PWM), aplicar el mismo escalón y registrar la curva.
*   **Análisis:** Se demostró que al incrementar la velocidad de agitación se reduce el tiempo muerto (Theta) y disminuye la constante de tiempo (Tau), ya que aumenta el coeficiente global de transferencia de calor (U).

#### Etapa 3: Ensayo de Variación de Masa Térmica (Efecto de Carga)
1.  Cargar el reactor con **3 Litros de agua** (agitación al 100% PWM).
2.  Aplicar un escalón del 100% de calor, registrar la curva de temperatura y descargar el archivo de datos.
*   **Análisis:** Al aumentar el volumen un 50%, la constante de tiempo (Tau) aumentó un 46% (pasando de 92s a 135s), demostrando que la inercia térmica es altamente sensible a la masa del fluido.

---

### 8. PROYECCIÓN AL DISEÑO DEL CONTROLADOR PID EN MATLAB
Los datos crudos recolectados en estos ensayos en lazo abierto permitieron modelar matemáticamente la planta y calcular los parámetros del controlador que se implementarán en la fase de control automático (Tema 4):

*   **Parámetros FOPDT Identificados (Caso 2 Litros):** 
    *   Ganancia (K): 0.18 grados/%.
    *   Constante de tiempo (Tau): 92 segundos.
    *   Tiempo Muerto (Theta): 6 segundos.
*   **Cálculo Teórico de Cohen-Coon:**
    *   **Ganancia Proporcional (Kp):** 114.7
    *   **Tiempo Integral (Ti):** 14.6 segundos
    *   **Tiempo Derivativo (Td):** 2.1 segundos
*   **Sintonización Fina y Simulación en MATLAB:**
    Al simular el modelo teórico en MATLAB, se identificó que la ganancia de 114.7 causaba un sobreimpulso crítico de 17.5C (llegando a 62.5C), lo que degradaría el producto farmacéutico. Por ello, se realizó un ajuste fino conservador en MATLAB para lograr un "aterrizaje" suave de temperatura a los 45C sin sobrepasos, sintonizando el PID final con: **Kp = 35.0, Ki = 0.024 (Ti = 40s) y Kd = 5.0 (Td = 5s)**.

---

### 9. INTERPRETACIÓN DE RESULTADOS OBTENIDOS (ANÁLISIS)
A partir del procesamiento de los datos descargados en formato CSV de las pruebas de lazo abierto y su modelado en MATLAB, se identificaron los siguientes comportamientos termodinámicos:

#### A. Ganancia Térmica (K) y Eficiencia del Calefactor
Se identificó una ganancia constante de **0.18 grados Celsius / % de potencia** para ambas cargas (2L y 3L). Esto demuestra que la ganancia es una propiedad estática ligada estrictamente a la potencia de la resistencia de inmersión y a las pérdidas de calor ambientales. El sistema es altamente eficiente, permitiendo predecir que con el 100% de potencia el reactor es térmicamente capaz de superar con creces la meta de 45C de diseño.

#### B. Impacto de la Masa en la Constante de Tiempo (Tau)
Los ensayos de carga demostraron la alta sensibilidad del reactor a la masa térmica:
*   **Con 2 Litros:** La constante de tiempo (Tau) fue de **92 segundos**.
*   **Con 3 Litros:** Al aumentar el volumen en un 50%, la constante de tiempo subió a **135 segundos (un incremento del 46%)**.
*   **Análisis:** Esto confirma experimentalmente que la velocidad de respuesta del reactor es inversamente proporcional al volumen de fluido acumulado (Vessel Holdup), lo que requiere que el futuro controlador PID sea sintonizado considerando el peor escenario de inercia (3L) para evitar oscilaciones en cargas menores.

#### C. Variación del Tiempo Muerto (Theta) por Profundidad del Sensor
Se demostró que el retardo de transporte (tiempo muerto) no es una constante electrónica, sino una variable física influenciada por la hidrodinámica del tanque:
*   **Sensor en el Fondo (Cerca de resistencia):** Registró un tiempo de 2 minutos 51 segundos para llegar a 40C.
*   **Sensor en la Superficie (Lejos de resistencia):** Registró un tiempo de 2 minutos 42 segundos.
*   **Análisis:** Al estar el motor encendido, se genera un flujo ascendente de agua caliente por convección forzada que sube rápidamente, lo que explica por qué la superficie calienta ligeramente más rápido (9 segundos de diferencia). Esta brecha mínima valida experimentalmente que la agitación destruye de forma efectiva los gradientes térmicos y permite asumir un modelo de **mezcla perfecta**.

#### D. Comparativa Dinámica del PID en Lazo Cerrado (MATLAB)
Al simular la sintonización del PID en MATLAB para analizar el comportamiento del lazo cerrado en el futuro diseño de control, se obtuvieron las siguientes conclusiones:
*   **Cohen-Coon Teórico (Agresivo):** Al aplicar $Kp = 114.7$, el sistema responde de forma violenta, provocando un sobreimpulso térmico crítico de **17.5 grados Celsius (alcanzando una temperatura pico de 62.5C)** que degradaría químicamente la solución farmacéutica.
*   **Sintonización Fina Suave (Cargada en el ESP32):** Al reducir la ganancia a **$Kp = 35.0$**, aumentar el tiempo integral a **$Ti = 40.0s$** ($Ki=0.024$) y la acción derivativa a **$Td = 5.0s$**, la temperatura real se estabiliza de forma plana en 45C en **150 segundos (2.5 minutos)** con un sobreimpulso seguro de solo **2.0 grados Celsius** (pico de 47C) y error estacionario cero.
