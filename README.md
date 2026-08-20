# CARACTERIZACIÓN TÉRMICA Y SINTONIZACIÓN PID DEL REACTOR BATCH - GRUPO 5
## Identificación Experimental Térmica y Dinámica de Sistemas - Sistemas de Control II (UNET)

Elaborado por: Daniel Torres y Yermey Castro

---

### 1. OBJETIVO DEL PROYECTO DE CARACTERIZACIÓN TÉRMICA
Este repositorio documenta la caracterización del lazo de calentamiento de un Reactor Batch a escala. Su objetivo es emular el acondicionamiento térmico de una mezcla farmacéutica utilizando una resistencia de inmersión de 110V AC.

El objetivo de esta fase en *Sistemas de Control II* es identificar experimentalmente los parámetros termodinámicos de la planta (Ganancia `K`, Constante de Tiempo `Tau` y Tiempo Muerto `Theta`) aplicando el método de la curva de reacción en lazo abierto. Estos datos son obligatorios para el posterior diseño y sintonización de un controlador PID que permita estabilizar el proceso de forma 

---

### 2. Hardware Mínimo Requerido

| Componente | Referencia / Modelo | Cantidad | Función en el sistema |
| :--- | :--- | :--- | :--- |
| **Microcontrolador** | ESP32 DevKit V1 | 1 | Procesamiento, temporización del SSR y servidor web. |
| **Sensor Térmico** | Sonda DS18B20 | 1 | Medición digital de temperatura en el fluido. |
| **Actuador Térmico**| Resistencia Inmersión | 1 | Aporte calórico (800W nominales). |
| **Interfaz Potencia**| Relé Estado Sólido SSR-50DA | 1 | Conmutación rápida de los 110V AC. |
| **Acoplamiento DC** | Transistor NPN 2N2222 | 1 | Interfaz de disparo de baja corriente para proteger el ESP32. |
| **Resistencias** | 4.7kΩ y 220Ω | 1 c/u | Pull-up para bus de datos (4.7k) y límite de base (220R). |
| **Fuentes de Poder** | ATX de PC y Cargador 5V| 1 c/u | ATX entrega 5V al SSR. Cargador aísla la lógica del ESP32. |

---

### 3. TABLA DE CONEXIONES Y PINOUT (CABLEADO CON ACOPLAMIENTO DE POTENCIA)
| Origen | Señal / Pin | Destino Hardware | Notas Técnicas de Conexión |
| :--- | :--- | :--- | :--- |
| ESP32 | `3V3` | DS18B20 `VCC` | Alimentación de sonda (Cable rojo). |
| ESP32 | `GPIO 04` | DS18B20 `DATA`| Bus One-Wire (Cable amarillo/azul). **Requiere R de 4.7kΩ a 3V3.** |
| ESP32 | `GPIO 18` | Transistor `Base`| Señal de disparo de calor (A través de R de 220Ω). |
| Transistor | `Emisor` | **GND COMÚN** | Pata izquierda hacia tierra. |
| Transistor | `Colector`| SSR `Pin 4 (-)`| Pata derecha. Disparo lógico (Conmuta el negativo). |
| Fuente ATX | `+5V` (Rojo) | SSR `Pin 3 (+)` | Alimenta el SSR directamente con 5V, evitando cargar al ESP32. |
| Red 110V AC | `Fase (Línea)` | SSR `Pin 1 (Load)` | Interrupción de la red de corriente alterna (Tras pasar por fusible). |
| SSR | `Pin 2 (Load)` | Resistencia | Retorno de fase conmutada hacia el calentador. |
| Red 110V AC | `Neutro` | Resistencia | Cierre del circuito AC directo al tomacorriente. |

*Nota sobre "Aislamiento":* El uso del transistor 2N2222 sirve como etapa de acoplamiento de corriente (driver) para evitar caídas de tensión en los pines del ESP32. Dado que el Emisor del transistor comparte el GND del ESP32 y de la fuente ATX, **no existe aislamiento galvánico en el lado de DC**, pero la electrónica de baja tensión queda protegida por el aislamiento óptico interno que posee el propio módulo SSR-50 DA respecto al lado de 110V AC.

*(El diagrama eléctrico detallado en formato .fzz de Fritzing se encuentra dentro de la carpeta `Diagrama electrico Caracterizacion temperatura` de este repositorio).*
---


