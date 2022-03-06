function graphBoxPlot(MPKIs, IPCs, PSELs, figurenum)
gbase = repmat({'SHiP++ alone'},1,1);
g0 = repmat({'Default set-duel'},1,1);
g1 = repmat({'m1'},1,1);
g2 = repmat({'m2'},1,1);
g3 = repmat({'m3'},1,1);
g4 = repmat({'m4'},1,1);
g = [gbase; g0; g1; g2; g3; g4];
gPSEL = [g0; g1; g2; g3; g4];

figure(figurenum)
boxplot(MPKIs, g)
xlabel('Policy')
ylabel('MPKI')

figure(figurenum+1)
boxplot(IPCs, g)
xlabel('Policy')
ylabel('IPC')

figure(figurenum+2)
boxplot(PSELs, gPSEL)
xlabel('Policy')
ylabel('Final PSEL')

figure(figurenum+3)
boxplot(MPKIs, g)
xlabel('Policy')
ylabel('MPKI')
ylim([0 85])

figure(figurenum+4)
boxplot(IPCs, g)
xlabel('Policy')
ylabel('IPC')
ylim([0 1.4])
end

