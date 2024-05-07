% FUNDAMENTALS OF POWER ELECTRONICS, 2020 - MAKSIMOVIC, ERICKSON 
% Fig. 19.12
% A MATLAB script that generates the analog and digital loop gain
% Bode plots shown in Fig. 19.11
% Values set to a 1 ms loop time for use on an RP2040

% Synchronous Buck converter parameters
Vg = 3.3; Vref = 1.8; D = Vref/Vg; % Input and reference voltages, duty cycle
L = 1e-6; RL = 30e-3; % Inductance and series resistance
C = 200e-6; Resr = 0.8e-3; % Capacitance and capacitor ESR
fo = 1/(2*pi*sqrt(L*C)); % Pole frequency
R = 1000; % Load resistance
fs = 100e3; Ts = 1/fs; % Switching frequency and period

s = tf('s'); z = tf('z',Ts); % Define s and z

% Open-loop control to output transfer function
Gvd = Vg*(Resr+1/s/C)/(Resr + 1/s/C + s*L + RL);
fp2 = 1e3; H = 1/(1 + s/2/pi/fp2); % Sensor transfer function
Tu = H * Gvd; % Uncompensated loop gain, no delay

% Analog PID compensator
fc = 500; % Cross-over frequency
fL = 8; fz = 33; fp1 = 300; % Corner frequencies
Gcm = sqrt(fz/fp1)*(fc/fo)^2/Vg; % Mid-frequency gain
% Analog compensator transfer function
Gc = Gcm*(1 + 2*pi*fL/s)*(1 + s/2/pi/fz)/(1+s/2/pi/fp1);
T = Gc*Tu; % Loop gain with analog compensator

% Uncompensated loop gain, including delay
td = D*Ts; % Delay in the digital control loop
Tu.IODelay = td; % Delay
Tud = c2d(Tu,Ts,'impulse'); % Mapping of Tu with delay
% Analog PID compensator redesigned for digital implementation
fL = 8e3; fz = 22e3; fp1 = 450e3; % Corner frequencies
Gcm = sqrt(fz/fp1)*(fc/fo)^2/Vg; % Mid-frequency gain
Gca = Gcm*(1 + 2*pi*fL/s)*(1 + s/2/pi/fz)/(1+s/2/pi/fp1);
% Digital compensator transfer function
Gcd = c2d(Gca, Ts, 'prewarp', 2*pi*fc);
Td = Tud*Gcd; % Loop gain with digital compensator

% Compare magnitude and phase responses of T and Td
options = bodeoptions; options.Grid = 'on';
options.FreqUnits = 'Hz'; options.XLim = [100, 500e3];
%bode(T, 'k', options); % Bode plot of T
hold on; % Overlay plots
bode(Td, 'b', options); % Bode plot of Td
