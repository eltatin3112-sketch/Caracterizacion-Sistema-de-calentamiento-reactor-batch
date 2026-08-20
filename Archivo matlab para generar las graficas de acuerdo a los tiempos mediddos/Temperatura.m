% SCRIPT MAESTRO DE MATLAB - SIMULACIÓN DE ENSAYOS DE LAZO ABIERTO
% GRUPO 5 - REACTOR BATCH (TEMA 2 / TEMA 5)
% Este script genera las dos graficas fundamentales de la caracterizacion termica

clear all; clc; close all;

% =========================================================================
% --- SECCIÓN DE CALIBRACIÓN DE PARÁMETROS (DATOS REALES DEL INFORME) ---
% =========================================================================

% 1. Parametros para la comparacion de cargas (Grafica 1 - 2L vs 3L)
K_nominal  = 0.18;    
tau_2L     = 92;      % Constante de tiempo de 2 Litros (segundos)
theta_2L   = 6;       % Tiempo muerto de 2 Litros (segundos)
tau_3L     = 135;     % Constante de tiempo de 3 Litros (segundos)
theta_3L   = 7;       % Tiempo muerto de 3 Litros (segundos)

% 2. Parametros para la comparacion espacial de 3 profundidades (Grafica 2 - 2L)
% Sensor en el Fondo (TT-101) -> Tardo 2:51 min (171s) en llegar a 40C
K_fondo    = 0.175;   
tau_fondo  = 92;      
theta_fondo = 4;      

% Sensor en la Mitad (TT-102) -> Tardo 2:48 min (168s) en llegar a 40C
K_medio    = 0.173;   
tau_medio  = 91;      
theta_medio = 3;      

% Sensor en el Principio/Superficie (TT-103) -> Tardo 2:42 min (162s) en llegar a 40C
K_principio = 0.170;  
tau_principio = 90;   
theta_principio = 1;  

% =========================================================================
% --- PROCESAMIENTO MATEMÁTICO Y GENERACIÓN DE GRÁFICAS ---
% =========================================================================

% --- GRÁFICA 1: COMPARATIVA DE CARGAS (2 LITROS VS 3 LITROS) ---
num_2L = [K_nominal]; den_2L = [tau_2L, 1];
G_2L = tf(num_2L, den_2L, 'InputDelay', theta_2L);

num_3L = [K_nominal]; den_3L = [tau_3L, 1];
G_3L = tf(num_3L, den_3L, 'InputDelay', theta_3L);

t1 = 0:1:400;
[y_2L, t1] = step(100 * G_2L, t1); % Escalon al 100% de potencia (Lazo Abierto)
[y_3L, t1] = step(100 * G_3L, t1);

temp_2L = y_2L + 25.0; % Sumar temperatura inicial de 25C (Ambiente)
temp_3L = y_3L + 25.0;

figure('Name', 'Grafica 1: Comparativa de Cargas 2L vs 3L', 'Color', [1 1 1]);
plot(t1, temp_2L, 'b-', 'LineWidth', 2.5); hold on;
plot(t1, temp_3L, 'r-', 'LineWidth', 2.5);
plot([0 400], [43 43], 'k--', 'LineWidth', 1.2);
title('Comparativa de Curvas de Reaccion del Reactor (Lazo Abierto 100% Potencia)');
xlabel('Tiempo (Segundos)'); ylabel('Temperatura (C)');
legend('Caso Nominal: 2 Litros (Tau = 92s, Theta = 6s)', ...
       'Caso Perturbado: 3 Litros (Tau = 135s, Theta = 7s)', ...
       'Temperatura de consigna aproximada', 'Location', 'Southeast');
grid on;


% --- GRÁFICA 2: COMPARATIVA ESPACIAL DE 3 POSICIONES Y DELTA T (2 LITROS) ---
num_f = [K_fondo]; den_f = [tau_fondo, 1];
G_fondo = tf(num_f, den_f, 'InputDelay', theta_fondo);

num_m = [K_medio]; den_m = [tau_medio, 1];
G_medio = tf(num_m, den_m, 'InputDelay', theta_medio);

num_p = [K_principio]; den_p = [tau_principio, 1];
G_principio = tf(num_p, den_p, 'InputDelay', theta_principio);

t2 = 0:1:160;
[y_f, t2] = step(100 * G_fondo, t2);
[y_m, t2] = step(100 * G_medio, t2);
[y_p, t2] = step(100 * G_principio, t2);

temp_f = y_f + 22.0; % Sumar temperatura inicial de 22C
temp_m = y_m + 22.0;
temp_p = y_p + 22.0;
delta_t = temp_f - temp_p; % Diferencia maxima (Fondo - Superficie)

figure('Name', 'Grafica 2: Homogeneidad de Mezcla (2L)', 'Color', [1 1 1]);
% Subplot Superior: Las 3 curvas de temperatura juntas
subplot(2, 1, 1);
plot(t2, temp_f, 'r-', 'LineWidth', 2.2); hold on;
plot(t2, temp_m, 'm-', 'LineWidth', 2.2);
plot(t2, temp_p, 'b-', 'LineWidth', 2.2);
title('Comparativa Espacial: Fondo vs Mitad vs Principio (2L)');
ylabel('Temperatura (C)');
legend('Fondo (2:51 min)', 'Mitad (2:48 min)', 'Principio/Superficie (2:42 min)', 'Location', 'Southeast');
grid on;

% Subplot Inferior: Delta T (Diferencia)
subplot(2, 1, 2);
plot(t2, delta_t, 'g-', 'LineWidth', 2.5);
title('Diferencia de Temperatura entre Sensores (Grado de Homogeneidad)');
xlabel('Tiempo (segundos)'); ylabel('Diferencia (C)');
legend('Delta T (Fondo - Principio)', 'Location', 'Northeast');
grid on;