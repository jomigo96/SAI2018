% Sistemas Avióncos Integrados - Lab 2 
% Groupo 4:
% - João Gonçalves
% - Tiago Oliveira
% - Carolina Serra

clear all

%% Import Data
data = csvread("data.csv",1,0);
pressure = data(:,1);
v45n=data(:,2);
v25=data(:,3);
v125=data(:,4);
clear data

%% Fit model to data

% model: V = b0 + b1*P + b2*P^2

% % for 2017b
% Bn45=lsqminnorm([ones(size(pressure)), pressure, pressure.^2], v45n);
% B25=lsqminnorm([ones(size(pressure)), pressure, pressure.^2], v25);
% B125=lsqminnorm([ones(size(pressure)), pressure, pressure.^2], v125);

% % without 2017b
A = [ones(size(pressure)), pressure, pressure.^2];
Bn45 = ((A'*A)^-1 * A')*v45n;
B25 = ((A'*A)^-1 * A')*v25;
B125 = ((A'*A)^-1 * A')*v125;

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
% b0 = (p1*T+p2)/(q1+T)

b0 = [Bn45(1) B25(1) B125(1)];

% Code from cf toolbox
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

% % For b1 and b2, linear fit is used
% % -> b1 = C1(1)+C1(2)*T;
% % -> b2 = C2(1)+C(2)*T
% C1 = lsqminnorm([1 -45; 1 25; 1 125], [Bn45(2); B25(2); B125(2)]);
% C2 = lsqminnorm([1 -45; 1 25; 1 125], [Bn45(3); B25(3); B125(3)]);

%% Compute pressure from Voltage + Temperature


%function compute_pressure(v, T)
p_c = zeros(3, length(v45n));
for j=1:length(v45n) 
    p_c(1,j) = compute_pressure(v45n(j), -45);
    p_c(2,j) = compute_pressure(v25(j), 25);
    p_c(3,j) = compute_pressure(v125(j), 125);
end

error = abs((p_c - [pressure'; pressure'; pressure']));

figure(8)
scatter3(v45n, -45*ones(size(v45n)), error(1,:), 30, error(1,:), 'filled');
hold on
scatter3(v25, 25*ones(size(v25)), error(2,:), 30, error(2,:), 'filled');
scatter3(v125, 125*ones(size(v125)), error(3,:), 30, error(3,:), 'filled');
view(2)
c=colorbar();
ylabel(c, 'Error [mBar]')
title('Model error')
xlabel('Voltage [V]')
ylabel('Temperature [deg C]')







