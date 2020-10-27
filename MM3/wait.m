p1_wait = load("d:\\mt\\p1_avg_wait.txt");
p2_wait = load("d:\\mt\\p2_avg_wait.txt");
p3_wait = load("d:\\mt\\p3_avg_wait.txt");

x1_wait = p1_wait(:,1);
x2_wait = p2_wait(:,1);
x3_wait = p3_wait(:,1);
y1_wait = p1_wait(:,2);
y2_wait = p2_wait(:,2);
y3_wait = p3_wait(:,2);

semilogy(x1_wait,y1_wait,x2_wait,y2_wait,x3_wait,y3_wait)
