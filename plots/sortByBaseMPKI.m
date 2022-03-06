% only for the S-curve at the start....
function [MPKIbase, MPKI0, MPKI1, MPKI2, MPKI3, MPKI4, ...
    IPCbase, IPC0, IPC1, IPC2, IPC3, IPC4, ...
    PSEL0, PSEL1, PSEL2, PSEL3, PSEL4] = sortByBaseMPKI(plottype)
    % do all the config2s (with prefetcher)
    base = readstats_nopsel('datapoints/ship-config2');
    pol0 = readstats('datapoints/adaptive-ship-lru-config2');
    pol1 = readstats('datapoints/adaptive-ship-lru-m1');
    pol2 = readstats('datapoints/adaptive-ship-lru-m2');
    pol3 = readstats('datapoints/adaptive-ship-lru-m3');
    pol4 = readstats('datapoints/adaptive-ship-lru-m4');

    % misses / (instructions / 1000)
    MPKIbase = base{1,2} ./ (base{1,3} ./ 1000);
    MPKI0 = pol0{1,2} ./ (pol0{1,3} ./ 1000);
    MPKI1 = pol1{1,2} ./ (pol1{1,3} ./ 1000);
    MPKI2 = pol2{1,2} ./ (pol2{1,3} ./ 1000);
    MPKI3 = pol3{1,2} ./ (pol3{1,3} ./ 1000);
    MPKI4 = pol4{1,2} ./ (pol4{1,3} ./ 1000);

    IPCbase = base{1,4};
    IPC0 = pol0{1,4};
    IPC1 = pol1{1,4};
    IPC2 = pol2{1,4};
    IPC3 = pol3{1,4};
    IPC4 = pol4{1,4};

    PSEL0 = pol0{1,5};
    PSEL1 = pol1{1,5};
    PSEL2 = pol2{1,5};
    PSEL3 = pol3{1,5};
    PSEL4 = pol4{1,5};

    % traces should already be in consistent order across all datapoints/
    % trace name, MPKIs, IPCs
    data = createBaseCellArray(base{1}, MPKIbase, MPKI0, MPKI1, MPKI2, MPKI3, MPKI4);
    % sort by baseline MPKI
    sortedData = sortrows(data,2);
    
    MPKIs = [MPKIbase, MPKI0, MPKI1, MPKI2, MPKI3, MPKI4];
    IPCs = [IPCbase, IPC0, IPC1, IPC2, IPC3, IPC4];
    PSELs = [PSEL0, PSEL1, PSEL2, PSEL3, PSEL4];
    if plottype == 1
        graphBaseComparison(sortedData,1)
    elseif plottype == 2
        graphBoxPlot(MPKIs, IPCs, PSELs, 1)
    else
        graphBaseComparison(sortedData,1)
        graphBoxPlot(MPKIs, IPCs, PSELs, 3)
    end
end