### 4. Cómo Descargar, Instalar y Ejecutar desde Cero

### A. Preparación y Compilación
1. Descargue este repositorio (Botón **"Code" -> "Download ZIP"**) o clónelo vía Git.
2. Abra la carpeta en **Visual Studio Code** con la extensión **PlatformIO** instalada.
3. Asegúrese de que el archivo `platformio.ini` esté en la raíz e incluya las librerías `OneWire` y `DallasTemperature`.
4. Conecte el ESP32, abra la terminal de PIO y ejecute la carga del firmware:
   `pio run --target upload`

### B. Puesta en Marcha (¡Precaución!)
1. **Llene el reactor con al menos 2 Litros de agua ANTES de continuar.**
2. Enchufe la etapa de control (ESP32 a 5V). 
3. *Solo después de validar el nivel de agua*, enchufe la clavija de 110V AC a la pared.
4. Conéctese a la red WiFi **Reactor_Batch_G5** (`control_industrial`) e ingrese a **`http://192.168.4.1`**.
5. Desde la HMI, varíe la potencia térmica y monitorice la gráfica.
---

### 5. VALIDACIÓN DE LIBRERÍAS LOCALES (MODO OFFLINE)
Para garantizar el funcionamiento del banco de pruebas en entornos industriales o laboratorios sin acceso a internet (como los de la UNET), las librerías de visualización y exportación se cargan directamente desde la memoria interna del ESP32 (LittleFS).

Al ingresar a la interfaz en tu navegador (`192.168.4.1`), verifique el estado en la sección de diagnóstico:
*   **Chart.js local:** cargado
*   **SheetJS local (XLSX):** cargado
*   **Modo offline:** OK

*Nota:* Si alguna librería aparece como "no cargado", asegúrese de haber ejecutado con éxito el comando `uploadfs` en el Paso 3 de la sección anterior.

---

### 6. PROCEDIMIENTO DEL ENSAYO DE CARACTERIZACIÓN TÉRMICA (LAZO ABIERTO)
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

### 7. PROYECCIÓN AL DISEÑO DEL CONTROLADOR PID EN MATLAB
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

### 8. INTERPRETACIÓN DE RESULTADOS OBTENIDOS (ANÁLISIS)
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

  ---

   9. Problemas Frecuentes (Troubleshooting)

*   **El sensor marca `-127.0 C` o se congela el sistema:** El cable de datos del DS18B20 está suelto o falta colocar la resistencia de Pull-Up de 4.7kΩ entre VCC y DATA.
*   **La página web no carga:** Ocurre si el ESP32 se "atasca" esperando respuesta del sensor térmico. Revise las conexiones del sensor y asegúrese de que el código tenga la función `sensors.setWaitForConversion(false)` para un manejo asíncrono.
*   **El SSR no se enciende (LED del SSR apagado):** Revise que la fuente ATX esté suministrando 5V al Pin 3 del SSR y que el transistor 2N2222 esté conectado con la polaridad correcta (Emisor a GND, Colector al SSR).
*   **El SSR se enciende pero el agua no calienta:** Falla en la etapa de potencia AC. Revise si se quemó el fusible de 10A o si el tomacorriente de pared tiene suministro.

  ---

 10. ADVERTENCIA CRÍTICA DE SEGURIDAD (110V AC)
**¡PELIGRO DE ELECTROCUCIÓN Y QUEMADURAS SEVERAS!**
Este circuito maneja voltajes letales de la red doméstica (110V AC) y corrientes cercanas a los 7.5 Amperios.
*   **Aislamiento AC:** La etapa de 110V debe estar en un bloque de terminales protegido. **NUNCA toque los bornes del SSR (Pines 1 y 2) ni la resistencia de inmersión mientras el equipo esté conectado al tomacorriente.**
*   **Marcha en Seco:** NUNCA encienda la resistencia eléctrica de inmersión si no está completamente cubierta por al menos 1.0 Litro de agua. Si se enciende en el aire, se fundirá en segundos y puede causar un incendio.
*   **Protección por Fusible:** Es obligatorio colocar un **fusible de 10A de acción rápida** en serie con el cable de Fase (Línea) antes de entrar al SSR.
*   **Apagado Seguro:** En caso de emergencia, la desconexión debe hacerse desenchufando físicamente la clavija de 110V de la pared, no confiando únicamente en la interrupción lógica del SSR.
