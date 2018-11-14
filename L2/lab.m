data = csvread("data.csv",1,0);
pressure = data(:,1);
outn45=data(:,2);
out25=data(:,3);
out125=data(:,4);
clear data

Bn45=lsqminnorm([ones(size(pressure)), pressure, pressure.^2], outn45);
B25=lsqminnorm([ones(size(out25)), out25, out25.^2], pressure);
B125=lsqminnorm([ones(size(out125)), out125, out125.^2], pressure);

yn45=Bn45(1) + Bn45(2)*pressure + Bn45(3)*pressure.^2;
y25=B25(1) + B25(2)*out25 + B25(3)*out125.^2;
y125=B125(1) + B125(2)*out125 + B125(3)*out125.^2;

%plot(outn45-yn45)
plot(pressure-y25)