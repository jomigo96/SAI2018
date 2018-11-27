% Sistemas Avióncos Integrados - Lab 2 
% Groupo 4:
% - João Gonçalves
% - Tiago Oliveira
% - Carolina Serra

clear all
global b1 b2 p1 p2 q1

%% Import Data
data = csvread("data.csv",1,0);
pressure = data(:,1);
v45n=data(:,2);
v25=data(:,3);
v125=data(:,4);
clear data

%% Fit model to data

% model: V = b0 + b1*P + b2*P^2

Bn45=lsqminnorm([ones(size(pressure)), pressure, pressure.^2], v45n);
B25=lsqminnorm([ones(size(pressure)), pressure, pressure.^2], v25);
B125=lsqminnorm([ones(size(pressure)), pressure, pressure.^2], v125);

y45n=(-Bn45(2)+sqrt(Bn45(2)^2-4*Bn45(3)*(Bn45(1)-v45n)))/(2*Bn45(3));
y25=(-B25(2)+sqrt(B25(2)^2-4*B25(3)*(B25(1)-v25)))/(2*B25(3));
y125=(-B125(2)+sqrt(B125(2)^2-4*B125(3)*(B125(1)-v125)))/(2*B125(3));

%% Model Comparison Plots

figure(1)
plot(v45n, pressure, 'k^');
hold on;
plot(v45n, y45n, 'b-', 'LineWidth', 2);
title('Pressure at -45deg against voltage');
xlabel('Voltage [V]');
ylabel('Pressure [mBar]');
legend('Experimental', 'Least Squares Fit', 'Location', 'NorthWest');

figure(2)
plot(v25, pressure, 'k^');
hold on;
plot(v25, y25, 'b-', 'LineWidth', 2);
title('Pressure at 25deg against voltage');
xlabel('Voltage [V]');
ylabel('Pressure [mBar]');
legend('Experimental', 'Least Squares Fit', 'Location', 'NorthWest');

figure(3)
plot(v125, pressure, 'k^');
hold on;
plot(v125, y125, 'b-', 'LineWidth', 2);
title('Pressure at 125deg against voltage');
xlabel('Voltage [V]');
ylabel('Pressure [mBar]');
legend('Experimental', 'Least Squares Fit', 'Location', 'NorthWest');

%% Error Plots

figure(4)
plot(pressure-y45n, 'LineWidth', 1);
title('Model error at -45deg');
xlabel('Sample');
ylabel('Error [mBar]');

figure(5)
plot(pressure-y25, 'LineWidth', 1);
title('Model error at 25deg');
xlabel('Sample');
ylabel('Error [mBar]');

figure(6)
plot(pressure-y125, 'LineWidth', 1);
title('Model error at 125deg');
xlabel('Sample');
ylabel('Error [mBar]');


%% Fit with Temperature

T = [-45, 25, 125];



% Confirmar, 3-data points, o melhor fit será racional: 
% b0 = (p1*T+p2)/(q1+T)
% Ou talvez exponencial negativo? Polinomio de 2a ordem não dá bem

b0 = [Bn45(1) B25(1) B125(1)];

% Este código vem da curve fitting toolbox
[xData, yData] = prepareCurveData( T, b0 );
ft = fittype( 'rat11' );
opts = fitoptions( 'Method', 'NonlinearLeastSquares' );
opts.Display = 'Off';
opts.StartPoint = [0.0344460805029088 0.438744359656398 0.381558457093008];
[fitresult, ~] = fit( xData, yData, ft, opts );

p1 = fitresult.p1;
p2 = fitresult.p2;
q1 = fitresult.q1;


figure(7)
plot(T, [Bn45(1) B25(1) B125(1)], 'k*');
hold on;
plot(T, [Bn45(2) B25(2) B125(2)], 'r^');
plot(T, [Bn45(3) B25(3) B125(3)], 'bv');
xx = linspace(-45,125, 50);
yy = arrayfun(@(x) (p1*x+p2)/(q1+x), xx);
plot(xx, yy, 'k');
title('Model coefficients as a function of temperature');
xlabel('Temperature [deg C]');
ylabel('Coefficients (Various units)');
legend('b_0', 'b_1', 'b_2', 'b_0 fit', 'Location', 'NorthWest');

% A figura mostra que o efeito da temperatura afeta principalmente o
% offset de Voltagem (b0)

%% Compute pressure from Voltage + Temperature

b1 = mean([Bn45(2) B25(2) B125(2)]);
b2 = mean([Bn45(3) B25(3) B125(3)]);

%function compute_pressure(v, T)

