function graphByMPKI(sortedCells, figurenum)
% cells are of form   51×3 cell array
% 51×1 cell array of MPKIs
MPKIs_cells = sortedCells(:,2);
MPKIs_mat = cell2mat(MPKIs_cells);
traces = sortedCells(:,1);
X = traces';
% needs to match given order which is sorted by MPKI
X = reordercats(X, traces');
figure(figurenum)
bar(X,Y)
end