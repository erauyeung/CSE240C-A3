function outarr = readstats_nopsel(filename)
%readstats Reads statistics for a single binary froma file
%   Trace	Misses	Total instructions	IPC
% 51 traces *  columns
sizeA = [4 Inf];
formatSpec = '%s %f %f %f';
fileID = fopen(filename,'r');
%outarr = fscanf(fileID,formatSpec,sizeA);
outarr = textscan(fileID, formatSpec);
%  1×4 cell array
%    {51×1 cell}    {51×1 int32}    {51×1 int32}    {51×1 double}
% A{1:end}
fclose(fileID);
end