function p = compute_pressure(v, T)
global b1 b2 p1 p2 q1
    b0 = (p1*T+p2)/(q1+T);
    p = (-b1+sqrt(b1^2-4*b2*(b0-v)))/(2*b2);
end

